/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_raw_cases.h"
#include "dsky_decimal_general_cases.h"
void dsky_decimal_raw_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t n,enum dsky_nv_mission *mission,uint8_t *rounded){static const agc_word minor[4]={0,AGC_MASK,037777,040000};uint32_t t=n%131072u;dsky_decimal_general_input(s,io,(uint16_t)((n<131072u?0u:8192u)+(n%8192u)),mission,rounded);s->words[0154]=(agc_word)(t>>2);s->words[0155]=minor[t&3u];}
