/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_monitor_consumer_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
/* Only known compiled observation boundaries. No native computation executes
 * in this oracle; the original machine proceeds continuously between them. */
static unsigned boundary(unsigned pc,unsigned fb,unsigned m)
{static const unsigned scalar[2][6]={{02615,02677,02704,02623,02635,0},{02516,02603,02610,02524,0,02653}},dp[2][3]={{02710,02715,02717},{02614,02621,02623}};unsigned i;if(fb==062000u){switch(pc){case 03321:return 0;case 02317:return 1;case 03333:return 2;case 02046:return 3;case 02221:return 4;case 02120:return 5;case 02133:return 6;case 02365:case 02373:case 02400:case 02360:case 02353:return 7;case 02504:return 8;case 03363:return 9;case 02422:return 10;case 02523:return 11;case 02537:return 12;case 02566:return 15;case 0136:return 18;case 02337:return 19;case 03351:return 23;case 02351:case 02064:return 24;default:break;}if(pc==(m?04675u:04631u))return 20;if(pc==(m?04271u:04255u))return 21;if(pc==(m?04156u:04142u)||pc==(m?04157u:04143u)||pc==(m?05217u:05155u))return pc==(m?05217u:05155u)?24u:22u;}
 if(fb==060000u){if(pc==(m?03133u:03226u))return 16;if(pc==(m?04675u:04631u))return 17;if(pc==(m?05747u:05705u))return 24;for(i=0;i<6u;++i)if(scalar[m][i]&&pc==scalar[m][i])return 13;for(i=0;i<3u;++i)if(pc==dp[m][i])return 14;}return 99;}
int main(int argc,char **argv){unsigned n,step,stage,next,cycles,i,pc,fb,m;enum dsky_nv_mission mission;dsky_charin_io io;agc_word channel;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<256u;++n){dsky_monitor_consumer_input(&s,&io,(uint16_t)n,&mission,&channel);m=mission==DSKY_NV_CM;machine=m?cm:lm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;machine.InputChannel[011]=io.channel11;machine.InputChannel[033]=channel;stage=0;
  for(step=0;step<100u;++step){for(cycles=0;cycles<8000u;++cycles){pc=get(RegZ);fb=get(RegFB);next=boundary(pc,fb,m);if(cycles&&next!=99u&&next!=stage&&!machine.PendFlag&&!machine.ExtraDelay)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==8000u){fprintf(stderr,"monitorconsumer timeout n%u step%u stage%u PC%oFB%o\n",n,step,stage,pc,fb);return 2;}if(stage==23u)io.wrote_channel11=1;
   printf("%u %u %u",n,step,stage);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],io.wrote_channel11,next);if(next==24u)break;stage=next;
  }if(step==100u)return 2;
 }return 0;}
