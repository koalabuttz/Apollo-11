/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_ALIGN_H
#define LANEC_EV_ALIGN_H
#include "lanec.h"
/* EXTENDED_VERBS alignment and radar-designate bodies: Luminary099 bank 43
 * (VBZERO through RRDESEND, IMUFINEK/FINEK2, RDRUSECK..CHECKP22, IMUATTCK)
 * and bank 23 (AURLOKON), Comanche055 bank 43 (VBZERO through FINEK2,
 * IMUATTCK, TRMATTCK, ATTCK1, CKLFTBTS, XACT0Q). Every label and return
 * point is an entry and runs to the first transfer out of these bodies;
 * a TC Q stops at register Q. Out-of-scope input returns
 * LANEC_OUT_OF_SCOPE before any change. */
/* Entry keys per mission: the address, plus 010000 for LM bank 23. */
const uint16_t *lanec_ev_align_entries(enum lanec_mission, uint16_t *count);
enum lanec_status lanec_ev_align_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
