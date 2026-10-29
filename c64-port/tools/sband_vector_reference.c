/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Host-only independent original engine fixture; never linked into native C64. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "sband_vector_cases.h"
static agc_t pristine;static uint8_t seen[010000];
int main(int argc,char **argv){uint32_t n,limit;enum apollo_fixed_mission mission;if(argc<3||agc_load_binfile(&pristine,argv[1]))return 2;mission=argv[2][0]=='C'?APOLLO_FIXED_CM:APOLLO_FIXED_LM;limit=argc>3?(uint32_t)strtoul(argv[3],0,0):131072u;InhibitAlarms=1;
 for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq,super,factor[6];uint16_t row[2053];unsigned i,cycles,done=mission==APOLLO_FIXED_CM?06030u:06061u;sband_vector_input(&m,&irq,&super,n,mission);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];if((n&65536u)&&(n&32767u)>=256u){unsigned bank=mission==APOLLO_FIXED_CM?026u:023u,off=m.words[0116]-02000u;sband_vector_fixed_input(factor,n);for(i=0;i<6u;++i)machine.Fixed[bank][off+i]=(int16_t)factor[i];}
 machine.AllowInterrupt=irq;machine.InputChannel[7]=machine.OutputChannel7=super;
 for(cycles=0;cycles<8000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(5)==done)break;if(!machine.PendFlag&&!machine.ExtraDelay)seen[get(5)]=1;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==8000u){fprintf(stderr,"cross %u timeout %o\n",n,get(5));return 2;}
 row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=1;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
 {FILE *f=fopen(mission==APOLLO_FIXED_CM?"build/sband-vector-CM-reference.trace":"build/sband-vector-LM-reference.trace","w");unsigned i;if(!f)return 2;for(i=0;i<010000u;++i)if(seen[i])fprintf(f,"%05o\n",i);if(fclose(f))return 2;}return 0;}
