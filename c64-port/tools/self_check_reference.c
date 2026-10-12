/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original AGC_BLOCK_TWO_SELF-CHECK (bank 43) oracle, both missions. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include <stdlib.h>
#include "self_check_cases.h"
#include "case_trace.h"
#include "cert_trace.h"
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction;}
/* A whole-rope run ends at one of self_check_rope_ends(): ENDSUMS and
 * SDISPLAY in bank 43, NUCHANG2 and NUDIRECT in bank 01, CHANG1 and ALARM2. */
static int rope_end(unsigned mission,unsigned z,unsigned fb){unsigned i,count;const agc_word *ends=self_check_rope_ends(mission,&count);
 for(i=0;i<count;++i)if(z==ends[i]){
  if(i<2u)return (fb&076000u)==(033u<<10)&&(machine.OutputChannel7&0100);
  if(i<4u)return (fb&076000u)==(01u<<10);
  return 1;}
 return 0;
}
/* argv: mission (0 Luminary099, 1 Comanche055), that mission's rope, and
 * "rope" for the whole-rope cases. From each case's state, as
 * tests/self_check_cases.c sets it up, until self_check_leaves() (or, for a
 * whole-rope case, one of its ends). Prints the interrupt-enable state,
 * SUPERBNK and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,mission,rope,cases,limit,proceeds;agc_t baseline;
 if(argc<3)return 2;
 mission=(unsigned)atoi(argv[1])!=0u;rope=argc>3;cases=rope?SELF_CHECK_ROPE_CASES:SELF_CHECK_CASES;limit=rope?60000000u:200000u;
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[2]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<cases;++n){apollo_overlay_state s;self_check_io io;
  if(rope)self_check_rope_input(mission,&s,&io,(uint16_t)n);else self_check_input(mission,&s,&io,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)*self_check_superbank(mission,&io);machine.AllowInterrupt=*self_check_interrupts(mission,&io);
  proceeds=rope?self_check_rope_proceeds((uint16_t)n):0u;
  for(cycles=0;cycles<limit;++cycles){
   if(quiet()){pc=get(RegZ);
    if(proceeds&&pc==self_check_rope_ends(mission,0)[1]&&rope_end(mission,pc,get(4))){   /* SHOWSUM: SDISPLAY's LXCHs, TC NXTBNK */
     --proceeds;for(i=0;i<2048u;++i)s.words[i]=(agc_word)*memory(i);self_check_showsum_proceed(mission,&s);
     for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];continue;}
    if(rope?rope_end(mission,pc,get(4)):self_check_leaves(mission,(agc_word)pc,(agc_word)(get(4)&076000u),(agc_word)machine.OutputChannel7)){if(cycles)break;}
    else{CASE_MARK();CERT_MARK();}}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==limit){fprintf(stderr,"SELF-CHECK %u/%u timeout %05o\n",mission,n,pc);return 2;}
  CASE_END(n);
  printf("%u %u %u",n,(unsigned)machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
