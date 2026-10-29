/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "goplay_cases.h"
#include "dsky_scale.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;uint8_t phase;for(n=0;n<1024u;++n){p66_bias_io io;agc_word channel;goplay_input(&s,&io,&channel,(uint16_t)(n*4u),0);for(phase=0;phase<18u;++phase){unsigned status;if(phase==0u)status=(unsigned)p66_goplay_enter(&s,&io);else if(phase==1u)status=(unsigned)p66_nv_enter(&s,&io);else if(phase==2u)status=(unsigned)p66_nv_verb(&s,&io);else if(phase==3u)status=(unsigned)p66_nv_noun(&s,&io);else if(phase==4u)status=(unsigned)p66_nv_data_prepare(&s,&io);else if(phase==17u)status=(unsigned)p66_goplay_finish(&s,&io,&channel);else switch((phase-5u)%4u){case 0:status=(unsigned)p66_nv_scale_select(&s,&io);break;case 1:status=(unsigned)dsky_scale_output(&s,&io,s.words[0117]==2u?DSKY_DP1OUT:DSKY_DP3OUT);break;case 2:status=(unsigned)p66_nv_decimal_row(&s,&io);break;default:status=(unsigned)p66_nv_next_row(&s,&io);break;}printf("%u %u %u",n,phase,status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel);}}return 0;}
