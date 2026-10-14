/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 T4RUPT interrupt (chained) oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include <string.h>
#include "cm_t4rupt_chain_cases.h"
static agc_t baseline;
static unsigned char executed[36u*1024u];
static const unsigned channels[9]={010,011,012,013,014,030,031,032,033};
static unsigned physical(unsigned pc,unsigned fb){return pc>=06000u?3u*1024u+(pc&01777u):pc>=04000u?2u*1024u+(pc&01777u):(fb>>10)*1024u+(pc&01777u);}
/* argv: rope[, executed-words file]. Runs the unmodified rope from T4RUPT
 * until Z first equals the continuation the native trampoline run stopped at
 * (and FB when Z is banked); prints the erasable image, SUPERBNK, the
 * interrupt-enable bit, InIsr and channels 10-14 and 30-33. A native
 * refusal is printed without a run. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind;if(argc!=2&&argc!=3)return 2;
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CCH_CASES;++n){static apollo_overlay_state s,native;static cch_io io,nio;uint16_t steps;agc_word z;
  kind=cch_run(&native,&nio,(uint16_t)n,&steps);
  if(kind==CCH_REFUSED){printf("%u %u\n",n,kind);continue;}
  z=native.words[5];
  machine=baseline;cch_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)io.cm.io.superbank;machine.AllowInterrupt=io.cm.io.interrupt_enabled;machine.InIsr=io.hw.in_interrupt;
  machine.InputChannel[010]=(int16_t)io.hw.channel10;machine.InputChannel[011]=(int16_t)io.hw.channel11;machine.InputChannel[012]=(int16_t)io.hw.channel12;
  machine.InputChannel[013]=(int16_t)io.hw.channel13;machine.InputChannel[014]=(int16_t)io.hw.channel14;
  machine.InputChannel[030]=(int16_t)io.hw.channel30;machine.InputChannel[031]=(int16_t)io.hw.channel31;machine.InputChannel[032]=(int16_t)io.hw.channel32;
  machine.InputChannel[033]=(int16_t)io.hw.channel33;kind=3;
  for(cycles=0;cycles<60000u;++cycles){
   if(!machine.PendFlag&&!machine.ExtraDelay){pc=get(RegZ);fb=get(RegFB)&076000u;if(pc>=02000u&&pc<010000u&&!(fb>=060000u&&(machine.OutputChannel7&0100)))executed[physical(pc,fb)]=1;}
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&get(RegZ)==z&&(z>=04000u||(get(RegFB)&076000u)==native.words[4])){kind=CCH_RETURNED;break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"cm-t4rupt-chain case %u timeout %05o (native %05o)\n",n,pc,(unsigned)z);return 2;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
  printf(" %u %u %u",(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.AllowInterrupt,(unsigned)machine.InIsr);
  for(i=0;i<9u;++i)printf(" %u",(unsigned)machine.InputChannel[channels[i]]&077777u);
  putchar('\n');
 }
 if(argc==3){FILE *f=fopen(argv[2],"w");if(!f)return 2;for(i=0;i<36u*1024u;++i)if(executed[i])fprintf(f,"%u\n",i);fclose(f);}
 return 0;
}
