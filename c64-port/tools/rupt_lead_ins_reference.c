/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original INTERRUPT_LEAD_INS oracle (both missions). */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include <string.h>
#include "rupt_lead_ins_cases.h"
static agc_t baseline;
static unsigned char executed[36u*1024u];
static unsigned physical(unsigned pc,unsigned fb){return pc>=06000u?3u*1024u+(pc&01777u):pc>=04000u?2u*1024u+(pc&01777u):(fb>>10)*1024u+(pc&01777u);}
/* argv: rope, lm|cm[, executed-words file]. Runs the unmodified rope from
 * each case entry until Z and FB first equal the continuation the native
 * routine left, or, when the native routine ended the interrupt, until
 * InIsr clears; prints the erasable image, SUPERBNK, the interrupt-enable
 * bit and InIsr. A native refusal is printed without a run. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind,zfixed,ended,cm;if(argc!=3&&argc!=4)return 2;cm=!strcmp(argv[2],"cm");
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<rli_cases(cm);++n){static apollo_overlay_state s,native;static rli_io io,nio;agc_word z,zfb,sb,irq;
  kind=rli_run(cm,&native,&nio,(uint16_t)n);
  if(kind==RLI_REFUSED){printf("%u %u\n",n,kind);continue;}
  z=native.words[5];zfb=native.words[4];zfixed=z>=02000u&&z<04000u;
  machine=baseline;rli_input(cm,&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  ended=io.hw.in_interrupt&&!nio.hw.in_interrupt;sb=cm?io.cm.superbank:io.lm.superbank;irq=cm?io.cm.interrupt_enabled:io.lm.interrupt_enabled;
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)sb;machine.AllowInterrupt=irq;machine.InIsr=io.hw.in_interrupt;kind=3;
  for(cycles=0;cycles<20000u;++cycles){
   if(!machine.PendFlag&&!machine.ExtraDelay){pc=get(RegZ);fb=get(RegFB)&076000u;if(pc>=02000u&&pc<010000u&&!(fb>=060000u&&(machine.OutputChannel7&0100)))executed[physical(pc,fb)]=1;}
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=get(RegFB)&076000u;
    if(ended?!machine.InIsr:pc==z&&(!zfixed||fb==zfb)){kind=RLI_RETURNED;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"rupt-lead-ins %s case %u timeout %05o (native %05o)\n",argv[2],n,pc,(unsigned)z);return 2;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
  printf(" %u %u %u\n",(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.AllowInterrupt,(unsigned)machine.InIsr);
 }
 /* Coverage only: the lead-ins other milestones compare (LM T6RUPT 04004 in
  * lm-t6rupt, T3RUPT 04014 in lm-exec-jobs and cm-exec-services) run from
  * their vector until Z leaves the lead-in words, so the executed-words file
  * covers every instruction of INTERRUPT_LEAD_INS.agc. */
 if(argc==4){static apollo_overlay_state s;static rli_io io;static const unsigned extra[2][2]={{04004u,04014u},{04014u,0u}};
  for(n=0;n<2u&&extra[cm][n];++n){machine=baseline;rli_input(cm,&s,&io,0);s.words[5]=(agc_word)extra[cm][n];
   for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InIsr=1;
   for(cycles=0;cycles<64u;++cycles){
    if(!machine.PendFlag&&!machine.ExtraDelay){pc=get(RegZ);if(pc<04000u||pc>=04100u)break;executed[physical(pc,0)]=1;}
    machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==64u){fprintf(stderr,"rupt-lead-ins %s coverage run from %05o did not leave the lead-ins\n",argv[2],extra[cm][n]);return 2;}}}
 if(argc==4){FILE *f=fopen(argv[3],"w");if(!f)return 2;for(i=0;i<36u*1024u;++i)if(executed[i])fprintf(f,"%u\n",i);fclose(f);}
 return 0;
}
