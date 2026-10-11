/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "nv_render_check.h"
#include "dsky_scale.h"
static apollo_overlay_state s,before;
static enum p66_bias_status invoke(uint8_t stage,p66_bias_io *io){if(stage==0u)return p66_nv_scale_select(&s,io);if(stage==2u)return p66_nv_decimal_row(&s,io);return p66_nv_next_row(&s,io);}
int main(void){unsigned f=nv_render_check();uint8_t stage,k;p66_bias_io io,old;for(stage=0;stage<4u;++stage){if(stage==1u)continue;for(k=0;k<10u;++k){nv_rows_input(&s,&io,0);(void)p66_nv_data_prepare(&s,&io);if(stage>=2u){(void)p66_nv_scale_select(&s,&io);(void)dsky_scale_output(&s,&io,DSKY_DP1OUT);}if(stage==3u)(void)p66_nv_decimal_row(&s,&io);
 switch(k){case 0:io.superbank=0;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=1;break;case 3:s.words[4]=0;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:if(stage==0u)s.words[0140]=1;else if(stage==2u)s.words[01023]=0;else s.words[0117]=3;break;case 8:if(stage==0u)s.words[0147]=0;else if(stage==2u)s.words[0777]=0;else s.words[0117]=3;break;default:io.superbank=077777;break;}
 before=s;old=io;if(invoke(stage,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++f;
 }}printf("64 original rendering boundary images and30 invalid inputs: %s\n",f?"FAIL":"PASS");return f?1:0;}
