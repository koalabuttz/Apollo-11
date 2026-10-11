/* SPDX-License-Identifier: MIT */
#include "display_job_lifecycle_cases.h"
void display_job_lifecycle_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n){display_job_input(s,io,channel,(uint16_t)(n%256u));if(n<256u)s->words[0100]=1;else{s->words[01012]=(agc_word)(n%7u+1u);s->words[0374]=(agc_word)(n>=512u?063476u:0u);}}
uint16_t display_job_lifecycle_phases(uint16_t n){return(uint16_t)(n<256u?7u:n<512u?10u:3u);}
agc_word display_job_lifecycle_boundary(uint16_t n,uint16_t phase){static const agc_word sleep[7]={05146,02553,02754,05137,02770,05133,03214},busy[10]={05146,02553,05137,02614,03070,04155,03121,04442,05133,03214},dup[3]={05146,02553,05155};return n<256u?sleep[phase]:n<512u?busy[phase]:dup[phase];}
