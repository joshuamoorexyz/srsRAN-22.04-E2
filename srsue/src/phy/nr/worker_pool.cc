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
#include "srsue/hdr/phy/nr/worker_pool.h"

namespace srsue {
namespace nr {

worker_pool::worker_pool(uint32_t max_workers) : pool(max_workers), logger(srslog::fetch_basic_logger("NR-PHY")) {}

bool worker_pool::init(const phy_args_nr_t& args, phy_common* common, stack_interface_phy_nr* stack_, int prio)
{
  phy_state.stack = stack_;
  phy_state.args  = args;

  // Set carrier attributes
  phy_state.carrier.id      = 500;
  phy_state.carrier.nof_prb = args.nof_prb;

  // Set NR arguments
  phy_state.args.nof_carriers   = args.nof_carriers;
  phy_state.args.dl.nof_max_prb = args.nof_prb;

  // Skip init of workers if no NR carriers
  if (phy_state.args.nof_carriers == 0) {
    return true;
  }

  // Add workers to workers pool and start threads
  for (uint32_t i = 0; i < args.nof_phy_threads; i++) {
    auto& log = srslog::fetch_basic_logger(fmt::format("PHY{}", i));
    log.set_level(srslog::str_to_basic_level(args.log.phy_level));
    log.set_hex_dump_max_size(args.log.phy_hex_limit);

    auto w = new sf_worker(common, &phy_state, log);
    pool.init_worker(i, w, prio, args.worker_cpu_mask);
    workers.push_back(std::unique_ptr<sf_worker>(w));

    if (not w->set_carrier_unlocked(0, &phy_state.carrier)) {
      return false;
    }
  }

  // Initialise PRACH
  auto& prach_log = srslog::fetch_basic_logger("NR-PRACH");
  prach_log.set_level(srslog::str_to_basic_level(args.log.phy_level));
  prach_buffer = std::unique_ptr<prach>(new prach(prach_log));
  prach_buffer->init(phy_state.args.dl.nof_max_prb);

  // Set PRACH hard-coded cell
  srslte_cell_t cell = {};
  cell.nof_prb       = 50;
  cell.id            = phy_state.carrier.id;
  if (not prach_buffer->set_cell(cell, phy_state.cfg.prach)) {
    prach_log.error("Setting PRACH cell");
    return false;
  }

  return true;
}

void worker_pool::start_worker(sf_worker* w)
{
  pool.start_worker(w);
}

sf_worker* worker_pool::wait_worker(uint32_t tti)
{
  sf_worker* worker = (sf_worker*)pool.wait_worker(tti);

  // Generate PRACH if ready
  if (prach_buffer->is_ready_to_send(tti, phy_state.carrier.id)) {
    uint32_t nof_prach_sf       = 0;
    float    prach_target_power = 0.0f;
    cf_t*    prach_ptr          = prach_buffer->generate(0.0f, &nof_prach_sf, &prach_target_power);
    worker->set_prach(prach_ptr, prach_target_power);
  }

  return worker;
}

void worker_pool::stop()
{
  pool.stop();
}

void worker_pool::send_prach(uint32_t prach_occasion, uint32_t preamble_index, int preamble_received_target_power)
{
  prach_buffer->prepare_to_send(preamble_index);
}

int worker_pool::set_ul_grant(std::array<uint8_t, SRSLTE_RAR_UL_GRANT_NBITS> packed_ul_grant,
                              uint16_t                                       rnti,
                              srslte_rnti_type_t                             rnti_type)
{
  // Copy DCI bits and setup DCI context
  srslte_dci_msg_nr_t dci_msg = {};
  dci_msg.format              = srslte_dci_format_nr_0_0; // MAC RAR grant shall be unpacked as DCI 0_0 format
  dci_msg.rnti_type           = rnti_type;
  dci_msg.search_space        = srslte_search_space_type_rar; // This indicates it is a MAC RAR
  dci_msg.rnti                = rnti;
  dci_msg.nof_bits            = SRSLTE_RAR_UL_GRANT_NBITS;
  srslte_vec_u8_copy(dci_msg.payload, packed_ul_grant.data(), SRSLTE_RAR_UL_GRANT_NBITS);

  srslte_dci_ul_nr_t dci_ul = {};

  if (srslte_dci_nr_rar_unpack(&dci_msg, &dci_ul) < SRSLTE_SUCCESS) {
    return SRSLTE_ERROR;
  }

  if (logger.info.enabled()) {
    std::array<char, 512> str;
    srslte_dci_ul_nr_to_str(&dci_ul, str.data(), str.size());
    logger.set_context(phy_state.rar_grant_tti);
    logger.info("Setting RAR Grant %s", str.data());
  }

  phy_state.set_ul_pending_grant(phy_state.rar_grant_tti, dci_ul);

  return SRSLTE_SUCCESS;
}
bool worker_pool::set_config(const srslte::phy_cfg_nr_t& cfg)
{
  phy_state.cfg = cfg;
  return true;
}

void worker_pool::sr_send(uint32_t sr_id)
{
  phy_state.set_pending_sr(sr_id);
}
} // namespace nr
} // namespace srsue
