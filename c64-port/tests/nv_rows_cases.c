/* SPDX-License-Identifier: MIT */
#include "nv_rows_cases.h"
void nv_rows_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){nv_words_input(s,io,n);(void)p66_nv_verb(s,io);(void)p66_nv_noun(s,io);}
