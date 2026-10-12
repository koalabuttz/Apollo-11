/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 idle loop, SUPDXCHZ and SVCT3 oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_exec_idle_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each cm-exec-idle case entry until Z and FB
 * first equal the continuation the native routine left (kind 0), or Z reaches
 * BAILOUT or POODOO (kind 1), and prints the erasable image, SUPERBNK, the
 * interrupt-enable bit and channel 11. The native continuation only picks
 * where to compare; every word must then agree. A native refusal (kind 2) is
 * printed without a run. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind,zfixed;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_EXI_CASES;++n){apollo_overlay_state s,native;cm_native_io io,nio;agc_word z,zfb;
  kind=cm_exi_run(&native,&nio,(uint16_t)n);
  if(kind==CM_EXI_REFUSED){printf("%u %u\n",n,kind);continue;}
  z=native.words[5];zfb=native.words[4];zfixed=z>=02000u&&z<04000u;
  machine=baseline;cm_exi_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  machine.InputChannel[011]=(int16_t)io.channel11;kind=3;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=get(RegFB)&076000u;
    if(pc==CM_EXEC_BAILOUT||pc==CM_EXEC_POODOO){kind=CM_EXI_ALARM;break;}
    if(pc==z&&(!zfixed||fb==zfb)){kind=CM_EXI_RETURNED;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"cm-exec-idle case %u timeout %05o (native %05o)\n",n,pc,(unsigned)z);return 2;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
  printf(" %u %u %u\n",(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011]&077777u);
 }return 0;
}
