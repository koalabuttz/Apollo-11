/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_ADMISSION_CHECK_H
#define DSKY_NV_ADMISSION_CHECK_H
#include "dsky_nv_admission_cases.h"
uint16_t dsky_nv_admission_checkpoint(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
uint8_t dsky_nv_admission_checkpoint_input(uint8_t,apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission *,enum dsky_nv_entry *);
#endif
