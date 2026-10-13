/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original T4RUPT core oracle (both missions). */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include <string.h>
#include "t4rupt_core_cases.h"
static agc_t baseline;
static unsigned char executed[36u*1024u];
static unsigned physical(unsigned pc,unsigned fb){return pc>=06000u?3u*1024u+(pc&01777u):pc>=04000u?2u*1024u+(pc&01777u):(fb>>10)*1024u+(pc&01777u);}
/* argv: rope, lm|cm[, executed-words file]. Runs the unmodified rope from
 * each case entry until Z first equals the continuation the native routine
 * left, and FB when Z is in banked memory; prints the erasable image,
 * SUPERBNK, the interrupt-enable bit, InIsr and channel 10.
 * A native refusal is printed without a run. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind,cm;if(argc!=3&&argc!=4)return 2;cm=!strcmp(argv[2],"cm");
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<t4_cases(cm);++n){static apollo_overlay_state s,native;static t4_io io,nio;agc_word z,sb,irq;
  kind=t4_run(cm,&native,&nio,(uint16_t)n);
  if(kind==T4_REFUSED){printf("%u %u\n",n,kind);continue;}
  z=native.words[5];
  machine=baseline;t4_input(cm,&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  sb=cm?io.cm.superbank:io.lm.superbank;irq=cm?io.cm.interrupt_enabled:io.lm.interrupt_enabled;
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)sb;machine.AllowInterrupt=irq;machine.InIsr=io.hw.in_interrupt;
  machine.InputChannel[010]=(int16_t)io.hw.channel10;machine.InputChannel[032]=(int16_t)io.hw.channel32;kind=3;
  for(cycles=0;cycles<20000u;++cycles){
   if(!machine.PendFlag&&!machine.ExtraDelay){pc=get(RegZ);fb=get(RegFB)&076000u;if(pc>=02000u&&pc<010000u&&!(fb>=060000u&&(machine.OutputChannel7&0100)))executed[physical(pc,fb)]=1;}
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&get(RegZ)==z&&(z>=04000u||(get(RegFB)&076000u)==native.words[4])){kind=T4_RETURNED;break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"t4rupt-core %s case %u timeout %05o (native %05o)\n",argv[2],n,pc,(unsigned)z);return 2;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
  printf(" %u %u %u %u\n",(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.AllowInterrupt,(unsigned)machine.InIsr,(unsigned)machine.InputChannel[010]&077777u);
 }
 if(argc==4){FILE *f=fopen(argv[3],"w");if(!f)return 2;for(i=0;i<36u*1024u;++i)if(executed[i])fprintf(f,"%u\n",i);fclose(f);}
 return 0;
}
