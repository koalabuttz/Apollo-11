/* SPDX-License-Identifier: GPL-2.0-or-later; host-only yaAGC oracle for milestone standard-lead-oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "standard_lead_oracle_cases.h"
/* argv: Luminary rope, Comanche rope. For every case the native run reached
 * (a refusal prints "n 0"), runs the original from the same image, entry,
 * interrupt inhibit and SUPERBNK until Z first equals the native Z with the
 * native FB, and prints the inhibit flag, channel 7 and the 2048 erasable
 * words (A, L, Q 16-bit). */
int main(int argc,char **argv){static agc_t rope[2];static apollo_overlay_state s,native;uint16_t n,superbank,inhint,i;unsigned cycles;
 if(argc!=3)return 2;
 for(i=0;i<2u;++i){memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1+i]))return 2;rope[i]=machine;}
 InhibitAlarms=1;
 for(n=0;n<SLO_CASES;++n){
  if(!slo_run(&native,&superbank,&inhint,n)){printf("%u 0\n",(unsigned)n);continue;}
  machine=rope[slo_mission(n)];slo_input(&s,&superbank,&inhint,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)superbank;machine.AllowInterrupt=!inhint;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&get(5)==native.words[5]&&get(4)==native.words[4])break;
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==20000u){fprintf(stderr,"standard-lead-oracle case %u timeout\n",(unsigned)n);return 2;}
  printf("%u 1 %u %u",(unsigned)n,(unsigned)!machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));printf("\n");
 }
 return 0;}
