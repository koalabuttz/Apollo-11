/* SPDX-License-Identifier: GPL-2.0-or-later; host-only yaAGC oracle for milestone cm-lead-tests. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_lead_tests_cases.h"
/* argv: Comanche rope. For every case the native run reached (a refusal
 * prints "n 0"), runs the original from the same image until Z first equals
 * the native Z with the native FB, and prints the 2048 erasable words. */
int main(int argc,char **argv){static agc_t rope;static apollo_overlay_state s,native;uint16_t n,superbank,i;unsigned cycles,k;
 if(argc!=2)return 2;
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;rope=machine;
 InhibitAlarms=1;
 for(n=0;n<CLT_CASES;++n){
  k=clt_run(&native,&superbank,n);if(!k){printf("%u 0\n",(unsigned)n);continue;}
  machine=rope;clt_input(&s,&superbank,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)superbank;
  for(cycles=0;cycles<100u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&get(5)==native.words[5]&&get(4)==native.words[4])break;
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==100u){fprintf(stderr,"cm-lead-tests case %u timeout\n",(unsigned)n);return 2;}
  printf("%u 1",(unsigned)n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));printf("\n");
 }
 return 0;}
