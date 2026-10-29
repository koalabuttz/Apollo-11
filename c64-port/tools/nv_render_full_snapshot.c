/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "nv_render_cases.h"
#include "dsky_scale.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;uint8_t phase;for(n=0;n<1024u;++n){p66_bias_io io;nv_rows_input(&s,&io,(uint16_t)(n*4u));(void)p66_nv_data_prepare(&s,&io);for(phase=0;phase<12u;++phase){enum p66_bias_status status;switch(phase%4u){case 0:status=p66_nv_scale_select(&s,&io);break;case 1:status=dsky_scale_output(&s,&io,s.words[0117]==2u?DSKY_DP1OUT:DSKY_DP3OUT);break;case 2:status=p66_nv_decimal_row(&s,&io);break;default:status=p66_nv_next_row(&s,&io);break;}printf("%u %u %u",n,phase,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}}return 0;}
