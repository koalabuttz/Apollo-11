/* SPDX-License-Identifier: MIT */
#ifndef LANEC_DI_INTERNAL_H
#define LANEC_DI_INTERNAL_H
#include "lanec.h"
/* DISPLAY_INTERFACE_ROUTINES paths shared by the Lane C display modules.
 * Each continues from the named source point to its first boundary, with
 * no domain check of its own: callers check their inputs first. */
void lanec_di_goflash2(apollo_overlay_state *, lanec_io *, enum lanec_mission); /* GOFLASH2, A = flags */
void lanec_di_godspr2(apollo_overlay_state *, lanec_io *, enum lanec_mission);  /* GODSPR2, A = flags */
void lanec_di_godsprs(apollo_overlay_state *, lanec_io *, enum lanec_mission);  /* GODSPRS, A = flags */
void lanec_di_copies(apollo_overlay_state *, lanec_io *, enum lanec_mission);   /* COPIES, A = index */
void lanec_di_copies2(apollo_overlay_state *, lanec_io *, enum lanec_mission);  /* COPIES2 */
void lanec_di_makemark(apollo_overlay_state *, lanec_io *, enum lanec_mission); /* MAKEMARK */
void lanec_di_witchone(apollo_overlay_state *, lanec_io *, enum lanec_mission, uint16_t back); /* TC WITCHONE */
void lanec_di_xchnyloc(apollo_overlay_state *, lanec_io *, enum lanec_mission); /* XCHNYLOC, A = new LOC; LOCCTR in domain */
void lanec_di_chkprio(apollo_overlay_state *, lanec_io *, enum lanec_mission);  /* CHKPRIO */
/* Source "TC Q" or "TC L": runs the TC held in that register. */
void lanec_di_tc_register(apollo_overlay_state *, uint16_t reg, uint16_t return_address);
#endif
