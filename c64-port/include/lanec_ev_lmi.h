/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_LMI_H
#define LANEC_EV_LMI_H
#include "lanec.h"
/* Interpretive words of Luminary099 EXTENDED_VERBS TRMTRAK1 (bank 42),
 * ATTACHIT and TCPIN (bank 43). Entry i is the interpreter at Z (INTPRET or
 * NEWOPS) in `bank` with Q (at INTPRET) or LOC (at NEWOPS) equal to the
 * returned word; it runs until the interpreter leaves these words. Other
 * input returns LANEC_OUT_OF_SCOPE before any change. */
const uint16_t *lanec_ev_lmi_entry(uint16_t i, uint16_t *bank, uint16_t *z, uint16_t *count);
enum lanec_status lanec_ev_lmi_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
