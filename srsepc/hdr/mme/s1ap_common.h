/*
 * \section LICENSE
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
#ifndef S1AP_COMMON_H
#define S1AP_COMMON_H

static const uint8_t MAX_TA=255;  //Maximum TA supported
static const uint8_t MAX_BPLMN=6; //Maximum broadcasted PLMNs per TAC

typedef struct{
  uint8_t       mme_code;
  uint16_t      mme_group;
  uint16_t      tac;        // 16-bit tac
  uint16_t      mcc;        // BCD-coded with 0xF filler
  uint16_t      mnc;        // BCD-coded with 0xF filler
  std::string   mme_bind_addr;
  std::string   mme_name;
} s1ap_args_t;

typedef struct{
  bool     enb_name_present;
  uint32_t enb_id;
  uint8_t  enb_name[150];
  uint16_t mcc, mnc;
  uint32_t plmn;
  uint8_t  nof_supported_ta;
  uint16_t tac[MAX_TA];
  uint8_t  nof_supported_bplmns[MAX_TA];
  uint16_t bplmns[MAX_TA][MAX_BPLMN];
  LIBLTE_S1AP_PAGINGDRX_ENUM drx;
  struct   sctp_sndrcvinfo sri;
} enb_ctx_t;

typedef struct{
  uint64_t imsi;
  uint32_t enb_ue_s1ap_id;
  uint32_t mme_ue_s1ap_id;
  uint8_t  xres[8];
  uint8_t  k_asme[32];
} ue_ctx_t;

#endif
