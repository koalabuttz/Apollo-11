/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 Executive/Waitlist service oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_exec_services_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each cm-exec-services case entry until Z
 * reaches one of the case's stops or the interrupt ends (kind 0), BAILOUT or
 * POODOO (kind 1), or the constant words DPBIT14 in LONGCYCL (01,3476 or
 * 01,3477) or the erasable word 01203 (kind 2), and prints the erasable
 * image, SUPERBNK, the interrupt-enable bit and InIsr (no image for kind 2). */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_EXS_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_exs_case c;
  machine=baseline;cm_exs_input(&s,&io,&c,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  machine.InIsr=c.hw.in_interrupt;kind=3;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=get(RegFB)&076000u;
    if(pc==CM_EXEC_BAILOUT||pc==CM_EXEC_POODOO){kind=CM_EXS_ALARM;break;}
    if(((pc==03476u||pc==03477u)&&fb==02000u)||pc==01203u){kind=CM_EXS_REFUSED;break;}
    if(c.resume&&!machine.InIsr){kind=CM_EXS_RETURNED;break;}
    for(i=0;i<c.stops&&!(pc==c.stop_z[i]&&(c.stop_fb[i]==CM_EXS_ANY_FB||fb==c.stop_fb[i]));++i){}
    if(i<c.stops){kind=CM_EXS_RETURNED;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"cm-exec-services case %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u",n,kind);
  if(kind!=CM_EXS_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
   printf(" %u %u %u",(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.AllowInterrupt,(unsigned)machine.InIsr);}
  putchar('\n');
 }return 0;
}
