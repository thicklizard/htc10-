/**
 * @file Broadcom Dongle Host Driver (DHD), Flow ring specific code at top level
 *
 * Flow rings are transmit traffic (=propagating towards antenna) related entities
 *
 *
 * Copyright (C) 1999-2016, Broadcom Corporation
 * 
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_flowrings.c jaganlv $
 */


#include <typedefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <proto/ethernet.h>
#include <proto/bcmevent.h>
#include <dngl_stats.h>

#include <dhd.h>

#include <dhd_flowring.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <proto/802.1d.h>
#include <pcie_core.h>
#include <bcmmsgbuf.h>
#include <dhd_pcie.h>

#ifdef CUSTOMER_HW_ONE
extern bool dhd_sta_associated(dhd_pub_t *dhdp, uint32 bssidx, uint8 *mac);
#endif

static INLINE int dhd_flow_queue_throttle(flow_queue_t *queue);

static INLINE uint16 dhd_flowid_find(dhd_pub_t *dhdp, uint8 ifindex,
                                     uint8 prio, char *sa, char *da);

static INLINE uint16 dhd_flowid_alloc(dhd_pub_t *dhdp, uint8 ifindex,
                                      uint8 prio, char *sa, char *da);

static INLINE int dhd_flowid_lookup(dhd_pub_t *dhdp, uint8 ifindex,
                                uint8 prio, char *sa, char *da, uint16 *flowid);
int BCMFASTPATH dhd_flow_queue_overflow(flow_queue_t *queue, void *pkt);

#define FLOW_QUEUE_PKT_NEXT(p)          PKTLINK(p)
#define FLOW_QUEUE_PKT_SETNEXT(p, x)    PKTSETLINK((p), (x))

#ifdef DHD_LOSSLESS_ROAMING
const uint8 prio2ac[8] = { 0, 1, 1, 0, 2, 2, 3, 7 };
#else
const uint8 prio2ac[8] = { 0, 1, 1, 0, 2, 2, 3, 3 };
#endif
const uint8 prio2tid[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

static INLINE int
dhd_flow_queue_throttle(flow_queue_t *queue)
{
	return DHD_FLOW_QUEUE_FULL(queue);
}

int BCMFASTPATH
dhd_flow_queue_overflow(flow_queue_t *queue, void *pkt)
{
	return BCME_NORESOURCE;
}

flow_ring_node_t *
dhd_flow_ring_node(dhd_pub_t *dhdp, uint16 flowid)
{
	flow_ring_node_t * flow_ring_node;

	ASSERT(dhdp != (dhd_pub_t*)NULL);
	ASSERT(flowid < dhdp->num_flow_rings);

	flow_ring_node = &(((flow_ring_node_t*)(dhdp->flow_ring_table))[flowid]);

	ASSERT(flow_ring_node->flowid == flowid);
	return flow_ring_node;
}

flow_queue_t *
dhd_flow_queue(dhd_pub_t *dhdp, uint16 flowid)
{
	flow_ring_node_t * flow_ring_node;

	flow_ring_node = dhd_flow_ring_node(dhdp, flowid);
	return &flow_ring_node->queue;
}


void
dhd_flow_queue_init(dhd_pub_t *dhdp, flow_queue_t *queue, int max)
{
	ASSERT((queue != NULL) && (max > 0));

	dll_init(&queue->list);
	queue->head = queue->tail = NULL;
	queue->len = 0;

	
	ASSERT(max > 1);
	DHD_FLOW_QUEUE_SET_MAX(queue, max);
	DHD_FLOW_QUEUE_SET_THRESHOLD(queue, max);
	DHD_FLOW_QUEUE_SET_CLEN(queue, &dhdp->cumm_ctr);
	DHD_FLOW_QUEUE_SET_L2CLEN(queue, &dhdp->l2cumm_ctr);

	queue->failures = 0U;
	queue->cb = &dhd_flow_queue_overflow;
}

void
dhd_flow_queue_register(flow_queue_t *queue, flow_queue_cb_t cb)
{
	ASSERT(queue != NULL);
	queue->cb = cb;
}

int BCMFASTPATH
dhd_flow_queue_enqueue(dhd_pub_t *dhdp, flow_queue_t *queue, void *pkt)
{
	int ret = BCME_OK;

	ASSERT(queue != NULL);

	if (dhd_flow_queue_throttle(queue)) {
		queue->failures++;
		ret = (*queue->cb)(queue, pkt);
		goto done;
	}

	if (queue->head) {
		FLOW_QUEUE_PKT_SETNEXT(queue->tail, pkt);
	} else {
		queue->head = pkt;
	}

	FLOW_QUEUE_PKT_SETNEXT(pkt, NULL);

	queue->tail = pkt; 

	queue->len++;
	
	DHD_CUMM_CTR_INCR(DHD_FLOW_QUEUE_CLEN_PTR(queue));
	
	DHD_CUMM_CTR_INCR(DHD_FLOW_QUEUE_L2CLEN_PTR(queue));

done:
	return ret;
}

void * BCMFASTPATH
dhd_flow_queue_dequeue(dhd_pub_t *dhdp, flow_queue_t *queue)
{
	void * pkt;

	ASSERT(queue != NULL);

	pkt = queue->head; 

	if (pkt == NULL) {
		ASSERT((queue->len == 0) && (queue->tail == NULL));
		goto done;
	}

	queue->head = FLOW_QUEUE_PKT_NEXT(pkt);
	if (queue->head == NULL)
		queue->tail = NULL;

	queue->len--;
	
	DHD_CUMM_CTR_DECR(DHD_FLOW_QUEUE_CLEN_PTR(queue));
	
	DHD_CUMM_CTR_DECR(DHD_FLOW_QUEUE_L2CLEN_PTR(queue));

	FLOW_QUEUE_PKT_SETNEXT(pkt, NULL); 

done:
	return pkt;
}

int BCMFASTPATH
dhd_flow_queue_reinsert(dhd_pub_t *dhdp, flow_queue_t *queue, void *pkt)
{
	int ret = BCME_OK;

	ASSERT(queue != NULL);

	if (dhd_flow_queue_throttle(queue)) {
		queue->failures++;
		ret = (*queue->cb)(queue, pkt);
		goto done;
	}

	if (queue->head == NULL) {
		queue->tail = pkt;
	}

	FLOW_QUEUE_PKT_SETNEXT(pkt, queue->head);
	queue->head = pkt;
	queue->len++;
	
	DHD_CUMM_CTR_INCR(DHD_FLOW_QUEUE_CLEN_PTR(queue));
	
	DHD_CUMM_CTR_INCR(DHD_FLOW_QUEUE_L2CLEN_PTR(queue));

done:
	return ret;
}

void
dhd_flow_ring_config_thresholds(dhd_pub_t *dhdp, uint16 flowid,
                     int queue_budget, int cumm_threshold, void *cumm_ctr,
                     int l2cumm_threshold, void *l2cumm_ctr)
{
	flow_queue_t * queue;

	ASSERT(dhdp != (dhd_pub_t*)NULL);
	ASSERT(queue_budget > 1);
	ASSERT(cumm_threshold > 1);
	ASSERT(cumm_ctr != (void*)NULL);
	ASSERT(l2cumm_threshold > 1);
	ASSERT(l2cumm_ctr != (void*)NULL);

	queue = dhd_flow_queue(dhdp, flowid);

	DHD_FLOW_QUEUE_SET_MAX(queue, queue_budget); 

	
	DHD_FLOW_QUEUE_SET_THRESHOLD(queue, cumm_threshold);
	DHD_FLOW_QUEUE_SET_CLEN(queue, cumm_ctr);

	
	DHD_FLOW_QUEUE_SET_L2THRESHOLD(queue, l2cumm_threshold);
	DHD_FLOW_QUEUE_SET_L2CLEN(queue, l2cumm_ctr);
}

int
dhd_flow_rings_init(dhd_pub_t *dhdp, uint32 num_flow_rings)
{
	uint32 idx;
	uint32 flow_ring_table_sz;
	uint32 if_flow_lkup_sz = 0;
	void * flowid_allocator;
	flow_ring_table_t *flow_ring_table = NULL;
	if_flow_lkup_t *if_flow_lkup = NULL;
	void *lock = NULL;
	void *list_lock = NULL;
	unsigned long flags;

	DHD_INFO(("%s\n", __FUNCTION__));

	
	flowid_allocator = id16_map_init(dhdp->osh,
	                       num_flow_rings - FLOW_RING_COMMON, FLOWID_RESERVED);
	if (flowid_allocator == NULL) {
		DHD_ERROR(("%s: flowid allocator init failure\n", __FUNCTION__));
		return BCME_NOMEM;
	}

	
	flow_ring_table_sz = (num_flow_rings * sizeof(flow_ring_node_t));
	flow_ring_table = (flow_ring_table_t *)MALLOCZ(dhdp->osh, flow_ring_table_sz);
	if (flow_ring_table == NULL) {
		DHD_ERROR(("%s: flow ring table alloc failure\n", __FUNCTION__));
		goto fail;
	}

	
	DHD_CUMM_CTR_INIT(&dhdp->cumm_ctr);
	DHD_CUMM_CTR_INIT(&dhdp->l2cumm_ctr);
	bzero((uchar *)flow_ring_table, flow_ring_table_sz);
	for (idx = 0; idx < num_flow_rings; idx++) {
		flow_ring_table[idx].status = FLOW_RING_STATUS_CLOSED;
		flow_ring_table[idx].flowid = (uint16)idx;
		flow_ring_table[idx].lock = dhd_os_spin_lock_init(dhdp->osh);
		if (flow_ring_table[idx].lock == NULL) {
			DHD_ERROR(("%s: Failed to init spinlock for queue!\n", __FUNCTION__));
			goto fail;
		}

		dll_init(&flow_ring_table[idx].list);

		
		dhd_flow_queue_init(dhdp, &flow_ring_table[idx].queue,
		                    FLOW_RING_QUEUE_THRESHOLD);
	}

	
	if_flow_lkup_sz = sizeof(if_flow_lkup_t) * DHD_MAX_IFS;
	if_flow_lkup = (if_flow_lkup_t *)DHD_OS_PREALLOC(dhdp,
		DHD_PREALLOC_IF_FLOW_LKUP, if_flow_lkup_sz);
	if (if_flow_lkup == NULL) {
		DHD_ERROR(("%s: if flow lkup alloc failure\n", __FUNCTION__));
		goto fail;
	}

	
	for (idx = 0; idx < DHD_MAX_IFS; idx++) {
		int hash_ix;
		if_flow_lkup[idx].status = 0;
		if_flow_lkup[idx].role = 0;
		for (hash_ix = 0; hash_ix < DHD_FLOWRING_HASH_SIZE; hash_ix++)
			if_flow_lkup[idx].fl_hash[hash_ix] = NULL;
	}

	lock = dhd_os_spin_lock_init(dhdp->osh);
	if (lock == NULL)
		goto fail;

	list_lock = dhd_os_spin_lock_init(dhdp->osh);
	if (list_lock == NULL)
		goto lock_fail;

	dhdp->flow_prio_map_type = DHD_FLOW_PRIO_AC_MAP;
	bcopy(prio2ac, dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);
#ifdef DHD_LOSSLESS_ROAMING
	dhdp->dequeue_prec_map = ALLPRIO;
#endif
	
	DHD_FLOWID_LOCK(lock, flags);
	dhdp->num_flow_rings = num_flow_rings;
	dhdp->flowid_allocator = (void *)flowid_allocator;
	dhdp->flow_ring_table = (void *)flow_ring_table;
	dhdp->if_flow_lkup = (void *)if_flow_lkup;
	dhdp->flowid_lock = lock;
	dhdp->flow_rings_inited = TRUE;
	dhdp->flowring_list_lock = list_lock;
	DHD_FLOWID_UNLOCK(lock, flags);

	DHD_INFO(("%s done\n", __FUNCTION__));
	return BCME_OK;

lock_fail:
	
	dhd_os_spin_lock_deinit(dhdp->osh, lock);

fail:
	
	if (if_flow_lkup != NULL) {
		DHD_OS_PREFREE(dhdp, if_flow_lkup, if_flow_lkup_sz);
	}
	if (flow_ring_table != NULL) {
		for (idx = 0; idx < num_flow_rings; idx++) {
			if (flow_ring_table[idx].lock != NULL)
				dhd_os_spin_lock_deinit(dhdp->osh, flow_ring_table[idx].lock);
		}
		MFREE(dhdp->osh, flow_ring_table, flow_ring_table_sz);
	}
	id16_map_fini(dhdp->osh, flowid_allocator);

	return BCME_NOMEM;
}

void dhd_flow_rings_deinit(dhd_pub_t *dhdp)
{
	uint16 idx;
	uint32 flow_ring_table_sz;
	uint32 if_flow_lkup_sz;
	flow_ring_table_t *flow_ring_table;
	unsigned long flags;
	void *lock;

	DHD_INFO(("dhd_flow_rings_deinit\n"));

	if (!(dhdp->flow_rings_inited)) {
		DHD_ERROR(("dhd_flow_rings not initialized!\n"));
		return;
	}

	if (dhdp->flow_ring_table != NULL) {

		ASSERT(dhdp->num_flow_rings > 0);

		DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
		flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;
		dhdp->flow_ring_table = NULL;
		DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
		for (idx = 0; idx < dhdp->num_flow_rings; idx++) {
			if (flow_ring_table[idx].active) {
				dhd_bus_clean_flow_ring(dhdp->bus, &flow_ring_table[idx]);
			}
			ASSERT(DHD_FLOW_QUEUE_EMPTY(&flow_ring_table[idx].queue));

			
			dhd_os_spin_lock_deinit(dhdp->osh, flow_ring_table[idx].lock);
			flow_ring_table[idx].lock = NULL;

		}

		
		flow_ring_table_sz = dhdp->num_flow_rings * sizeof(flow_ring_table_t);
		MFREE(dhdp->osh, flow_ring_table, flow_ring_table_sz);
	}

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);

	
	if (dhdp->if_flow_lkup != NULL) {
		if_flow_lkup_sz = sizeof(if_flow_lkup_t) * DHD_MAX_IFS;
		bzero((uchar *)dhdp->if_flow_lkup, if_flow_lkup_sz);
		DHD_OS_PREFREE(dhdp, dhdp->if_flow_lkup, if_flow_lkup_sz);
		dhdp->if_flow_lkup = NULL;
	}

	
	if (dhdp->flowid_allocator != NULL)
		dhdp->flowid_allocator = id16_map_fini(dhdp->osh, dhdp->flowid_allocator);

	dhdp->num_flow_rings = 0U;
	bzero(dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);

	lock = dhdp->flowid_lock;
	dhdp->flowid_lock = NULL;

	DHD_FLOWID_UNLOCK(lock, flags);
	dhd_os_spin_lock_deinit(dhdp->osh, lock);

	dhd_os_spin_lock_deinit(dhdp->osh, dhdp->flowring_list_lock);
	dhdp->flowring_list_lock = NULL;

	ASSERT(dhdp->if_flow_lkup == NULL);
	ASSERT(dhdp->flowid_allocator == NULL);
	ASSERT(dhdp->flow_ring_table == NULL);
	dhdp->flow_rings_inited = FALSE;
}

uint8
dhd_flow_rings_ifindex2role(dhd_pub_t *dhdp, uint8 ifindex)
{
	if_flow_lkup_t *if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;
	ASSERT(if_flow_lkup);
	return if_flow_lkup[ifindex].role;
}

#ifdef WLTDLS
bool is_tdls_destination(dhd_pub_t *dhdp, uint8 *da)
{
	tdls_peer_node_t *cur = dhdp->peer_tbl.node;
	while (cur != NULL) {
		if (!memcmp(da, cur->addr, ETHER_ADDR_LEN)) {
			return TRUE;
		}
		cur = cur->next;
	}
	return FALSE;
}
#endif 

static INLINE uint16
dhd_flowid_find(dhd_pub_t *dhdp, uint8 ifindex, uint8 prio, char *sa, char *da)
{
	int hash;
	bool ismcast = FALSE;
	flow_hash_info_t *cur;
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	ASSERT(if_flow_lkup);

	if (if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_STA) {
#ifdef WLTDLS
		if (dhdp->peer_tbl.tdls_peer_count && !(ETHER_ISMULTI(da)) &&
			is_tdls_destination(dhdp, da)) {
			hash = DHD_FLOWRING_HASHINDEX(da, prio);
			cur = if_flow_lkup[ifindex].fl_hash[hash];
			while (cur != NULL) {
				if (!memcmp(cur->flow_info.da, da, ETHER_ADDR_LEN)) {
					DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
					return cur->flowid;
				}
				cur = cur->next;
			}
			DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
			return FLOWID_INVALID;
		}
#endif 
		cur = if_flow_lkup[ifindex].fl_hash[prio];
		if (cur) {
			DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
			return cur->flowid;
		}
	} else {

		if (ETHER_ISMULTI(da)) {
			ismcast = TRUE;
			hash = 0;
		} else {
			hash = DHD_FLOWRING_HASHINDEX(da, prio);
		}

		cur = if_flow_lkup[ifindex].fl_hash[hash];

		while (cur) {
			if ((ismcast && ETHER_ISMULTI(cur->flow_info.da)) ||
				(!memcmp(cur->flow_info.da, da, ETHER_ADDR_LEN) &&
				(cur->flow_info.tid == prio))) {
				DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
				return cur->flowid;
			}
			cur = cur->next;
		}
	}
	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);

	DHD_INFO(("%s: cannot find flowid\n", __FUNCTION__));
	return FLOWID_INVALID;
} 

static INLINE uint16
dhd_flowid_alloc(dhd_pub_t *dhdp, uint8 ifindex, uint8 prio, char *sa, char *da)
{
	flow_hash_info_t *fl_hash_node, *cur;
	if_flow_lkup_t *if_flow_lkup;
	int hash;
	uint16 flowid;
	unsigned long flags;

	fl_hash_node = (flow_hash_info_t *) MALLOC(dhdp->osh, sizeof(flow_hash_info_t));
	memcpy(fl_hash_node->flow_info.da, da, sizeof(fl_hash_node->flow_info.da));

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	ASSERT(dhdp->flowid_allocator != NULL);
	flowid = id16_map_alloc(dhdp->flowid_allocator);
	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);

	if (flowid == FLOWID_INVALID) {
		MFREE(dhdp->osh, fl_hash_node,  sizeof(flow_hash_info_t));
		DHD_ERROR(("%s: cannot get free flowid \n", __FUNCTION__));
#ifdef CUSTOMER_HW_ONE
		queue_work(dhdp->bus->cleanup_wq, &dhdp->bus->cleanup_flow_work);
#endif
		return FLOWID_INVALID;
	}

	fl_hash_node->flowid = flowid;
	fl_hash_node->flow_info.tid = prio;
	fl_hash_node->flow_info.ifindex = ifindex;
	fl_hash_node->next = NULL;

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	if (if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_STA) {
		
#ifdef WLTDLS
		if (dhdp->peer_tbl.tdls_peer_count &&
			(is_tdls_destination(dhdp, da))) {
			hash = DHD_FLOWRING_HASHINDEX(da, prio);
			cur = if_flow_lkup[ifindex].fl_hash[hash];
			if (cur) {
				while (cur->next) {
					cur = cur->next;
				}
				cur->next = fl_hash_node;
			} else {
				if_flow_lkup[ifindex].fl_hash[hash] = fl_hash_node;
			}
		} else
#endif 
			if_flow_lkup[ifindex].fl_hash[prio] = fl_hash_node;
	} else {

		
		hash = ETHER_ISMULTI(da) ? 0 : DHD_FLOWRING_HASHINDEX(da, prio);
		cur = if_flow_lkup[ifindex].fl_hash[hash];
		if (cur) {
			while (cur->next) {
				cur = cur->next;
			}
			cur->next = fl_hash_node;
		} else
			if_flow_lkup[ifindex].fl_hash[hash] = fl_hash_node;
	}
	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);

	DHD_INFO(("%s: allocated flowid %d\n", __FUNCTION__, fl_hash_node->flowid));

	return fl_hash_node->flowid;
} 

static INLINE int
dhd_flowid_lookup(dhd_pub_t *dhdp, uint8 ifindex,
                  uint8 prio, char *sa, char *da, uint16 *flowid)
{
	uint16 id;
	flow_ring_node_t *flow_ring_node;
	flow_ring_table_t *flow_ring_table;
	unsigned long flags;

	DHD_INFO(("%s\n", __FUNCTION__));

	if (!dhdp->flow_ring_table) {
		return BCME_ERROR;
	}

	flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;

	id = dhd_flowid_find(dhdp, ifindex, prio, sa, da);

	if (id == FLOWID_INVALID) {

		if_flow_lkup_t *if_flow_lkup;
		if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

		if (!if_flow_lkup[ifindex].status)
			return BCME_ERROR;

#ifdef CUSTOMER_HW_ONE
		if (!ETHER_ISMULTI(da) &&
				((if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_AP) ||
				 (if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_P2P_GO)) &&
				(!dhd_sta_associated(dhdp, ifindex, da)))
			return BCME_ERROR;
#endif

		id = dhd_flowid_alloc(dhdp, ifindex, prio, sa, da);
		if (id == FLOWID_INVALID) {
			DHD_ERROR(("%s: alloc flowid ifindex %u status %u\n",
			           __FUNCTION__, ifindex, if_flow_lkup[ifindex].status));
			return BCME_ERROR;
		}

		
		dhd_add_flowid(dhdp, ifindex, prio, da, id);
	}

	ASSERT(id < dhdp->num_flow_rings);

	flow_ring_node = (flow_ring_node_t *) &flow_ring_table[id];
	DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);
	if (flow_ring_node->active) {
		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
		*flowid = id;
		return BCME_OK;
	}
	
	memcpy(flow_ring_node->flow_info.sa, sa, sizeof(flow_ring_node->flow_info.sa));
	memcpy(flow_ring_node->flow_info.da, da, sizeof(flow_ring_node->flow_info.da));
	flow_ring_node->flow_info.tid = prio;
	flow_ring_node->flow_info.ifindex = ifindex;
	flow_ring_node->active = TRUE;
	flow_ring_node->status = FLOW_RING_STATUS_PENDING;
	DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
	DHD_FLOWRING_LIST_LOCK(dhdp->flowring_list_lock, flags);
	dll_prepend(&dhdp->bus->const_flowring, &flow_ring_node->list);
	DHD_FLOWRING_LIST_UNLOCK(dhdp->flowring_list_lock, flags);

	
	if (dhd_bus_flow_ring_create_request(dhdp->bus, (void *)flow_ring_node)
	        != BCME_OK) {
		DHD_ERROR(("%s: create error %d\n", __FUNCTION__, id));
		return BCME_ERROR;
	}

	*flowid = id;
	return BCME_OK;
} 

int BCMFASTPATH
dhd_flowid_update(dhd_pub_t *dhdp, uint8 ifindex, uint8 prio, void *pktbuf)
{
	uint8 *pktdata = (uint8 *)PKTDATA(dhdp->osh, pktbuf);
	struct ether_header *eh = (struct ether_header *)pktdata;
	uint16 flowid;

	ASSERT(ifindex < DHD_MAX_IFS);

	if (ifindex >= DHD_MAX_IFS) {
		return BCME_BADARG;
	}

	if (!dhdp->flowid_allocator) {
		DHD_ERROR(("%s: Flow ring not intited yet  \n", __FUNCTION__));
		return BCME_ERROR;
	}

	if (dhd_flowid_lookup(dhdp, ifindex, prio, eh->ether_shost, eh->ether_dhost,
		&flowid) != BCME_OK) {
		return BCME_ERROR;
	}

	DHD_INFO(("%s: prio %d flowid %d\n", __FUNCTION__, prio, flowid));

	
	DHD_PKT_SET_FLOWID(pktbuf, flowid);
	return BCME_OK;
}

void
dhd_flowid_free(dhd_pub_t *dhdp, uint8 ifindex, uint16 flowid)
{
	int hashix;
	bool found = FALSE;
	flow_hash_info_t *cur, *prev;
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	for (hashix = 0; hashix < DHD_FLOWRING_HASH_SIZE; hashix++) {

		cur = if_flow_lkup[ifindex].fl_hash[hashix];

		if (cur) {
			if (cur->flowid == flowid) {
				found = TRUE;
			}

			prev = NULL;
			while (!found && cur) {
				if (cur->flowid == flowid) {
					found = TRUE;
					break;
				}
				prev = cur;
				cur = cur->next;
			}
			if (found) {
				if (!prev) {
					if_flow_lkup[ifindex].fl_hash[hashix] = cur->next;
				} else {
					prev->next = cur->next;
				}

				
				dhd_del_flowid(dhdp, ifindex, flowid);

				id16_map_free(dhdp->flowid_allocator, flowid);
				DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
				MFREE(dhdp->osh, cur, sizeof(flow_hash_info_t));

				return;
			}
		}
	}

	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
	DHD_ERROR(("%s: could not free flow ring hash entry flowid %d\n",
	           __FUNCTION__, flowid));
} 

void
dhd_flow_rings_delete(dhd_pub_t *dhdp, uint8 ifindex)
{
	uint32 id;
	flow_ring_table_t *flow_ring_table;

	DHD_INFO(("%s: ifindex %u\n", __FUNCTION__, ifindex));

	ASSERT(ifindex < DHD_MAX_IFS);
	if (ifindex >= DHD_MAX_IFS)
		return;

	if (!dhdp->flow_ring_table)
		return;

	flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;
	for (id = 0; id < dhdp->num_flow_rings; id++) {
		if (flow_ring_table[id].active &&
		    (flow_ring_table[id].flow_info.ifindex == ifindex)) {
			dhd_bus_flow_ring_delete_request(dhdp->bus,
			                                 (void *) &flow_ring_table[id]);
		}
	}
}
#ifdef CUSTOMER_HW_ONE
#define MAX_NUM_OF_ASSOCLIST	64
#define dtoh32(i) (i)
void
dhd_flow_rings_pending_cleanup(dhd_pub_t *dhdp)
{
	uint32 id, i;
	uint match;
	uint  ret = 0;
	flow_ring_table_t *flow_ring_table;
	char mac_buf[MAX_NUM_OF_ASSOCLIST *
		sizeof(struct ether_addr) + sizeof(uint)] = {0};
	struct maclist *assoc_maclist = (struct maclist *)mac_buf;

	DHD_ERROR(("%s: Enter\n", __func__));

	if (!dhdp->flow_ring_table)
		return;

	assoc_maclist->count = MAX_NUM_OF_ASSOCLIST;
	if ((ret = dhd_wl_ioctl_cmd(dhdp, WLC_GET_ASSOCLIST, assoc_maclist, sizeof(mac_buf), false, 0)) != 0) {
		DHD_ERROR(("%s: failed to get assoclist error %d\n",
				__FUNCTION__, ret));
		return;
	}

	assoc_maclist->count = dtoh32(assoc_maclist->count);
	flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;
	if (assoc_maclist->count) {
		for (i = 0; i < assoc_maclist->count; i++) {
			DHD_ERROR(("%s : associated="MACDBG "\n",
				__FUNCTION__, MAC2STRDBG(assoc_maclist->ea[i].octet)));
		}
	}
	if (assoc_maclist->count) {
		for (id = 0; id < dhdp->num_flow_rings; id++) {
			match = 0;
			for (i = 0; i < assoc_maclist->count; i++) {
				if (memcmp(flow_ring_table[id].flow_info.da, assoc_maclist->ea[i].octet, ETHER_ADDR_LEN) == 0) {
					match = 1;
					break;
				}
			}
			if((!match) && (flow_ring_table[id].active)) {
					dhd_bus_flow_ring_delete_request(dhdp->bus,
							(void *) &flow_ring_table[id]);
			}
		}
	}
	else {
		for (id = 0; id < dhdp->num_flow_rings; id++) {
			if((flow_ring_table[id].active))
				dhd_bus_flow_ring_delete_request(dhdp->bus,
						(void *) &flow_ring_table[id]);
		}
#if defined(CONFIG_DHD_USE_STATIC_BUF)
		PKTFREE_ALL_STATIC(dhdp->osh);
#endif 
	}
}
#endif

void
dhd_flow_rings_delete_for_peer(dhd_pub_t *dhdp, uint8 ifindex, char *addr)
{
	uint32 id;
	flow_ring_table_t *flow_ring_table;

	DHD_ERROR(("%s: ifindex %u\n", __FUNCTION__, ifindex));

	ASSERT(ifindex < DHD_MAX_IFS);
	if (ifindex >= DHD_MAX_IFS)
		return;

	if (!dhdp->flow_ring_table)
		return;

	flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;
	for (id = 0; id < dhdp->num_flow_rings; id++) {
		if (flow_ring_table[id].active &&
			(flow_ring_table[id].flow_info.ifindex == ifindex) &&
			(!memcmp(flow_ring_table[id].flow_info.da, addr, ETHER_ADDR_LEN)) &&
			(flow_ring_table[id].status != FLOW_RING_STATUS_DELETE_PENDING)) {
			DHD_INFO(("%s: deleting flowid %d\n",
				__FUNCTION__, flow_ring_table[id].flowid));
			dhd_bus_flow_ring_delete_request(dhdp->bus,
				(void *) &flow_ring_table[id]);
		}
	}
}

void
dhd_update_interface_flow_info(dhd_pub_t *dhdp, uint8 ifindex,
                               uint8 op, uint8 role)
{
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;

	ASSERT(ifindex < DHD_MAX_IFS);
	if (ifindex >= DHD_MAX_IFS)
		return;

	DHD_INFO(("%s: ifindex %u op %u role is %u \n",
	          __FUNCTION__, ifindex, op, role));
	if (!dhdp->flowid_allocator) {
		DHD_ERROR(("%s: Flow ring not intited yet  \n", __FUNCTION__));
		return;
	}

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	if (op == WLC_E_IF_ADD || op == WLC_E_IF_CHANGE) {

		if_flow_lkup[ifindex].role = role;

		if (role != WLC_E_IF_ROLE_STA) {
			if_flow_lkup[ifindex].status = TRUE;
			DHD_INFO(("%s: Mcast Flow ring for ifindex %d role is %d \n",
			          __FUNCTION__, ifindex, role));
			
		}
	} else	if (op == WLC_E_IF_DEL) {
		if_flow_lkup[ifindex].status = FALSE;
		DHD_INFO(("%s: cleanup all Flow rings for ifindex %d role is %d \n",
		          __FUNCTION__, ifindex, role));
	}
	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
}

int
dhd_update_interface_link_status(dhd_pub_t *dhdp, uint8 ifindex, uint8 status)
{
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;

	ASSERT(ifindex < DHD_MAX_IFS);
	if (ifindex >= DHD_MAX_IFS)
		return BCME_BADARG;

	DHD_INFO(("%s: ifindex %d status %d\n", __FUNCTION__, ifindex, status));

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	if (if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_STA) {
		if (status)
			if_flow_lkup[ifindex].status = TRUE;
		else
			if_flow_lkup[ifindex].status = FALSE;
	}
	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);

	return BCME_OK;
}

int dhd_update_flow_prio_map(dhd_pub_t *dhdp, uint8 map)
{
	uint16 flowid;
	flow_ring_node_t *flow_ring_node;

	if (map > DHD_FLOW_PRIO_TID_MAP)
		return BCME_BADOPTION;

	
	if (map == dhdp->flow_prio_map_type)
		return BCME_OK;

	
	for (flowid = 0; flowid < dhdp->num_flow_rings; flowid++) {
		flow_ring_node = DHD_FLOW_RING(dhdp, flowid);
		if (flow_ring_node->active)
			return BCME_EPERM;
	}

	
	if (BCME_OK != dhd_flow_prio_map(dhdp, &map, TRUE))
		return BCME_ERROR;

	
	dhdp->flow_prio_map_type = map;
	if (dhdp->flow_prio_map_type == DHD_FLOW_PRIO_TID_MAP)
		bcopy(prio2tid, dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);
	else
		bcopy(prio2ac, dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);

	return BCME_OK;
}

int dhd_flow_prio_map(dhd_pub_t *dhd, uint8 *map, bool set)
{
	uint8 iovbuf[24];
	if (!set) {
		bcm_mkiovar("bus:fl_prio_map", NULL, 0, (char*)iovbuf, sizeof(iovbuf));
		if (dhd_wl_ioctl_cmd(dhd, WLC_GET_VAR, iovbuf, sizeof(iovbuf), FALSE, 0) < 0) {
			DHD_ERROR(("%s: failed to get fl_prio_map\n", __FUNCTION__));
			return BCME_ERROR;
		}
		*map = iovbuf[0];
		return BCME_OK;
	}
	bcm_mkiovar("bus:fl_prio_map", (char *)map, 4, (char*)iovbuf, sizeof(iovbuf));
	if (dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0) < 0) {
		DHD_ERROR(("%s: failed to set fl_prio_map \n",
			__FUNCTION__));
		return BCME_ERROR;
	}
	return BCME_OK;
}