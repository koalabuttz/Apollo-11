/* SPDX-License-Identifier: GPL-2.0-or-later; original mixed INITDSP source phases. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_init_display_mixed_cases.h"
#include "cm_fixed_compat_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,phase,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_INIT_DISPLAY_MIXED_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned winner=(n/2u)%7u,interpreter=(n&1u)&&winner,target,bank,super;machine=baseline;cm_init_display_mixed_input(&s,&io,n);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(phase=0;phase<3u;++phase){if(phase==0u){target=05210;bank=020000u|((s.words[01071]&03400u)>>8);super=s.words[0366]&0160u;}else if(phase==1u){target=interpreter?06037u:winner?s.words[0164u+winner*12u]:03267u;bank=winner?s.words[0165u+winner*12u]:(020000u|((s.words[01071]&03400u)>>8)|(s.words[0366]&0160u));super=bank&0160u;}else{unsigned cadr=s.words[0374]-3u;target=02000u+(cadr&01777u);bank=(cadr&076000u)|((s.words[01071]&03400u)>>8);super=s.words[0366]&0160u;}
   for(cycles=0;cycles<1000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target&&get(RegBB)==(bank&076007u)&&machine.OutputChannel7==super)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==1000u){fprintf(stderr,"CM INITDSP timeout%lu phase%u PC%05o BB%05o\n",(unsigned long)n,phase,get(RegZ),get(RegBB));return 2;}
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_fixed_dump(n,(uint16_t)phase,EXEC_OK,&s,&io);if(phase==1u&&winner)break;
  }
 }return 0;}
