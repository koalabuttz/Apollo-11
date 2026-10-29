/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 RESTARTS composition oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_restart_routines_cases.h"
static agc_t baseline;
static unsigned word(unsigned address){unsigned bank;
 if(address<02000u)return get(address);
 bank=address>=04000u?address/02000u:(unsigned)(get(4)>>10);
 return (unsigned)machine.Fixed[bank][address%02000u]&077777u;
}
/* Runs the unmodified rope from RESTARTS. At each service entry (FINDVAC,
 * NOVAC, WAITLIST, LONGCALL) records the registers and the 2CADR at Q, then
 * resumes at Q+2 without running the service; stops at SWRETURN. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,calls;unsigned log[4][6];if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_RSR_CASES;++n){apollo_overlay_state s;
  machine=baseline;(void)lm_rsselect_input(&s,lm_rsr_select_case((uint16_t)n));for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=0;machine.AllowInterrupt=0;calls=0;
  for(cycles=0;cycles<2000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);
    if(pc==LM_RS_SWRETURN)break;
    if(pc==LM_RS_FINDVAC||pc==LM_RS_NOVAC||pc==LM_RS_WAITLIST||pc==LM_RS_LONGCALL){unsigned q=get(RegQ);
     if(calls==4u){fprintf(stderr,"LM restart composition %u: too many calls\n",n);return 2;}
     log[calls][0]=pc;log[calls][1]=(unsigned)*memory(0)&0177777u;log[calls][2]=(unsigned)*memory(1)&0177777u;log[calls][3]=q;log[calls][4]=word(q);log[calls][5]=word(q+1u);++calls;
     *memory(RegZ)=(int16_t)(q+2u);continue;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000u){fprintf(stderr,"LM restart composition %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u",n,calls);for(i=0;i<calls;++i)printf(" %u %u %u %u %u %u",log[i][0],log[i][1],log[i][2],log[i][3],log[i][4],log[i][5]);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
