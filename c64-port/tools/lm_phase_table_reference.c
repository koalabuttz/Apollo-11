/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 PHASE_TABLE_MAINTENANCE oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_phase_table_cases.h"
#define LM_PHASE_SITE_LIST
#include "../src/lm_phase_sites.inc"
static agc_t baseline;
static unsigned word(unsigned address){unsigned bank;
 if(address<02000u)return get(address);
 if(address>=04000u)bank=address/02000u;
 else{bank=(unsigned)(get(4)>>10);if(bank>=030u&&(machine.OutputChannel7&0100))bank+=010u;}
 return (unsigned)machine.Fixed[bank][address%02000u]&077777u;
}
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode;}
/* From each case's entry, as its caller enters it, to the caller's return
 * (Z at the word after the inline operands, under the caller's FB) or
 * CHECKMM's Q+1/Q+2. At NOVAC records the registers and the 2CADR at Q,
 * then resumes at Q+2 without running NOVAC. Prints the interrupt-enable
 * state, the NOVAC record and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_PT_CASES;++n){apollo_overlay_state s;p66_bias_io io;unsigned k=n%lm_pt_site_count(),ret,fb,calls=0,log[5]={0,0,0,0,0};
  lm_pt_input(&s,&io,(uint16_t)n);
  ret=s.words[2]+(k<LM_PHASE_SITE_COUNT?lm_phase_site_list[k].words:0u);fb=s.words[4];
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<3000u;++cycles){
   if(cycles&&quiet()){pc=get(RegZ);
    if(s.words[5]==LM_PT_CHECKMM?(pc==LM_PT_Q_PLUS_1||pc==LM_PT_Q_PLUS_2):(pc==ret&&get(4)==fb))break;
    if(pc==LM_PT_NOVAC){unsigned q=get(RegQ);
     if(calls){fprintf(stderr,"LM phase table %u: second NOVAC\n",n);return 2;}
     calls=1;log[0]=(unsigned)*memory(0)&0177777u;log[1]=(unsigned)*memory(1)&0177777u;log[2]=q;log[3]=word(q);log[4]=word(q+1u);
     *memory(RegZ)=(int16_t)(q+2u);continue;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==3000u){fprintf(stderr,"LM phase table %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u %u",n,(unsigned)machine.AllowInterrupt,calls);if(calls)printf(" %u %u %u %u %u",log[0],log[1],log[2],log[3],log[4]);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
