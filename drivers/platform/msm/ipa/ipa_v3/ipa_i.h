/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _IPA3_I_H_
#define _IPA3_I_H_

#include <linux/bitops.h>
#include <linux/cdev.h>
#include <linux/export.h>
#include <linux/idr.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/ipa.h>
#include <linux/msm-sps.h>
#include <asm/dma-iommu.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>
#include <linux/firmware.h>
#include "ipa_hw_defs.h"
#include "ipa_ram_mmap.h"
#include "ipa_reg.h"
#include "ipa_qmi_service.h"
#include "../ipa_api.h"

#define DRV_NAME "ipa"
#define NAT_DEV_NAME "ipaNatTable"
#define IPA_COOKIE 0x57831603
#define MTU_BYTE 1500

#define IPA3_MAX_NUM_PIPES 31
#define IPA_SYS_DESC_FIFO_SZ 0x800
#define IPA_SYS_TX_DATA_DESC_FIFO_SZ 0x1000
#define IPA_LAN_RX_HEADER_LENGTH (2)
#define IPA_QMAP_HEADER_LENGTH (4)
#define IPA_DL_CHECKSUM_LENGTH (8)
#define IPA_NUM_DESC_PER_SW_TX (3)
#define IPA_GENERIC_RX_POOL_SZ 192

#define IPA_MAX_STATUS_STAT_NUM 30
#define __FILENAME__ \
	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define IPADBG(fmt, args...) \
	pr_debug(DRV_NAME " %s:%d " fmt, __func__, __LINE__, ## args)
#define IPAERR(fmt, args...) \
	pr_err(DRV_NAME " %s:%d " fmt, __func__, __LINE__, ## args)

#define WLAN_AMPDU_TX_EP 15
#define WLAN_PROD_TX_EP  19
#define WLAN1_CONS_RX_EP  14
#define WLAN2_CONS_RX_EP  16
#define WLAN3_CONS_RX_EP  17
#define WLAN4_CONS_RX_EP  18

#define MAX_NUM_EXCP     8

#define IPA_STATS

#ifdef IPA_STATS
#define IPA_STATS_INC_CNT(val) (++val)
#define IPA_STATS_DEC_CNT(val) (--val)
#define IPA_STATS_EXCP_CNT(flags, base) do {			\
			int i;					\
			for (i = 0; i < MAX_NUM_EXCP; i++)	\
				if (flags & BIT(i))		\
					++base[i];		\
			if (flags == 0)				\
				++base[MAX_NUM_EXCP - 1];	\
			} while (0)
#else
#define IPA_STATS_INC_CNT(x) do { } while (0)
#define IPA_STATS_DEC_CNT(x)
#define IPA_STATS_EXCP_CNT(flags, base) do { } while (0)
#endif

#define IPA_TOS_EQ			BIT(0)
#define IPA_PROTOCOL_EQ			BIT(1)
#define IPA_TC_EQ			BIT(2)
#define IPA_OFFSET_MEQ128_0		BIT(3)
#define IPA_OFFSET_MEQ128_1		BIT(4)
#define IPA_OFFSET_MEQ32_0		BIT(5)
#define IPA_OFFSET_MEQ32_1		BIT(6)
#define IPA_IHL_OFFSET_MEQ32_0		BIT(7)
#define IPA_IHL_OFFSET_MEQ32_1		BIT(8)
#define IPA_METADATA_COMPARE		BIT(9)
#define IPA_IHL_OFFSET_RANGE16_0	BIT(10)
#define IPA_IHL_OFFSET_RANGE16_1	BIT(11)
#define IPA_IHL_OFFSET_EQ_32		BIT(12)
#define IPA_IHL_OFFSET_EQ_16		BIT(13)
#define IPA_FL_EQ			BIT(14)
#define IPA_IS_FRAG			BIT(15)

#define IPA_HDR_BIN0 0
#define IPA_HDR_BIN1 1
#define IPA_HDR_BIN2 2
#define IPA_HDR_BIN3 3
#define IPA_HDR_BIN4 4
#define IPA_HDR_BIN_MAX 5

#define IPA_HDR_PROC_CTX_BIN0 0
#define IPA_HDR_PROC_CTX_BIN1 1
#define IPA_HDR_PROC_CTX_BIN_MAX 2

#define IPA_EVENT_THRESHOLD 0x10

#define IPA_USB_EVENT_THRESHOLD 0x4001

#define IPA_RX_POOL_CEIL 32
#define IPA_RX_SKB_SIZE 1792

#define IPA_A5_MUX_HDR_NAME "ipa_excp_hdr"
#define IPA_LAN_RX_HDR_NAME "ipa_lan_hdr"
#define IPA_INVALID_L4_PROTOCOL 0xFF

#define IPA_CLIENT_IS_PROD(x) (x >= IPA_CLIENT_PROD && x < IPA_CLIENT_CONS)
#define IPA_CLIENT_IS_CONS(x) (x >= IPA_CLIENT_CONS && x < IPA_CLIENT_MAX)
#define IPA_SETFIELD(val, shift, mask) (((val) << (shift)) & (mask))
#define IPA_SETFIELD_IN_REG(reg, val, shift, mask) \
			(reg |= ((val) << (shift)) & (mask))

#define IPA_HW_TABLE_ALIGNMENT(start_ofst) \
	(((start_ofst) + 127) & ~127)
#define IPA_RT_FLT_HW_RULE_BUF_SIZE	(256)

#define IPA_HW_TBL_WIDTH (8)
#define IPA_HW_TBL_SYSADDR_ALIGNMENT (127)
#define IPA_HW_TBL_LCLADDR_ALIGNMENT (7)
#define IPA_HW_TBL_ADDR_MASK (127)
#define IPA_HW_TBL_BLK_SIZE_ALIGNMENT (127)
#define IPA_HW_TBL_HDR_WIDTH (8)
#define IPA_HW_RULE_START_ALIGNMENT (7)

#define IPA_HW_TBL_OFSET_TO_LCLADDR(__ofst) \
	( \
	(((__ofst)/(IPA_HW_TBL_LCLADDR_ALIGNMENT+1)) * \
	(IPA_HW_TBL_ADDR_MASK + 1)) + 1 \
	)

#define IPA_RULE_MAX_PRIORITY (0)
#define IPA_RULE_MIN_PRIORITY (1023)

#define IPA_RULE_ID_MIN_VAL (0x01)
#define IPA_RULE_ID_MAX_VAL (0x1FF)
#define IPA_RULE_ID_RULE_MISS (0x3FF)

#define IPA_HDR_PROC_CTX_TABLE_ALIGNMENT_BYTE 8
#define IPA_HDR_PROC_CTX_TABLE_ALIGNMENT(start_ofst) \
	(((start_ofst) + IPA_HDR_PROC_CTX_TABLE_ALIGNMENT_BYTE - 1) & \
	~(IPA_HDR_PROC_CTX_TABLE_ALIGNMENT_BYTE - 1))

#define MAX_RESOURCE_TO_CLIENTS (IPA_CLIENT_MAX)
#define IPA_MEM_PART(x_) (ipa3_ctx->ctrl->mem_partition.x_)

#define IPA_SMMU_AP_VA_START 0x1000
#define IPA_SMMU_AP_VA_SIZE 0x40000000
#define IPA_SMMU_AP_VA_END (IPA_SMMU_AP_VA_START +  IPA_SMMU_AP_VA_SIZE)
#define IPA_SMMU_UC_VA_START 0x40000000
#define IPA_SMMU_UC_VA_SIZE 0x20000000
#define IPA_SMMU_UC_VA_END (IPA_SMMU_UC_VA_START +  IPA_SMMU_UC_VA_SIZE)

#define IPA_GSI_CHANNEL_STOP_MAX_RETRY 10
#define IPA_GSI_CHANNEL_STOP_PKT_SIZE 1
#define IPA_GSI_CHANNEL_STOP_SLEEP_MIN_USEC (1000)
#define IPA_GSI_CHANNEL_STOP_SLEEP_MAX_USEC (2000)

#define IPA_SLEEP_CLK_RATE_KHZ (32)

#define IPA_ACTIVE_CLIENTS_PREP_EP(log_info, client) \
		log_info.file = __FILENAME__; \
		log_info.line = __LINE__; \
		log_info.type = EP; \
		log_info.id_string = ipa3_clients_strings[client]

#define IPA_ACTIVE_CLIENTS_PREP_SIMPLE(log_info) \
		log_info.file = __FILENAME__; \
		log_info.line = __LINE__; \
		log_info.type = SIMPLE; \
		log_info.id_string = __func__

#define IPA_ACTIVE_CLIENTS_PREP_RESOURCE(log_info, resource_name) \
		log_info.file = __FILENAME__; \
		log_info.line = __LINE__; \
		log_info.type = RESOURCE; \
		log_info.id_string = resource_name

#define IPA_ACTIVE_CLIENTS_PREP_SPECIAL(log_info, id_str) \
		log_info.file = __FILENAME__; \
		log_info.line = __LINE__; \
		log_info.type = SPECIAL; \
		log_info.id_string = id_str

#define IPA_ACTIVE_CLIENTS_INC_EP(client) \
	do { \
		struct ipa3_active_client_logging_info log_info; \
		IPA_ACTIVE_CLIENTS_PREP_EP(log_info, client); \
		ipa3_inc_client_enable_clks(&log_info); \
	} while (0)

#define IPA_ACTIVE_CLIENTS_DEC_EP(client) \
	do { \
		struct ipa3_active_client_logging_info log_info; \
		IPA_ACTIVE_CLIENTS_PREP_EP(log_info, client); \
		ipa3_dec_client_disable_clks(&log_info); \
	} while (0)

#define IPA_ACTIVE_CLIENTS_INC_SIMPLE() \
	do { \
		struct ipa3_active_client_logging_info log_info; \
		IPA_ACTIVE_CLIENTS_PREP_SIMPLE(log_info); \
		ipa3_inc_client_enable_clks(&log_info); \
	} while (0)

#define IPA_ACTIVE_CLIENTS_DEC_SIMPLE() \
	do { \
		struct ipa3_active_client_logging_info log_info; \
		IPA_ACTIVE_CLIENTS_PREP_SIMPLE(log_info); \
		ipa3_dec_client_disable_clks(&log_info); \
	} while (0)

#define IPA_ACTIVE_CLIENTS_INC_RESOURCE(resource_name) \
	do { \
		struct ipa3_active_client_logging_info log_info; \
		IPA_ACTIVE_CLIENTS_PREP_RESOURCE(log_info, resource_name); \
		ipa3_inc_client_enable_clks(&log_info); \
	} while (0)

#define IPA_ACTIVE_CLIENTS_DEC_RESOURCE(resource_name) \
	do { \
		struct ipa3_active_client_logging_info log_info; \
		IPA_ACTIVE_CLIENTS_PREP_RESOURCE(log_info, resource_name); \
		ipa3_dec_client_disable_clks(&log_info); \
	} while (0)

#define IPA_ACTIVE_CLIENTS_INC_SPECIAL(id_str) \
	do { \
		struct ipa3_active_client_logging_info log_info; \
		IPA_ACTIVE_CLIENTS_PREP_SPECIAL(log_info, id_str); \
		ipa3_inc_client_enable_clks(&log_info); \
	} while (0)

#define IPA_ACTIVE_CLIENTS_DEC_SPECIAL(id_str) \
	do { \
		struct ipa3_active_client_logging_info log_info; \
		IPA_ACTIVE_CLIENTS_PREP_SPECIAL(log_info, id_str); \
		ipa3_dec_client_disable_clks(&log_info); \
	} while (0)

#define IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES 120
#define IPA3_ACTIVE_CLIENTS_LOG_LINE_LEN 96
#define IPA3_ACTIVE_CLIENTS_LOG_HASHTABLE_SIZE 50
#define IPA3_ACTIVE_CLIENTS_LOG_NAME_LEN 40

extern const char *ipa3_clients_strings[];

enum ipa3_active_client_log_type {
	EP,
	SIMPLE,
	RESOURCE,
	SPECIAL,
	INVALID
};

struct ipa3_active_client_logging_info {
	const char *id_string;
	char *file;
	int line;
	enum ipa3_active_client_log_type type;
};

struct ipa3_active_client_htable_entry {
	struct hlist_node list;
	char id_string[IPA3_ACTIVE_CLIENTS_LOG_NAME_LEN];
	int count;
	enum ipa3_active_client_log_type type;
};

struct ipa3_active_clients_log_ctx {
	char *log_buffer[IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES];
	int log_head;
	int log_tail;
	bool log_rdy;
	struct hlist_head htable[IPA3_ACTIVE_CLIENTS_LOG_HASHTABLE_SIZE];
};

struct ipa3_client_names {
	enum ipa_client_type names[MAX_RESOURCE_TO_CLIENTS];
	int length;
};

struct ipa_smmu_cb_ctx {
	bool valid;
	struct device *dev;
	struct dma_iommu_mapping *mapping;
	struct iommu_domain *iommu;
	unsigned long next_addr;
};

struct ipa3_mem_buffer {
	void *base;
	dma_addr_t phys_base;
	u32 size;
};

struct ipa3_flt_entry {
	struct list_head link;
	struct ipa_flt_rule rule;
	u32 cookie;
	struct ipa3_flt_tbl *tbl;
	struct ipa3_rt_tbl *rt_tbl;
	u32 hw_len;
	int id;
	u16 prio;
	u16 rule_id;
};

struct ipa3_rt_tbl {
	struct list_head link;
	struct list_head head_rt_rule_list;
	char name[IPA_RESOURCE_NAME_MAX];
	u32 idx;
	u32 rule_cnt;
	u32 ref_cnt;
	struct ipa3_rt_tbl_set *set;
	u32 cookie;
	bool in_sys[IPA_RULE_TYPE_MAX];
	u32 sz[IPA_RULE_TYPE_MAX];
	struct ipa3_mem_buffer curr_mem[IPA_RULE_TYPE_MAX];
	struct ipa3_mem_buffer prev_mem[IPA_RULE_TYPE_MAX];
	int id;
	struct idr rule_ids;
};

struct ipa3_hdr_entry {
	struct list_head link;
	u8 hdr[IPA_HDR_MAX_SIZE];
	u32 hdr_len;
	char name[IPA_RESOURCE_NAME_MAX];
	enum ipa_hdr_l2_type type;
	u8 is_partial;
	bool is_hdr_proc_ctx;
	dma_addr_t phys_base;
	struct ipa3_hdr_proc_ctx_entry *proc_ctx;
	struct ipa3_hdr_offset_entry *offset_entry;
	u32 cookie;
	u32 ref_cnt;
	int id;
	u8 is_eth2_ofst_valid;
	u16 eth2_ofst;
};

struct ipa3_hdr_offset_entry {
	struct list_head link;
	u32 offset;
	u32 bin;
};

struct ipa3_hdr_tbl {
	struct list_head head_hdr_entry_list;
	struct list_head head_offset_list[IPA_HDR_BIN_MAX];
	struct list_head head_free_offset_list[IPA_HDR_BIN_MAX];
	u32 hdr_cnt;
	u32 end;
};

struct ipa3_hdr_proc_ctx_offset_entry {
	struct list_head link;
	u32 offset;
	u32 bin;
};

struct ipa3_hdr_proc_ctx_add_hdr_seq {
	struct ipa3_hdr_proc_ctx_hdr_add hdr_add;
	struct ipa3_hdr_proc_ctx_tlv end;
};

struct ipa3_hdr_proc_ctx_add_hdr_cmd_seq {
	struct ipa3_hdr_proc_ctx_hdr_add hdr_add;
	struct ipa3_hdr_proc_ctx_tlv cmd;
	struct ipa3_hdr_proc_ctx_tlv end;
};

struct ipa3_hdr_proc_ctx_entry {
	struct list_head link;
	enum ipa_hdr_proc_type type;
	struct ipa3_hdr_proc_ctx_offset_entry *offset_entry;
	struct ipa3_hdr_entry *hdr;
	u32 cookie;
	u32 ref_cnt;
	int id;
};

struct ipa3_hdr_proc_ctx_tbl {
	struct list_head head_proc_ctx_entry_list;
	struct list_head head_offset_list[IPA_HDR_PROC_CTX_BIN_MAX];
	struct list_head head_free_offset_list[IPA_HDR_PROC_CTX_BIN_MAX];
	u32 proc_ctx_cnt;
	u32 end;
	u32 start_offset;
};

struct ipa3_flt_tbl {
	struct list_head head_flt_rule_list;
	u32 rule_cnt;
	bool in_sys[IPA_RULE_TYPE_MAX];
	u32 sz[IPA_RULE_TYPE_MAX];
	struct ipa3_mem_buffer curr_mem[IPA_RULE_TYPE_MAX];
	struct ipa3_mem_buffer prev_mem[IPA_RULE_TYPE_MAX];
	bool sticky_rear;
	struct idr rule_ids;
};

struct ipa3_rt_entry {
	struct list_head link;
	struct ipa_rt_rule rule;
	u32 cookie;
	struct ipa3_rt_tbl *tbl;
	struct ipa3_hdr_entry *hdr;
	struct ipa3_hdr_proc_ctx_entry *proc_ctx;
	u32 hw_len;
	int id;
	u16 prio;
	u16 rule_id;
};

struct ipa3_rt_tbl_set {
	struct list_head head_rt_tbl_list;
	u32 tbl_cnt;
};

struct ipa3_ep_cfg_status {
	bool status_en;
	u8 status_ep;
	bool status_location;
};

struct ipa3_wlan_stats {
	u32 rx_pkts_rcvd;
	u32 rx_pkts_status_rcvd;
	u32 rx_hd_processed;
	u32 rx_hd_reply;
	u32 rx_hd_rcvd;
	u32 rx_pkt_leak;
	u32 rx_dp_fail;
	u32 tx_pkts_rcvd;
	u32 tx_pkts_sent;
	u32 tx_pkts_dropped;
};

struct ipa3_wlan_comm_memb {
	spinlock_t wlan_spinlock;
	spinlock_t ipa_tx_mul_spinlock;
	u32 wlan_comm_total_cnt;
	u32 wlan_comm_free_cnt;
	u32 total_tx_pkts_freed;
	struct list_head wlan_comm_desc_list;
	atomic_t active_clnt_cnt;
};

struct ipa_gsi_ep_mem_info {
	u16 evt_ring_len;
	u64 evt_ring_base_addr;
	void *evt_ring_base_vaddr;
	u16 chan_ring_len;
	u64 chan_ring_base_addr;
	void *chan_ring_base_vaddr;
};

struct ipa3_status_stats {
	struct ipa3_hw_pkt_status status[IPA_MAX_STATUS_STAT_NUM];
	int curr;
};

struct ipa3_ep_context {
	int valid;
	enum ipa_client_type client;
	struct sps_pipe *ep_hdl;
	unsigned long gsi_chan_hdl;
	unsigned long gsi_evt_ring_hdl;
	struct ipa_gsi_ep_mem_info gsi_mem_info;
	union __packed gsi_channel_scratch chan_scratch;
	bool bytes_xfered_valid;
	u16 bytes_xfered;
	dma_addr_t phys_base;
	struct ipa_ep_cfg cfg;
	struct ipa_ep_cfg_holb holb;
	struct ipa3_ep_cfg_status status;
	u32 dst_pipe_index;
	u32 rt_tbl_idx;
	struct sps_connect connect;
	void *priv;
	void (*client_notify)(void *priv, enum ipa_dp_evt_type evt,
		       unsigned long data);
	bool desc_fifo_in_pipe_mem;
	bool data_fifo_in_pipe_mem;
	u32 desc_fifo_pipe_mem_ofst;
	u32 data_fifo_pipe_mem_ofst;
	bool desc_fifo_client_allocated;
	bool data_fifo_client_allocated;
	atomic_t avail_fifo_desc;
	u32 dflt_flt4_rule_hdl;
	u32 dflt_flt6_rule_hdl;
	bool skip_ep_cfg;
	bool keep_ipa_awake;
	struct ipa3_wlan_stats wstats;
	u32 wdi_state;
	bool disconnect_in_progress;
	u32 qmi_request_sent;

	
	struct ipa3_sys_context *sys;
};

struct ipa_request_gsi_channel_params {
	struct ipa_ep_cfg ipa_ep_cfg;
	enum ipa_client_type client;
	void *priv;
	ipa_notify_cb notify;
	bool skip_ep_cfg;
	bool keep_ipa_awake;
	struct gsi_evt_ring_props evt_ring_params;
	union __packed gsi_evt_scratch evt_scratch;
	struct gsi_chan_props chan_params;
	union __packed gsi_channel_scratch chan_scratch;
};


enum ipa3_sys_pipe_policy {
	IPA_POLICY_INTR_MODE,
	IPA_POLICY_NOINTR_MODE,
	IPA_POLICY_INTR_POLL_MODE,
};

struct ipa3_repl_ctx {
	struct ipa3_rx_pkt_wrapper **cache;
	atomic_t head_idx;
	atomic_t tail_idx;
	u32 capacity;
};

struct ipa3_sys_context {
	u32 len;
	struct sps_register_event event;
	atomic_t curr_polling_state;
	struct delayed_work switch_to_intr_work;
	enum ipa3_sys_pipe_policy policy;
	int (*pyld_hdlr)(struct sk_buff *skb, struct ipa3_sys_context *sys);
	struct sk_buff * (*get_skb)(unsigned int len, gfp_t flags);
	void (*free_skb)(struct sk_buff *skb);
	u32 rx_buff_sz;
	u32 rx_pool_sz;
	struct sk_buff *prev_skb;
	unsigned int len_rem;
	unsigned int len_pad;
	unsigned int len_partial;
	struct work_struct work;
	void (*sps_callback)(struct sps_event_notify *notify);
	enum sps_option sps_option;
	struct delayed_work replenish_rx_work;
	struct work_struct repl_work;
	void (*repl_hdlr)(struct ipa3_sys_context *sys);
	struct ipa3_repl_ctx repl;

	
	struct ipa3_ep_context *ep;
	struct list_head head_desc_list;
	spinlock_t spinlock;
	struct workqueue_struct *wq;
	struct workqueue_struct *repl_wq;
	struct ipa3_status_stats *status_stat;
	
};

enum ipa3_desc_type {
	IPA_DATA_DESC,
	IPA_DATA_DESC_SKB,
	IPA_IMM_CMD_DESC
};

struct ipa3_tx_pkt_wrapper {
	enum ipa3_desc_type type;
	struct ipa3_mem_buffer mem;
	struct work_struct work;
	struct list_head link;
	void (*callback)(void *user1, int user2);
	void *user1;
	int user2;
	struct ipa3_sys_context *sys;
	struct ipa3_mem_buffer mult;
	u32 cnt;
	void *bounce;
	bool no_unmap_dma;
};

struct ipa3_dma_xfer_wrapper {
	u64 phys_addr_src;
	u64 phys_addr_dest;
	u16 len;
	struct list_head link;
	struct completion xfer_done;
	void (*callback)(void *user1);
	void *user1;
};

struct ipa3_desc {
	enum ipa3_desc_type type;
	void *pyld;
	dma_addr_t dma_address;
	bool dma_address_valid;
	u16 len;
	u16 opcode;
	void (*callback)(void *user1, int user2);
	void *user1;
	int user2;
	struct completion xfer_done;
};

struct ipa3_rx_pkt_wrapper {
	struct list_head link;
	struct ipa_rx_data data;
	u32 len;
	struct work_struct work;
	struct ipa3_sys_context *sys;
};

struct ipa3_nat_mem {
	struct class *class;
	struct device *dev;
	struct cdev cdev;
	dev_t dev_num;
	void *vaddr;
	dma_addr_t dma_handle;
	size_t size;
	bool is_mapped;
	bool is_sys_mem;
	bool is_dev_init;
	bool is_dev;
	struct mutex lock;
	void *nat_base_address;
	char *ipv4_rules_addr;
	char *ipv4_expansion_rules_addr;
	char *index_table_addr;
	char *index_table_expansion_addr;
	u32 size_base_tables;
	u32 size_expansion_tables;
	u32 public_ip_addr;
	void *tmp_vaddr;
	dma_addr_t tmp_dma_handle;
	bool is_tmp_mem;
};

enum ipa3_hw_mode {
	IPA_HW_MODE_NORMAL  = 0,
	IPA_HW_MODE_VIRTUAL = 1,
	IPA_HW_MODE_PCIE    = 2
};

enum ipa3_config_this_ep {
	IPA_CONFIGURE_THIS_EP,
	IPA_DO_NOT_CONFIGURE_THIS_EP,
};

struct ipa3_stats {
	u32 tx_sw_pkts;
	u32 tx_hw_pkts;
	u32 rx_pkts;
	u32 rx_excp_pkts[MAX_NUM_EXCP];
	u32 rx_repl_repost;
	u32 tx_pkts_compl;
	u32 rx_q_len;
	u32 msg_w[IPA_EVENT_MAX_NUM];
	u32 msg_r[IPA_EVENT_MAX_NUM];
	u32 stat_compl;
	u32 aggr_close;
	u32 wan_aggr_close;
	u32 wan_rx_empty;
	u32 wan_repl_rx_empty;
	u32 lan_rx_empty;
	u32 lan_repl_rx_empty;
	u32 flow_enable;
	u32 flow_disable;
};

struct ipa3_active_clients {
	struct mutex mutex;
	spinlock_t spinlock;
	bool mutex_locked;
	int cnt;
};

struct ipa3_wakelock_ref_cnt {
	spinlock_t spinlock;
	int cnt;
};

struct ipa3_tag_completion {
	struct completion comp;
	atomic_t cnt;
};

struct ipa3_debugfs_rt_entry {
	struct ipa_ipfltri_rule_eq eq_attrib;
	uint8_t retain_hdr;
	u16 prio;
	u16 rule_id;
	u8 dst;
	u8 hdr_ofset;
	u8 system;
	u8 is_proc_ctx;
};

struct ipa3_controller;

#define FEATURE_ENUM_VAL(feature, opcode) ((feature << 5) | opcode)
#define EXTRACT_UC_FEATURE(value) (value >> 5)

#define IPA_HW_NUM_FEATURES 0x8

enum ipa3_hw_features {
	IPA_HW_FEATURE_COMMON = 0x0,
	IPA_HW_FEATURE_MHI    = 0x1,
	IPA_HW_FEATURE_WDI    = 0x3,
	IPA_HW_FEATURE_MAX    = IPA_HW_NUM_FEATURES
};

struct IpaHwSharedMemCommonMapping_t {
	u8  cmdOp;
	u8  reserved_01;
	u16 reserved_03_02;
	u32 cmdParams;
	u32 cmdParams_hi;
	u8  responseOp;
	u8  reserved_09;
	u16 reserved_0B_0A;
	u32 responseParams;
	u8  eventOp;
	u8  reserved_11;
	u16 reserved_13_12;
	u32 eventParams;
	u32 reserved_1B_18;
	u32 firstErrorAddress;
	u8  hwState;
	u8  warningCounter;
	u16 reserved_23_22;
	u16 interfaceVersionCommon;
	u16 reserved_27_26;
} __packed;

union IpaHwFeatureInfoData_t {
	struct IpaHwFeatureInfoParams_t {
		u32 offset:16;
		u32 size:16;
	} __packed params;
	u32 raw32b;
} __packed;

struct IpaHwEventInfoData_t {
	u32 baseAddrOffset;
	union IpaHwFeatureInfoData_t featureInfo[IPA_HW_NUM_FEATURES];
} __packed;

struct IpaHwEventLogInfoData_t {
	u32 featureMask;
	u32 circBuffBaseAddrOffset;
	struct IpaHwEventInfoData_t statsInfo;
	struct IpaHwEventInfoData_t configInfo;

} __packed;

struct ipa3_uc_hdlrs {
	void (*ipa_uc_loaded_hdlr)(void);
	void (*ipa_uc_event_hdlr)
		(struct IpaHwSharedMemCommonMapping_t *uc_sram_mmio);
	int (*ipa3_uc_response_hdlr)
		(struct IpaHwSharedMemCommonMapping_t *uc_sram_mmio,
		u32 *uc_status);
	void (*ipa_uc_event_log_info_hdlr)
		(struct IpaHwEventLogInfoData_t *uc_event_top_mmio);
};

enum ipa3_hw_flags {
	IPA_HW_FLAG_HALT_SYSTEM_ON_ASSERT_FAILURE	= 0x01,
	IPA_HW_FLAG_NO_REPORT_MHI_CHANNEL_ERORR		= 0x02,
	IPA_HW_FLAG_NO_REPORT_MHI_CHANNEL_WAKE_UP	= 0x04,
	IPA_HW_FLAG_WORK_OVER_DDR			= 0x08,
	IPA_HW_FLAG_NO_REPORT_OOB			= 0x10,
	IPA_HW_FLAG_NO_REPORT_DB_MODE			= 0x20,
	IPA_HW_FLAG_NO_START_OOB_TIMER			= 0x40
};

enum ipa3_hw_mhi_channel_states {
	IPA_HW_MHI_CHANNEL_STATE_DISABLE	= 0,
	IPA_HW_MHI_CHANNEL_STATE_ENABLE		= 1,
	IPA_HW_MHI_CHANNEL_STATE_RUN		= 2,
	IPA_HW_MHI_CHANNEL_STATE_SUSPEND	= 3,
	IPA_HW_MHI_CHANNEL_STATE_STOP		= 4,
	IPA_HW_MHI_CHANNEL_STATE_ERROR		= 5,
	IPA_HW_MHI_CHANNEL_STATE_INVALID	= 0xFF
};

union IpaHwMhiDlUlSyncCmdData_t {
	struct IpaHwMhiDlUlSyncCmdParams_t {
		u32 isDlUlSyncEnabled:8;
		u32 UlAccmVal:8;
		u32 ulMsiEventThreshold:8;
		u32 dlMsiEventThreshold:8;
	} params;
	u32 raw32b;
};

struct ipa3_uc_ctx {
	bool uc_inited;
	bool uc_loaded;
	bool uc_failed;
	struct mutex uc_lock;
	spinlock_t uc_spinlock;
	struct completion uc_completion;
	struct IpaHwSharedMemCommonMapping_t *uc_sram_mmio;
	struct IpaHwEventLogInfoData_t *uc_event_top_mmio;
	u32 uc_event_top_ofst;
	u32 pending_cmd;
	u32 uc_status;
	bool uc_zip_error;
	u32 uc_error_type;
};

struct ipa3_uc_wdi_ctx {
	
	u32 wdi_uc_stats_ofst;
	struct IpaHwStatsWDIInfoData_t *wdi_uc_stats_mmio;
	void *priv;
	ipa_uc_ready_cb uc_ready_cb;
};

struct ipa3_transport_pm {
	spinlock_t lock;
	bool res_granted;
	bool res_rel_in_prog;
	atomic_t dec_clients;
	atomic_t eot_activity;
};

struct ipa3cm_client_info {
	enum ipacm_client_enum client_enum;
	bool uplink;
};
struct ipa3_hash_tuple {
	
	bool src_id;
	bool src_ip_addr;
	bool dst_ip_addr;
	bool src_port;
	bool dst_port;
	bool protocol;
	bool meta_data;
};

struct ipa3_ready_cb_info {
	struct list_head link;
	ipa_ready_cb ready_cb;
	void *user_data;
};

struct ipa3_context {
	struct class *class;
	dev_t dev_num;
	struct device *dev;
	struct cdev cdev;
	unsigned long bam_handle;
	struct ipa3_ep_context ep[IPA3_MAX_NUM_PIPES];
	bool skip_ep_cfg_shadow[IPA3_MAX_NUM_PIPES];
	u32 ep_flt_bitmap;
	u32 ep_flt_num;
	bool resume_on_connect[IPA_CLIENT_MAX];
	struct ipa3_flt_tbl flt_tbl[IPA3_MAX_NUM_PIPES][IPA_IP_MAX];
	void __iomem *mmio;
	u32 ipa_wrapper_base;
	struct ipa3_hdr_tbl hdr_tbl;
	struct ipa3_hdr_proc_ctx_tbl hdr_proc_ctx_tbl;
	struct ipa3_rt_tbl_set rt_tbl_set[IPA_IP_MAX];
	struct ipa3_rt_tbl_set reap_rt_tbl_set[IPA_IP_MAX];
	struct kmem_cache *flt_rule_cache;
	struct kmem_cache *rt_rule_cache;
	struct kmem_cache *hdr_cache;
	struct kmem_cache *hdr_offset_cache;
	struct kmem_cache *hdr_proc_ctx_cache;
	struct kmem_cache *hdr_proc_ctx_offset_cache;
	struct kmem_cache *rt_tbl_cache;
	struct kmem_cache *tx_pkt_wrapper_cache;
	struct kmem_cache *rx_pkt_wrapper_cache;
	unsigned long rt_idx_bitmap[IPA_IP_MAX];
	struct mutex lock;
	u16 smem_sz;
	u16 smem_restricted_bytes;
	u16 smem_reqd_sz;
	struct ipa3_nat_mem nat_mem;
	u32 excp_hdr_hdl;
	u32 dflt_v4_rt_rule_hdl;
	u32 dflt_v6_rt_rule_hdl;
	uint aggregation_type;
	uint aggregation_byte_limit;
	uint aggregation_time_limit;
	bool hdr_tbl_lcl;
	bool hdr_proc_ctx_tbl_lcl;
	struct ipa3_mem_buffer hdr_mem;
	struct ipa3_mem_buffer hdr_proc_ctx_mem;
	bool ip4_rt_tbl_hash_lcl;
	bool ip4_rt_tbl_nhash_lcl;
	bool ip6_rt_tbl_hash_lcl;
	bool ip6_rt_tbl_nhash_lcl;
	bool ip4_flt_tbl_hash_lcl;
	bool ip4_flt_tbl_nhash_lcl;
	bool ip6_flt_tbl_hash_lcl;
	bool ip6_flt_tbl_nhash_lcl;
	struct ipa3_mem_buffer empty_rt_tbl_mem;
	struct gen_pool *pipe_mem_pool;
	struct dma_pool *dma_pool;
	struct ipa3_active_clients ipa3_active_clients;
	struct ipa3_active_clients_log_ctx ipa3_active_clients_logging;
	struct workqueue_struct *power_mgmt_wq;
	struct workqueue_struct *transport_power_mgmt_wq;
	bool tag_process_before_gating;
	struct ipa3_transport_pm transport_pm;
	u32 clnt_hdl_cmd;
	u32 clnt_hdl_data_in;
	u32 clnt_hdl_data_out;
	spinlock_t disconnect_lock;
	u8 a5_pipe_index;
	struct list_head intf_list;
	struct list_head msg_list;
	struct list_head pull_msg_list;
	struct mutex msg_lock;
	wait_queue_head_t msg_waitq;
	enum ipa_hw_type ipa_hw_type;
	enum ipa3_hw_mode ipa3_hw_mode;
	bool use_ipa_teth_bridge;
	bool ipa_bam_remote_mode;
	bool modem_cfg_emb_pipe_flt;
	
	struct ipa3_stats stats;
	void *smem_pipe_mem;
	u32 ipa_bus_hdl;
	struct ipa3_controller *ctrl;
	struct idr ipa_idr;
	struct device *pdev;
	struct device *uc_pdev;
	spinlock_t idr_lock;
	u32 enable_clock_scaling;
	u32 curr_ipa_clk_rate;
	bool q6_proxy_clk_vote_valid;
	u32 ipa_num_pipes;

	struct ipa3_wlan_comm_memb wc_memb;

	struct ipa3_uc_ctx uc_ctx;

	struct ipa3_uc_wdi_ctx uc_wdi_ctx;
	u32 wan_rx_ring_size;
	bool skip_uc_pipe_reset;
	enum ipa_transport_type transport_prototype;
	unsigned long gsi_dev_hdl;
	u32 ee;
	bool apply_rg10_wa;
	bool smmu_present;
	unsigned long peer_bam_iova;
	phys_addr_t peer_bam_pa;
	u32 peer_bam_map_size;
	unsigned long peer_bam_dev;
	u32 peer_bam_map_cnt;
	u32 wdi_map_cnt;
	struct wakeup_source w_lock;
	struct ipa3_wakelock_ref_cnt wakelock_ref_cnt;
	
	bool ipa_client_apps_wan_cons_agg_gro;
	
	struct ipa3cm_client_info ipacm_client[IPA3_MAX_NUM_PIPES];
	bool tethered_flow_control;
	bool ipa_initialization_complete;
	struct list_head ipa_ready_cb_list;
	struct completion init_completion_obj;
};

struct ipa3_route {
	u32 route_dis;
	u32 route_def_pipe;
	u32 route_def_hdr_table;
	u32 route_def_hdr_ofst;
	u8  route_frag_def_pipe;
	u32 route_def_retain_hdr;
};

enum ipa3_pipe_mem_type {
	IPA_SPS_PIPE_MEM = 0,
	IPA_PRIVATE_MEM  = 1,
	IPA_SYSTEM_MEM   = 2,
};

struct ipa3_plat_drv_res {
	bool use_ipa_teth_bridge;
	u32 ipa_mem_base;
	u32 ipa_mem_size;
	u32 transport_mem_base;
	u32 transport_mem_size;
	u32 ipa_irq;
	u32 transport_irq;
	u32 ipa_pipe_mem_start_ofst;
	u32 ipa_pipe_mem_size;
	enum ipa_hw_type ipa_hw_type;
	enum ipa3_hw_mode ipa3_hw_mode;
	u32 ee;
	bool ipa_bam_remote_mode;
	bool modem_cfg_emb_pipe_flt;
	u32 wan_rx_ring_size;
	bool skip_uc_pipe_reset;
	enum ipa_transport_type transport_prototype;
	bool apply_rg10_wa;
	bool tethered_flow_control;
};

struct ipa3_mem_partition {
	u16 ofst_start;
	u16 nat_ofst;
	u16 nat_size;
	u16 v4_flt_hash_ofst;
	u16 v4_flt_hash_size;
	u16 v4_flt_hash_size_ddr;
	u16 v4_flt_nhash_ofst;
	u16 v4_flt_nhash_size;
	u16 v4_flt_nhash_size_ddr;
	u16 v6_flt_hash_ofst;
	u16 v6_flt_hash_size;
	u16 v6_flt_hash_size_ddr;
	u16 v6_flt_nhash_ofst;
	u16 v6_flt_nhash_size;
	u16 v6_flt_nhash_size_ddr;
	u16 v4_rt_num_index;
	u16 v4_modem_rt_index_lo;
	u16 v4_modem_rt_index_hi;
	u16 v4_apps_rt_index_lo;
	u16 v4_apps_rt_index_hi;
	u16 v4_rt_hash_ofst;
	u16 v4_rt_hash_size;
	u16 v4_rt_hash_size_ddr;
	u16 v4_rt_nhash_ofst;
	u16 v4_rt_nhash_size;
	u16 v4_rt_nhash_size_ddr;
	u16 v6_rt_num_index;
	u16 v6_modem_rt_index_lo;
	u16 v6_modem_rt_index_hi;
	u16 v6_apps_rt_index_lo;
	u16 v6_apps_rt_index_hi;
	u16 v6_rt_hash_ofst;
	u16 v6_rt_hash_size;
	u16 v6_rt_hash_size_ddr;
	u16 v6_rt_nhash_ofst;
	u16 v6_rt_nhash_size;
	u16 v6_rt_nhash_size_ddr;
	u16 modem_hdr_ofst;
	u16 modem_hdr_size;
	u16 apps_hdr_ofst;
	u16 apps_hdr_size;
	u16 apps_hdr_size_ddr;
	u16 modem_hdr_proc_ctx_ofst;
	u16 modem_hdr_proc_ctx_size;
	u16 apps_hdr_proc_ctx_ofst;
	u16 apps_hdr_proc_ctx_size;
	u16 apps_hdr_proc_ctx_size_ddr;
	u16 modem_comp_decomp_ofst;
	u16 modem_comp_decomp_size;
	u16 modem_ofst;
	u16 modem_size;
	u16 apps_v4_flt_hash_ofst;
	u16 apps_v4_flt_hash_size;
	u16 apps_v4_flt_nhash_ofst;
	u16 apps_v4_flt_nhash_size;
	u16 apps_v6_flt_hash_ofst;
	u16 apps_v6_flt_hash_size;
	u16 apps_v6_flt_nhash_ofst;
	u16 apps_v6_flt_nhash_size;
	u16 uc_info_ofst;
	u16 uc_info_size;
	u16 end_ofst;
	u16 apps_v4_rt_hash_ofst;
	u16 apps_v4_rt_hash_size;
	u16 apps_v4_rt_nhash_ofst;
	u16 apps_v4_rt_nhash_size;
	u16 apps_v6_rt_hash_ofst;
	u16 apps_v6_rt_hash_size;
	u16 apps_v6_rt_nhash_ofst;
	u16 apps_v6_rt_nhash_size;
};

struct ipa3_controller {
	struct ipa3_mem_partition mem_partition;
	u32 ipa_clk_rate_turbo;
	u32 ipa_clk_rate_nominal;
	u32 ipa_clk_rate_svs;
	u32 clock_scaling_bw_threshold_turbo;
	u32 clock_scaling_bw_threshold_nominal;
	u32 ipa_reg_base_ofst;
	u32 max_holb_tmr_val;
	void (*ipa_sram_read_settings)(void);
	int (*ipa_init_sram)(void);
	int (*ipa_init_hdr)(void);
	int (*ipa_init_rt4)(void);
	int (*ipa_init_rt6)(void);
	int (*ipa_init_flt4)(void);
	int (*ipa_init_flt6)(void);
	void (*ipa3_cfg_ep_hdr)(u32 pipe_number,
			const struct ipa_ep_cfg_hdr *ipa_ep_hdr_cfg);
	int (*ipa3_cfg_ep_hdr_ext)(u32 pipe_number,
		const struct ipa_ep_cfg_hdr_ext *ipa_ep_hdr_ext_cfg);
	void (*ipa3_cfg_ep_aggr)(u32 pipe_number,
			const struct ipa_ep_cfg_aggr *ipa_ep_agrr_cfg);
	int (*ipa3_cfg_ep_deaggr)(u32 pipe_index,
			const struct ipa_ep_cfg_deaggr *ep_deaggr);
	void (*ipa3_cfg_ep_nat)(u32 pipe_number,
			const struct ipa_ep_cfg_nat *ipa_ep_nat_cfg);
	void (*ipa3_cfg_ep_mode)(u32 pipe_number, u32 dst_pipe_number,
			const struct ipa_ep_cfg_mode *ep_mode);
	void (*ipa3_cfg_ep_route)(u32 pipe_index, u32 rt_tbl_index);
	void (*ipa3_cfg_ep_holb)(u32 pipe_index,
			const struct ipa_ep_cfg_holb *ep_holb);
	void (*ipa3_cfg_route)(struct ipa3_route *route);
	int (*ipa3_read_gen_reg)(char *buff, int max_len);
	int (*ipa3_read_ep_reg)(char *buff, int max_len, int pipe);
	void (*ipa3_write_dbg_cnt)(int option);
	int (*ipa3_read_dbg_cnt)(char *buf, int max_len);
	void (*ipa3_cfg_ep_status)(u32 clnt_hdl,
			const struct ipa3_ep_cfg_status *ep_status);
	int (*ipa3_commit_flt)(enum ipa_ip_type ip);
	int (*ipa3_commit_rt)(enum ipa_ip_type ip);
	int (*ipa_generate_rt_hw_rule)(enum ipa_ip_type ip,
		struct ipa3_rt_entry *entry, u8 *buf);
	int (*ipa3_commit_hdr)(void);
	void (*ipa3_cfg_ep_cfg)(u32 clnt_hdl,
			const struct ipa_ep_cfg_cfg *cfg);
	void (*ipa3_cfg_ep_metadata_mask)(u32 clnt_hdl,
			const struct ipa_ep_cfg_metadata_mask *metadata_mask);
	void (*ipa3_enable_clks)(void);
	void (*ipa3_disable_clks)(void);
	struct msm_bus_scale_pdata *msm_bus_data_ptr;

	void (*ipa3_cfg_ep_metadata)(u32 pipe_number,
			const struct ipa_ep_cfg_metadata *);
};

extern struct ipa3_context *ipa3_ctx;

int ipa3_connect(const struct ipa_connect_params *in,
		struct ipa_sps_params *sps,
		u32 *clnt_hdl);
int ipa3_disconnect(u32 clnt_hdl);

int ipa3_request_gsi_channel(struct ipa_request_gsi_channel_params *params,
			     struct ipa_req_chan_out_params *out_params);

int ipa3_release_gsi_channel(u32 clnt_hdl);

int ipa3_start_gsi_channel(u32 clnt_hdl);

int ipa3_stop_gsi_channel(u32 clnt_hdl);

int ipa3_reset_gsi_channel(u32 clnt_hdl);

int ipa3_reset_gsi_event_ring(u32 clnt_hdl);

int ipa3_set_usb_max_packet_size(
	enum ipa_usb_max_usb_packet_size usb_max_packet_size);

int ipa3_xdci_connect(u32 clnt_hdl, u8 xferrscidx, bool xferrscidx_valid);

int ipa3_xdci_disconnect(u32 clnt_hdl, bool should_force_clear, u32 qmi_req_id);

int ipa3_xdci_suspend(u32 ul_clnt_hdl, u32 dl_clnt_hdl,
	bool should_force_clear, u32 qmi_req_id, bool is_dpl);

int ipa3_xdci_resume(u32 ul_clnt_hdl, u32 dl_clnt_hdl, bool is_dpl);

int ipa3_usb_init(void);

int ipa3_usb_init_teth_prot(enum ipa_usb_teth_prot teth_prot,
			   struct ipa_usb_teth_params *teth_params,
			   int (*ipa_usb_notify_cb)(enum ipa_usb_notify_event,
			   void *),
			   void *user_data);

int ipa3_usb_xdci_connect(struct ipa_usb_xdci_chan_params *ul_chan_params,
			 struct ipa_usb_xdci_chan_params *dl_chan_params,
			 struct ipa_req_chan_out_params *ul_out_params,
			 struct ipa_req_chan_out_params *dl_out_params,
			 struct ipa_usb_xdci_connect_params *connect_params);

int ipa3_usb_xdci_disconnect(u32 ul_clnt_hdl, u32 dl_clnt_hdl,
			     enum ipa_usb_teth_prot teth_prot);

int ipa3_usb_deinit_teth_prot(enum ipa_usb_teth_prot teth_prot);

int ipa3_usb_xdci_suspend(u32 ul_clnt_hdl, u32 dl_clnt_hdl,
			  enum ipa_usb_teth_prot teth_prot);

int ipa3_usb_xdci_resume(u32 ul_clnt_hdl, u32 dl_clnt_hdl,
			enum ipa_usb_teth_prot teth_prot);

int ipa3_reset_endpoint(u32 clnt_hdl);

int ipa3_clear_endpoint_delay(u32 clnt_hdl);

int ipa3_cfg_ep(u32 clnt_hdl, const struct ipa_ep_cfg *ipa_ep_cfg);

int ipa3_cfg_ep_nat(u32 clnt_hdl, const struct ipa_ep_cfg_nat *ipa_ep_cfg);

int ipa3_cfg_ep_hdr(u32 clnt_hdl, const struct ipa_ep_cfg_hdr *ipa_ep_cfg);

int ipa3_cfg_ep_hdr_ext(u32 clnt_hdl,
			const struct ipa_ep_cfg_hdr_ext *ipa_ep_cfg);

int ipa3_cfg_ep_mode(u32 clnt_hdl, const struct ipa_ep_cfg_mode *ipa_ep_cfg);

int ipa3_cfg_ep_aggr(u32 clnt_hdl, const struct ipa_ep_cfg_aggr *ipa_ep_cfg);

int ipa3_cfg_ep_deaggr(u32 clnt_hdl,
		      const struct ipa_ep_cfg_deaggr *ipa_ep_cfg);

int ipa3_cfg_ep_route(u32 clnt_hdl, const struct ipa_ep_cfg_route *ipa_ep_cfg);

int ipa3_cfg_ep_holb(u32 clnt_hdl, const struct ipa_ep_cfg_holb *ipa_ep_cfg);

int ipa3_cfg_ep_cfg(u32 clnt_hdl, const struct ipa_ep_cfg_cfg *ipa_ep_cfg);

int ipa3_cfg_ep_metadata_mask(u32 clnt_hdl,
		const struct ipa_ep_cfg_metadata_mask *ipa_ep_cfg);

int ipa3_cfg_ep_holb_by_client(enum ipa_client_type client,
				const struct ipa_ep_cfg_holb *ipa_ep_cfg);

int ipa3_cfg_ep_ctrl(u32 clnt_hdl, const struct ipa_ep_cfg_ctrl *ep_ctrl);

int ipa3_add_hdr(struct ipa_ioc_add_hdr *hdrs);

int ipa3_del_hdr(struct ipa_ioc_del_hdr *hdls);

int ipa3_commit_hdr(void);

int ipa3_reset_hdr(void);

int ipa3_get_hdr(struct ipa_ioc_get_hdr *lookup);

int ipa3_put_hdr(u32 hdr_hdl);

int ipa3_copy_hdr(struct ipa_ioc_copy_hdr *copy);

int ipa3_add_hdr_proc_ctx(struct ipa_ioc_add_hdr_proc_ctx *proc_ctxs);

int ipa3_del_hdr_proc_ctx(struct ipa_ioc_del_hdr_proc_ctx *hdls);

int ipa3_add_rt_rule(struct ipa_ioc_add_rt_rule *rules);

int ipa3_add_rt_rule_after(struct ipa_ioc_add_rt_rule_after *rules);

int ipa3_del_rt_rule(struct ipa_ioc_del_rt_rule *hdls);

int ipa3_commit_rt(enum ipa_ip_type ip);

int ipa3_reset_rt(enum ipa_ip_type ip);

int ipa3_get_rt_tbl(struct ipa_ioc_get_rt_tbl *lookup);

int ipa3_put_rt_tbl(u32 rt_tbl_hdl);

int ipa3_query_rt_index(struct ipa_ioc_get_rt_tbl_indx *in);

int ipa3_mdfy_rt_rule(struct ipa_ioc_mdfy_rt_rule *rules);

int ipa3_add_flt_rule(struct ipa_ioc_add_flt_rule *rules);

int ipa3_add_flt_rule_after(struct ipa_ioc_add_flt_rule_after *rules);

int ipa3_del_flt_rule(struct ipa_ioc_del_flt_rule *hdls);

int ipa3_mdfy_flt_rule(struct ipa_ioc_mdfy_flt_rule *rules);

int ipa3_commit_flt(enum ipa_ip_type ip);

int ipa3_reset_flt(enum ipa_ip_type ip);

int ipa3_allocate_nat_device(struct ipa_ioc_nat_alloc_mem *mem);

int ipa3_nat_init_cmd(struct ipa_ioc_v4_nat_init *init);

int ipa3_nat_dma_cmd(struct ipa_ioc_nat_dma_cmd *dma);

int ipa3_nat_del_cmd(struct ipa_ioc_v4_nat_del *del);

int ipa3_send_msg(struct ipa_msg_meta *meta, void *buff,
		  ipa_msg_free_fn callback);
int ipa3_register_pull_msg(struct ipa_msg_meta *meta, ipa_msg_pull_fn callback);
int ipa3_deregister_pull_msg(struct ipa_msg_meta *meta);

int ipa3_register_intf(const char *name, const struct ipa_tx_intf *tx,
		       const struct ipa_rx_intf *rx);
int ipa3_register_intf_ext(const char *name, const struct ipa_tx_intf *tx,
		       const struct ipa_rx_intf *rx,
		       const struct ipa_ext_intf *ext);
int ipa3_deregister_intf(const char *name);

int ipa3_set_aggr_mode(enum ipa_aggr_mode mode);

int ipa3_set_qcncm_ndp_sig(char sig[3]);

int ipa3_set_single_ndp_per_mbim(bool enable);

int ipa3_tx_dp(enum ipa_client_type dst, struct sk_buff *skb,
		struct ipa_tx_meta *metadata);

int ipa3_tx_dp_mul(enum ipa_client_type dst,
			struct ipa_tx_data_desc *data_desc);

void ipa3_free_skb(struct ipa_rx_data *);

int ipa3_setup_sys_pipe(struct ipa_sys_connect_params *sys_in, u32 *clnt_hdl);

int ipa3_teardown_sys_pipe(u32 clnt_hdl);

int ipa3_sys_setup(struct ipa_sys_connect_params *sys_in,
	unsigned long *ipa_bam_hdl,
	u32 *ipa_pipe_num, u32 *clnt_hdl, bool en_status);

int ipa3_sys_teardown(u32 clnt_hdl);

int ipa3_sys_update_gsi_hdls(u32 clnt_hdl, unsigned long gsi_ch_hdl,
	unsigned long gsi_ev_hdl);

int ipa3_connect_wdi_pipe(struct ipa_wdi_in_params *in,
		struct ipa_wdi_out_params *out);
int ipa3_disconnect_wdi_pipe(u32 clnt_hdl);
int ipa3_enable_wdi_pipe(u32 clnt_hdl);
int ipa3_disable_wdi_pipe(u32 clnt_hdl);
int ipa3_resume_wdi_pipe(u32 clnt_hdl);
int ipa3_suspend_wdi_pipe(u32 clnt_hdl);
int ipa3_get_wdi_stats(struct IpaHwStatsWDIInfoData_t *stats);
u16 ipa3_get_smem_restr_bytes(void);
int ipa3_uc_wdi_get_dbpa(struct ipa_wdi_db_params *out);

int ipa3_uc_reg_rdyCB(struct ipa_wdi_uc_ready_params *param);
int ipa3_uc_dereg_rdyCB(void);

int ipa3_rm_create_resource(struct ipa_rm_create_params *create_params);

int ipa3_rm_delete_resource(enum ipa_rm_resource_name resource_name);

int ipa3_rm_register(enum ipa_rm_resource_name resource_name,
			struct ipa_rm_register_params *reg_params);

int ipa3_rm_deregister(enum ipa_rm_resource_name resource_name,
			struct ipa_rm_register_params *reg_params);

int ipa3_rm_set_perf_profile(enum ipa_rm_resource_name resource_name,
			struct ipa_rm_perf_profile *profile);

int ipa3_rm_add_dependency(enum ipa_rm_resource_name resource_name,
			enum ipa_rm_resource_name depends_on_name);

int ipa3_rm_delete_dependency(enum ipa_rm_resource_name resource_name,
			enum ipa_rm_resource_name depends_on_name);

int ipa3_rm_request_resource(enum ipa_rm_resource_name resource_name);

int ipa3_rm_release_resource(enum ipa_rm_resource_name resource_name);

int ipa3_rm_notify_completion(enum ipa_rm_event event,
		enum ipa_rm_resource_name resource_name);

int ipa3_rm_inactivity_timer_init(enum ipa_rm_resource_name resource_name,
				 unsigned long msecs);

int ipa3_rm_inactivity_timer_destroy(enum ipa_rm_resource_name resource_name);

int ipa3_rm_inactivity_timer_request_resource(
				enum ipa_rm_resource_name resource_name);

int ipa3_rm_inactivity_timer_release_resource(
				enum ipa_rm_resource_name resource_name);

int ipa3_teth_bridge_init(struct teth_bridge_init_params *params);

int ipa3_teth_bridge_disconnect(enum ipa_client_type client);

int ipa3_teth_bridge_connect(struct teth_bridge_connect_params *connect_params);

void ipa3_set_client(int index, enum ipacm_client_enum client, bool uplink);

enum ipacm_client_enum ipa3_get_client(int pipe_idx);

bool ipa3_get_client_uplink(int pipe_idx);


int ipa3_odu_bridge_init(struct odu_bridge_params *params);

int ipa3_odu_bridge_connect(void);

int ipa3_odu_bridge_disconnect(void);

int ipa3_odu_bridge_tx_dp(struct sk_buff *skb, struct ipa_tx_meta *metadata);

int ipa3_odu_bridge_cleanup(void);

int ipa3_dma_init(void);

int ipa3_dma_enable(void);

int ipa3_dma_disable(void);

int ipa3_dma_sync_memcpy(u64 dest, u64 src, int len);

int ipa3_dma_async_memcpy(u64 dest, u64 src, int len,
			void (*user_cb)(void *user1), void *user_param);

int ipa3_dma_uc_memcpy(phys_addr_t dest, phys_addr_t src, int len);

void ipa3_dma_destroy(void);

int ipa3_mhi_init(struct ipa_mhi_init_params *params);

int ipa3_mhi_start(struct ipa_mhi_start_params *params);

int ipa3_mhi_connect_pipe(struct ipa_mhi_connect_params *in, u32 *clnt_hdl);

int ipa3_mhi_disconnect_pipe(u32 clnt_hdl);

int ipa3_mhi_suspend(bool force);

int ipa3_mhi_resume(void);

void ipa3_mhi_destroy(void);

int ipa3_write_qmap_id(struct ipa_ioc_write_qmapid *param_in);

int ipa3_add_interrupt_handler(enum ipa_irq_type interrupt,
		ipa_irq_handler_t handler,
		bool deferred_flag,
		void *private_data);

int ipa3_remove_interrupt_handler(enum ipa_irq_type interrupt);

void ipa3_bam_reg_dump(void);

int ipa3_get_ep_mapping(enum ipa_client_type client);

bool ipa3_is_ready(void);

void ipa3_proxy_clk_vote(void);
void ipa3_proxy_clk_unvote(void);

bool ipa3_is_client_handle_valid(u32 clnt_hdl);

enum ipa_client_type ipa3_get_client_mapping(int pipe_idx);

void ipa_init_ep_flt_bitmap(void);

bool ipa_is_ep_support_flt(int pipe_idx);

enum ipa_rm_resource_name ipa3_get_rm_resource_from_ep(int pipe_idx);

bool ipa3_get_modem_cfg_emb_pipe_flt(void);


int ipa3_bind_api_controller(enum ipa_hw_type ipa_hw_type,
	struct ipa_api_controller *api_ctrl);

bool ipa_is_modem_pipe(int pipe_idx);

int ipa3_send_one(struct ipa3_sys_context *sys, struct ipa3_desc *desc,
		bool in_atomic);
int ipa3_send(struct ipa3_sys_context *sys,
		u32 num_desc,
		struct ipa3_desc *desc,
		bool in_atomic);
int ipa3_get_ep_mapping(enum ipa_client_type client);
int ipa_get_ep_group(enum ipa_client_type client);

int ipa3_generate_hw_rule(enum ipa_ip_type ip,
			 const struct ipa_rule_attrib *attrib,
			 u8 **buf,
			 u16 *en_rule);
u8 *ipa3_write_64(u64 w, u8 *dest);
u8 *ipa3_write_32(u32 w, u8 *dest);
u8 *ipa3_write_16(u16 hw, u8 *dest);
u8 *ipa3_write_8(u8 b, u8 *dest);
u8 *ipa3_pad_to_32(u8 *dest);
u8 *ipa3_pad_to_64(u8 *dest);
int ipa3_init_hw(void);
struct ipa3_rt_tbl *__ipa3_find_rt_tbl(enum ipa_ip_type ip, const char *name);
int ipa3_set_single_ndp_per_mbim(bool);
void ipa3_debugfs_init(void);
void ipa3_debugfs_remove(void);

void ipa3_dump_buff_internal(void *base, dma_addr_t phy_base, u32 size);
#ifdef IPA_DEBUG
#define IPA_DUMP_BUFF(base, phy_base, size) \
	ipa3_dump_buff_internal(base, phy_base, size)
#else
#define IPA_DUMP_BUFF(base, phy_base, size)
#endif
int ipa3_controller_static_bind(struct ipa3_controller *controller,
		enum ipa_hw_type ipa_hw_type);
int ipa3_cfg_route(struct ipa3_route *route);
int ipa3_send_cmd(u16 num_desc, struct ipa3_desc *descr);
int ipa3_cfg_filter(u32 disable);
int ipa3_pipe_mem_init(u32 start_ofst, u32 size);
int ipa3_pipe_mem_alloc(u32 *ofst, u32 size);
int ipa3_pipe_mem_free(u32 ofst, u32 size);
int ipa3_straddle_boundary(u32 start, u32 end, u32 boundary);
struct ipa3_context *ipa3_get_ctx(void);
void ipa3_enable_clks(void);
void ipa3_disable_clks(void);
void ipa3_inc_client_enable_clks(struct ipa3_active_client_logging_info *id);
int ipa3_inc_client_enable_clks_no_block(struct ipa3_active_client_logging_info
		*id);
void ipa3_dec_client_disable_clks(struct ipa3_active_client_logging_info *id);
void ipa3_active_clients_log_dec(struct ipa3_active_client_logging_info *id,
		bool int_ctx);
void ipa3_active_clients_log_inc(struct ipa3_active_client_logging_info *id,
		bool int_ctx);
int ipa3_active_clients_log_print_buffer(char *buf, int size);
int ipa3_active_clients_log_print_table(char *buf, int size);
void ipa3_active_clients_log_clear(void);
int ipa3_interrupts_init(u32 ipa_irq, u32 ee, struct device *ipa_dev);
int __ipa3_del_rt_rule(u32 rule_hdl);
int __ipa3_del_hdr(u32 hdr_hdl);
int __ipa3_release_hdr(u32 hdr_hdl);
int __ipa3_release_hdr_proc_ctx(u32 proc_ctx_hdl);
int _ipa_read_gen_reg_v3_0(char *buff, int max_len);
int _ipa_read_ep_reg_v3_0(char *buf, int max_len, int pipe);
void _ipa_write_dbg_cnt_v3_0(int option);
int _ipa_read_dbg_cnt_v3_0(char *buf, int max_len);
void _ipa_enable_clks_v3_0(void);
void _ipa_disable_clks_v3_0(void);
struct device *ipa3_get_dma_dev(void);
void ipa3_suspend_active_aggr_wa(u32 clnt_hdl);
void ipa3_suspend_handler(enum ipa_irq_type interrupt,
				void *private_data,
				void *interrupt_data);


static inline u32 ipa_read_reg(void *base, u32 offset)
{
	return ioread32(base + offset);
}

static inline u32 ipa_read_reg_field(void *base, u32 offset,
		u32 mask, u32 shift)
{
	return (ipa_read_reg(base, offset) & mask) >> shift;
}

static inline void ipa_write_reg(void *base, u32 offset, u32 val)
{
	iowrite32(val, base + offset);
}

int ipa_bridge_init(void);
void ipa_bridge_cleanup(void);

ssize_t ipa3_read(struct file *filp, char __user *buf, size_t count,
		 loff_t *f_pos);
int ipa3_pull_msg(struct ipa_msg_meta *meta, char *buff, size_t count);
int ipa3_query_intf(struct ipa_ioc_query_intf *lookup);
int ipa3_query_intf_tx_props(struct ipa_ioc_query_intf_tx_props *tx);
int ipa3_query_intf_rx_props(struct ipa_ioc_query_intf_rx_props *rx);
int ipa3_query_intf_ext_props(struct ipa_ioc_query_intf_ext_props *ext);

void wwan_cleanup(void);

int ipa3_teth_bridge_driver_init(void);
void ipa3_lan_rx_cb(void *priv, enum ipa_dp_evt_type evt, unsigned long data);

int _ipa_init_sram_v3_0(void);
int _ipa_init_hdr_v3_0(void);
int _ipa_init_rt4_v3(void);
int _ipa_init_rt6_v3(void);
int _ipa_init_flt4_v3(void);
int _ipa_init_flt6_v3(void);

int __ipa_commit_flt_v3(enum ipa_ip_type ip);
int __ipa_commit_rt_v3(enum ipa_ip_type ip);
int __ipa_generate_rt_hw_rule_v3_0(enum ipa_ip_type ip,
	struct ipa3_rt_entry *entry, u8 *buf);

int __ipa_commit_hdr_v3_0(void);
int ipa3_generate_flt_eq(enum ipa_ip_type ip,
		const struct ipa_rule_attrib *attrib,
		struct ipa_ipfltri_rule_eq *eq_attrib);
void ipa3_skb_recycle(struct sk_buff *skb);
void ipa3_install_dflt_flt_rules(u32 ipa_ep_idx);
void ipa3_delete_dflt_flt_rules(u32 ipa_ep_idx);

int ipa3_enable_data_path(u32 clnt_hdl);
int ipa3_disable_data_path(u32 clnt_hdl);
int ipa3_alloc_rule_id(struct idr *rule_ids);
int ipa3_id_alloc(void *ptr);
void *ipa3_id_find(u32 id);
void ipa3_id_remove(u32 id);

int ipa3_set_required_perf_profile(enum ipa_voltage_level floor_voltage,
				  u32 bandwidth_mbps);

int ipa3_cfg_ep_status(u32 clnt_hdl,
		const struct ipa3_ep_cfg_status *ipa_ep_cfg);
int ipa3_cfg_aggr_cntr_granularity(u8 aggr_granularity);
int ipa3_cfg_eot_coal_cntr_granularity(u8 eot_coal_granularity);

int ipa3_suspend_resource_no_block(enum ipa_rm_resource_name name);
int ipa3_suspend_resource_sync(enum ipa_rm_resource_name name);
int ipa3_resume_resource(enum ipa_rm_resource_name name);
bool ipa3_should_pipe_be_suspended(enum ipa_client_type client);
int ipa3_tag_aggr_force_close(int pipe_num);

void ipa3_active_clients_lock(void);
int ipa3_active_clients_trylock(unsigned long *flags);
void ipa3_active_clients_unlock(void);
void ipa3_active_clients_trylock_unlock(unsigned long *flags);
int ipa3_wdi_init(void);
int ipa3_write_qmapid_wdi_pipe(u32 clnt_hdl, u8 qmap_id);
int ipa3_tag_process(struct ipa3_desc *desc, int num_descs,
		    unsigned long timeout);

int ipa3_q6_cleanup(void);
int ipa3_q6_pipe_reset(void);
int ipa3_init_q6_smem(void);

int ipa3_sps_connect_safe(struct sps_pipe *h, struct sps_connect *connect,
			 enum ipa_client_type ipa_client);

int ipa3_mhi_handle_ipa_config_req(struct ipa_config_req_msg_v01 *config_req);

int ipa3_uc_interface_init(void);
int ipa3_uc_reset_pipe(enum ipa_client_type ipa_client);
int ipa3_uc_state_check(void);
int ipa3_uc_loaded_check(void);
void ipa3_uc_load_notify(void);
int ipa3_uc_send_cmd(u32 cmd, u32 opcode, u32 expected_status,
		    bool polling_mode, unsigned long timeout_jiffies);
void ipa3_register_panic_hdlr(void);
void ipa3_uc_register_handlers(enum ipa3_hw_features feature,
			      struct ipa3_uc_hdlrs *hdlrs);
int ipa3_create_nat_device(void);
int ipa3_uc_notify_clk_state(bool enabled);
void ipa3_dma_async_memcpy_notify_cb(void *priv,
		enum ipa_dp_evt_type evt, unsigned long data);

int ipa3_uc_update_hw_flags(u32 flags);

int ipa3_uc_mhi_init(void (*ready_cb)(void), void (*wakeup_request_cb)(void));
void ipa3_uc_mhi_cleanup(void);
int ipa3_uc_mhi_send_dl_ul_sync_info(union IpaHwMhiDlUlSyncCmdData_t cmd);
int ipa3_uc_mhi_init_engine(struct ipa_mhi_msi_info *msi, u32 mmio_addr,
	u32 host_ctrl_addr, u32 host_data_addr, u32 first_ch_idx,
	u32 first_evt_idx);
int ipa3_uc_mhi_init_channel(int ipa_ep_idx, int channelHandle,
	int contexArrayIndex, int channelDirection);
int ipa3_uc_mhi_reset_channel(int channelHandle);
int ipa3_uc_mhi_suspend_channel(int channelHandle);
int ipa3_uc_mhi_resume_channel(int channelHandle, bool LPTransitionRejected);
int ipa3_uc_mhi_stop_event_update_channel(int channelHandle);
int ipa3_uc_mhi_print_stats(char *dbg_buff, int size);
int ipa3_uc_memcpy(phys_addr_t dest, phys_addr_t src, int len);
void ipa3_tag_free_buf(void *user1, int user2);
struct ipa_gsi_ep_config *ipa3_get_gsi_ep_info(int ipa_ep_idx);
void ipa3_uc_rg10_write_reg(void *base, u32 offset, u32 val);

u32 ipa3_get_num_pipes(void);
struct ipa_smmu_cb_ctx *ipa3_get_wlan_smmu_ctx(void);
struct ipa_smmu_cb_ctx *ipa3_get_uc_smmu_ctx(void);
struct iommu_domain *ipa_get_uc_smmu_domain(void);
int ipa3_ap_suspend(struct device *dev);
int ipa3_ap_resume(struct device *dev);
int ipa3_init_interrupts(void);
struct iommu_domain *ipa3_get_smmu_domain(void);
int ipa3_rm_add_dependency_sync(enum ipa_rm_resource_name resource_name,
		enum ipa_rm_resource_name depends_on_name);
int ipa3_release_wdi_mapping(u32 num_buffers, struct ipa_wdi_buffer_info *info);
int ipa3_create_wdi_mapping(u32 num_buffers, struct ipa_wdi_buffer_info *info);
int ipa3_set_flt_tuple_mask(int pipe_idx, struct ipa3_hash_tuple *tuple);
int ipa3_set_rt_tuple_mask(int tbl_idx, struct ipa3_hash_tuple *tuple);
void ipa3_set_resorce_groups_min_max_limits(void);
void ipa3_suspend_apps_pipes(bool suspend);
void ipa3_flow_control(enum ipa_client_type ipa_client, bool enable,
			uint32_t qmap_id);
int ipa3_generate_eq_from_hw_rule(
	struct ipa_ipfltri_rule_eq *attrib, u8 *buf, u8 *rule_size);
int ipa3_flt_read_tbl_from_hw(u32 pipe_idx,
	enum ipa_ip_type ip_type,
	bool hashable,
	struct ipa3_flt_entry entry[],
	int *num_entry);
int ipa3_rt_read_tbl_from_hw(u32 tbl_idx,
	enum ipa_ip_type ip_type,
	bool hashable,
	struct ipa3_debugfs_rt_entry entry[],
	int *num_entry);
int ipa3_calc_extra_wrd_bytes(const struct ipa_ipfltri_rule_eq *attrib);
const char *ipa3_rm_resource_str(enum ipa_rm_resource_name resource_name);
int ipa3_restore_suspend_handler(void);
int ipa3_inject_dma_task_for_gsi(void);
void ipa3_inc_acquire_wakelock(void);
void ipa3_dec_release_wakelock(void);
int ipa3_load_fws(const struct firmware *firmware);
int ipa3_register_ipa_ready_cb(void (*ipa_ready_cb)(void *), void *user_data);
#endif 
