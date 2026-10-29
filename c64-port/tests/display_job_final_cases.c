/* SPDX-License-Identifier: MIT */
#include "display_job_final_cases.h"
void display_job_final_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n){display_job_input(s,io,channel,n);s->words[0167]=022000;if(n%3u!=0){s->words[0203]=(agc_word)(n%3u==1u?010000u:010401u);s->words[0200]=(agc_word)(n%3u==1u?02546u:agc_negate(03353));s->words[0201]=(agc_word)(n%3u==1u?020067u:064067u);s->words[0202]=0401;if(n%3u==1u){s->words[0171]=01474;s->words[0173]=063476;s->words[0174]=023400;}else{s->words[0177]=077776;s->words[0400]=0;}}}
