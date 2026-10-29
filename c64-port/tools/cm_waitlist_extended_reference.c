/* SPDX-License-Identifier: GPL-2.0-or-later; full original CM ENDOFJOB and interpreter dispatch. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_waitlist_extended_cases.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles,pc;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_WAITLIST_EXTENDED_CASES;++n){apollo_overlay_state s;cm_native_io io;machine=baseline;cm_waitlist_extended_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.InputChannel[011]=io.channel11;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<2000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&(pc==05721u||pc==07755u||pc==(n<1024u?0707u:0706u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000u){fprintf(stderr,"CM finish timeout %u PC%05o BB%05o\n",n,pc,get(RegBB));return 2;}
  printf("%u %u",n,pc==05721u?(unsigned)CM_WL_BAD_DELAY:pc==07755u?(unsigned)CM_WL_FULL:(unsigned)CM_WL_OK);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt);
 }return 0;
}
