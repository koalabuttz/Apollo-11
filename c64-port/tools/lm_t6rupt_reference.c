/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 T6RUPT program oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_t6rupt_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each lm-t6rupt case entry until Z and FB
 * first equal the continuation the native routine left, or, when the native
 * routine ended the interrupt, until InIsr clears; prints the erasable
 * image, SUPERBNK, the interrupt-enable bit, InIsr and channels 5, 6 and
 * 13. A native refusal is printed without a run. With a second argument,
 * also writes there the fixed-memory words the original executed (physical
 * bank*1024+offset, one per line), for the source map. */
static unsigned char executed[36u*1024u];
static unsigned physical(unsigned pc,unsigned fb){return pc>=06000u?3u*1024u+(pc&01777u):pc>=04000u?2u*1024u+(pc&01777u):(fb>>10)*1024u+(pc&01777u);}
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind,zfixed,ended;if(argc!=2&&argc!=3)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_T6_CASES;++n){apollo_overlay_state s,native;p66_bias_io io,nio;agc_hardware hw,nhw;agc_word z,zfb;
  kind=lm_t6_run(&native,&nio,&nhw,(uint16_t)n);
  if(kind==LM_T6_REFUSED){printf("%u %u\n",n,kind);continue;}
  z=native.words[5];zfb=native.words[4];zfixed=z>=02000u&&z<04000u;
  machine=baseline;lm_t6_input(&s,&io,&hw,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  ended=hw.in_interrupt&&!nhw.in_interrupt;
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  machine.InputChannel[5]=(int16_t)hw.channel5;machine.InputChannel[6]=(int16_t)hw.channel6;machine.InputChannel[013]=(int16_t)hw.channel13;
  machine.InIsr=hw.in_interrupt;kind=3;
  for(cycles=0;cycles<20000u;++cycles){
   if(!machine.PendFlag&&!machine.ExtraDelay){pc=get(RegZ);fb=get(RegFB)&076000u;if(pc>=02000u&&pc<010000u&&!(fb>=060000u&&(machine.OutputChannel7&0100)))executed[physical(pc,fb)]=1;}
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=get(RegFB)&076000u;
    if(ended?!machine.InIsr:pc==z&&(!zfixed||fb==zfb)){kind=LM_T6_RETURNED;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"lm-t6rupt case %u timeout %05o (native %05o)\n",n,pc,(unsigned)z);return 2;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
  printf(" %u %u %u %u %u %u\n",(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.AllowInterrupt,(unsigned)machine.InIsr,
   (unsigned)machine.InputChannel[5]&077777u,(unsigned)machine.InputChannel[6]&077777u,(unsigned)machine.InputChannel[013]&077777u);
 }
 if(argc==3){FILE *f=fopen(argv[2],"w");if(!f)return 2;for(i=0;i<36u*1024u;++i)if(executed[i])fprintf(f,"%u\n",i);fclose(f);}
 return 0;
}
