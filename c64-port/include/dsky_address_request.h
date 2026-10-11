/* SPDX-License-Identifier: MIT */
#ifndef DSKY_ADDRESS_REQUEST_H
#define DSKY_ADDRESS_REQUEST_H
#include "dsky_charin_nv.h"
#include "dsky_machine_cadr.h"
#include "dsky_five_blank.h"
#include "noun_reader.h"
enum dsky_request_route {DSKY_REQUEST_VERB,DSKY_REQUEST_END,DSKY_REQUEST_NVSUB_END,DSKY_REQUEST_PASTE,DSKY_REQUEST_ALMCYCLE,DSKY_REQUEST_CCS_HOLE};
/* Actual REQADD2064, operator return2074, USEADD2105, REQDATX/Y/Z2303/5/7
 * or REQCOM2310, bank41. Canonical image; allEB/four selectingSUPER/IRQ.
 * Selected numeric request ordinaryQcaller; source ENTRET producer returns
 * ENDINST/NVSBENDL/MONBACK only when used. USEADD/selected octalreturn noun
 *0..99/eitherzero. Caller owns restoring producer exclusion. */
enum p66_bias_status dsky_address_request(apollo_overlay_state *,dsky_charin_io *,enum dsky_request_route *);
enum exec_status cm_address_request(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,enum dsky_request_route *);
#endif
