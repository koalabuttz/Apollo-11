/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_FRONT_H
#define LANEC_EV_FRONT_H
#include "lanec.h"
/* EXTENDED_VERBS front end (both missions, bank 43). Each entry runs from
 * the named source address to its first boundary; see lanec.h.
 *   goextvb          GOEXTVB, MPAC = verb-40 (0..59) -> selected verb entry
 *   testxact         TESTXACT -> FALTON (busy) or NVSUB
 *   setxtact         SETXTACT -> NVSUB
 *   xact0            XACT0 -> NVSUB
 *   setxtact_return  SETXTACT +5/+6 (NVSUB returns) -> saved caller return
 *   xactalm          XACTALM -> FALTON; XACTALM +1 (FALTON returns) -> ENDEXT
 *   alm_end          ALM/END -> FALTON; GOPIN (FALTON returns) -> POSTJUMP
 *   chkpooh          LM CHKPOOH -> TCQ (P00) or FALTON
 * Out-of-scope input returns LANEC_OUT_OF_SCOPE before any change. */
enum lanec_status lanec_ev_goextvb(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_ev_testxact(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_ev_setxtact(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_ev_xact0(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_ev_setxtact_return(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_ev_xactalm(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_ev_alm_end(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_ev_chkpooh(apollo_overlay_state *, lanec_io *, enum lanec_mission);
/* Selects the entry above from Z (the continuation table for bank 43). */
enum lanec_status lanec_ev_front_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
