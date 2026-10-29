/* SPDX-License-Identifier: MIT */
#include "nv_render_cases.h"
void nv_render_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){nv_rows_input(s,io,n);(void)p66_nv_data_prepare(s,io);s->words[0117]=(uint16_t)(n%3u);}
