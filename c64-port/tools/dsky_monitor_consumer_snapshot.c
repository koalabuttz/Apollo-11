/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_monitor_consumer_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,step;enum dsky_nv_mission m;enum dsky_monitor_consumer_stage stage,next;dsky_charin_io io;agc_word channel;for(n=0;n<256u;++n){dsky_monitor_consumer_input(&s,&io,n,&m,&channel);stage=DSKY_MONC_DISPLAY;for(step=0;step<100u;++step){if(dsky_monitor_consumer(&s,&io,m,stage,channel,&next)!=P66_BIAS_OK){fprintf(stderr,"reject n%u step%u stage%u PC%o FB%o\n",n,step,(unsigned)stage,s.words[5],s.words[4]);return 2;}printf("%u %u %u",n,step,(unsigned)stage);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.wrote_channel11,(unsigned)next);if(next==DSKY_MONC_FRONTIER)break;stage=next;}if(step==100u)return 2;}return 0;}
