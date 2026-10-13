/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original INTERPRETER entry oracle (both missions). */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include <string.h>
#include "interp_entry_cases.h"
static agc_t baseline;
/* argv: rope, lm|cm. Runs the unmodified rope from each case entry until Z
 * first reaches NEWOPS; prints the erasable image, SUPERBNK and the
 * interrupt-enable bit. A native refusal is printed without a run. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,kind,cm,newops;if(argc!=3)return 2;cm=!strcmp(argv[2],"cm");
 newops=(cm?INTERP_ENTRY_CM_INTPRET:INTERP_ENTRY_LM_INTPRET)+031u;
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<IE_CASES;++n){static apollo_overlay_state s,native;static ie_io io,nio;agc_word sb,irq;
  kind=ie_run(cm,&native,&nio,(uint16_t)n);
  if(kind==IE_REFUSED){printf("%u %u\n",n,kind);continue;}
  machine=baseline;ie_input(cm,&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  sb=cm?io.cm.superbank:io.lm.superbank;irq=cm?io.cm.interrupt_enabled:io.lm.interrupt_enabled;
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)sb;machine.AllowInterrupt=irq;kind=3;
  for(cycles=0;cycles<200u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);if(pc==newops){kind=IE_RETURNED;break;}}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"interp-entry %s case %u timeout %05o\n",argv[2],n,pc);return 2;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
  printf(" %u %u\n",(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.AllowInterrupt);
 }
 return 0;
}
