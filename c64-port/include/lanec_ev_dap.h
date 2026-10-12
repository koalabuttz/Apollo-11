/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_DAP_H
#define LANEC_EV_DAP_H
#include "lanec.h"
/* EXTENDED_VERBS V48 DAP data load: Luminary099 R03 (bank 20 DAPDATA1 and
 * bank 01 STIKLOAD through TRIMDONE) and Comanche055 DAPDISP (bank 43) with
 * DAPDISP1 (bank 42). Every label and return point is an entry of its bank
 * and runs to the first transfer out of these bodies. Out-of-scope input
 * returns LANEC_OUT_OF_SCOPE before any change. */
const uint16_t *lanec_ev_dap_entries(enum lanec_mission, uint16_t bank, uint16_t *count);
enum lanec_status lanec_ev_dap_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
