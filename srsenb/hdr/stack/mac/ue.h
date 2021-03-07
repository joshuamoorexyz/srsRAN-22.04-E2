/**
 * Copyright 2013-2021 Software Radio Systems Limited
 *
 * This file is part of srsLTE.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#ifndef SRSENB_UE_H
#define SRSENB_UE_H

#include "mac_metrics.h"
#include "srslte/adt/circular_array.h"
#include "srslte/common/block_queue.h"
#include "srslte/common/log.h"
#include "srslte/common/mac_pcap.h"
#include "srslte/common/mac_pcap_net.h"
#include "srslte/interfaces/sched_interface.h"
#include "srslte/mac/pdu.h"
#include "srslte/mac/pdu_queue.h"
#include "srslte/srslog/srslog.h"
#include "ta.h"
#include <pthread.h>
#include <vector>

namespace srsenb {

class rrc_interface_mac;
class rlc_interface_mac;
class phy_interface_stack_lte;

class cc_buffer_handler
{
public:
  // List of Tx softbuffers for all HARQ processes of one carrier
  using cc_softbuffer_tx_list_t = std::vector<srslte_softbuffer_tx_t>;
  // List of Rx softbuffers for all HARQ processes of one carrier
  using cc_softbuffer_rx_list_t = std::vector<srslte_softbuffer_rx_t>;

  cc_buffer_handler();
  ~cc_buffer_handler();

  void reset();
  void allocate_cc(uint32_t nof_prb, uint32_t nof_rx_harq_proc, uint32_t nof_tx_harq_proc);
  void deallocate_cc();

  bool                    empty() const { return softbuffer_tx_list.empty() and softbuffer_rx_list.empty(); }
  srslte_softbuffer_tx_t& get_tx_softbuffer(uint32_t pid, uint32_t tb_idx)
  {
    return softbuffer_tx_list.at(pid * SRSLTE_MAX_TB + tb_idx);
  }
  srslte_softbuffer_rx_t& get_rx_softbuffer(uint32_t tti) { return softbuffer_rx_list.at(tti % nof_rx_harq_proc); }
  srslte::byte_buffer_t*  get_tx_payload_buffer(size_t harq_pid, size_t tb)
  {
    return tx_payload_buffer[harq_pid][tb].get();
  }
  std::map<uint32_t, uint8_t*>& get_rx_used_buffers() { return rx_used_buffers; }

private:
  // args
  uint32_t nof_prb;
  uint32_t nof_rx_harq_proc;
  uint32_t nof_tx_harq_proc;

  // buffers
  cc_softbuffer_tx_list_t      softbuffer_tx_list; ///< List of softbuffer lists for Tx
  cc_softbuffer_rx_list_t      softbuffer_rx_list; ///< List of softbuffer lists for Rx
  std::map<uint32_t, uint8_t*> rx_used_buffers;

  // One buffer per TB per HARQ process and per carrier is needed for each UE.
  std::array<std::array<srslte::unique_byte_buffer_t, SRSLTE_MAX_TB>, SRSLTE_FDD_NOF_HARQ> tx_payload_buffer;
};

class ue : public srslte::read_pdu_interface, public srslte::pdu_queue::process_callback, public mac_ta_ue_interface
{
public:
  ue(uint16_t                 rnti,
     uint32_t                 nof_prb,
     sched_interface*         sched,
     rrc_interface_mac*       rrc_,
     rlc_interface_mac*       rlc,
     phy_interface_stack_lte* phy_,
     srslte::log_ref          log_,
     srslog::basic_logger&    logger,
     uint32_t                 nof_cells_,
     uint32_t                 nof_rx_harq_proc = SRSLTE_FDD_NOF_HARQ,
     uint32_t                 nof_tx_harq_proc = SRSLTE_FDD_NOF_HARQ);

  virtual ~ue();
  void     reset();
  void     start_pcap(srslte::mac_pcap* pcap_);
  void     start_pcap_net(srslte::mac_pcap_net* pcap_net_);
  void     set_tti(uint32_t tti);
  uint16_t get_rnti() { return rnti; }
  uint32_t set_ta(int ta) override;
  void     start_ta() { ta_fsm.start(); };
  uint32_t set_ta_us(float ta_us) { return ta_fsm.push_value(ta_us); };
  void     tic();

  uint8_t* generate_pdu(uint32_t                              ue_cc_idx,
                        uint32_t                              harq_pid,
                        uint32_t                              tb_idx,
                        const sched_interface::dl_sched_pdu_t pdu[sched_interface::MAX_RLC_PDU_LIST],
                        uint32_t                              nof_pdu_elems,
                        uint32_t                              grant_size);
  uint8_t*
  generate_mch_pdu(uint32_t harq_pid, sched_interface::dl_pdu_mch_t sched, uint32_t nof_pdu_elems, uint32_t grant_size);

  srslte_softbuffer_tx_t*
                          get_tx_softbuffer(const uint32_t ue_cc_idx, const uint32_t harq_process, const uint32_t tb_idx);
  srslte_softbuffer_rx_t* get_rx_softbuffer(const uint32_t ue_cc_idx, const uint32_t tti);

  bool     process_pdus();
  uint8_t* request_buffer(uint32_t tti, uint32_t ue_cc_idx, const uint32_t len);
  void     process_pdu(uint8_t* pdu, uint32_t nof_bytes, srslte::pdu_queue::channel_t channel) override;
  void     push_pdu(uint32_t tti, uint32_t ue_cc_idx, uint32_t len);
  void     deallocate_pdu(uint32_t tti, uint32_t ue_cc_idx);
  void     clear_old_buffers(uint32_t tti);

  void metrics_read(mac_ue_metrics_t* metrics_);
  void metrics_rx(bool crc, uint32_t tbs);
  void metrics_tx(bool crc, uint32_t tbs);
  void metrics_phr(float phr);
  void metrics_dl_ri(uint32_t dl_cqi);
  void metrics_dl_pmi(uint32_t dl_cqi);
  void metrics_dl_cqi(uint32_t dl_cqi);
  void metrics_cnt();

  int read_pdu(uint32_t lcid, uint8_t* payload, uint32_t requested_bytes) final;

private:
  void allocate_sdu(srslte::sch_pdu* pdu, uint32_t lcid, uint32_t sdu_len);
  bool process_ce(srslte::sch_subh* subh);
  void allocate_ce(srslte::sch_pdu* pdu, uint32_t lcid);

  uint32_t         phr_counter    = 0;
  uint32_t         dl_cqi_counter = 0;
  uint32_t         dl_ri_counter  = 0;
  uint32_t         dl_pmi_counter = 0;
  mac_ue_metrics_t ue_metrics     = {};

  srslte::mac_pcap*     pcap             = nullptr;
  srslte::mac_pcap_net* pcap_net         = nullptr;
  uint64_t              conres_id        = 0;
  uint16_t              rnti             = 0;
  uint32_t              nof_prb          = 0;
  uint32_t              last_tti         = 0;
  uint32_t              nof_failures     = 0;
  int                   nof_rx_harq_proc = 0;
  int                   nof_tx_harq_proc = 0;

  std::vector<cc_buffer_handler> cc_buffers;

  std::mutex rx_buffers_mutex;

  srslte::block_queue<uint32_t> pending_ta_commands;
  ta                            ta_fsm;

  // For UL there are multiple buffers per PID and are managed by pdu_queue
  srslte::pdu_queue pdus;
  srslte::sch_pdu   mac_msg_dl, mac_msg_ul;
  srslte::mch_pdu   mch_mac_msg_dl;

  rlc_interface_mac*       rlc = nullptr;
  rrc_interface_mac*       rrc = nullptr;
  phy_interface_stack_lte* phy = nullptr;
  srslte::log_ref          log_h;
  srslog::basic_logger&    logger;
  sched_interface*         sched = nullptr;

  // Mutexes
  std::mutex mutex;

  const uint8_t UL_CC_IDX = 0; ///< Passed to write CC index in PCAP (TODO: use actual CC idx)
};

} // namespace srsenb

#endif // SRSENB_UE_H
