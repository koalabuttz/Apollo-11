/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OCTAL_VERB_CHECK_H
#define DSKY_OCTAL_VERB_CHECK_H
#include "dsky_octal_verb_cases.h"
uint8_t dsky_octal_verb_checkpoint_input(uint8_t,apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission *,uint8_t *);
uint16_t dsky_octal_verb_checkpoint(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
