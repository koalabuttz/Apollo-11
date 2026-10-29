/* SPDX-License-Identifier: GPL-2.0-or-later; original source host-only oracle. */
#define main executive_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_priority_display_cases.h"
#include "cm_priority_display_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,phase,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
for(n=0;n<CM_PRIORITY_DISPLAY_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned target,status,bankjump;machine=baseline;cm_priority_display_input(&s,&io,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
for(phase=0;phase<3u;++phase){bankjump=0;target=phase==0?05160u:03107u;
 for(cycles=0;cycles<1000u;++cycles){if(phase==2u&&get(RegZ)==04704u)bankjump=1;if(cycles&&((phase==2u&&bankjump&&get(RegZ)>=02000u&&get(RegZ)<04000u)||(phase<2u&&!machine.PendFlag&&!machine.ExtraDelay&&(get(RegZ)==target||get(RegZ)==07755u))))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==1000u){fprintf(stderr,"prioritydisplay timeout %lu phase%u PC%o target%o\n",(unsigned long)n,phase,get(RegZ),target);return 2;}status=get(RegZ)==07755u?(get(RegQ)==02642u?EXEC_NO_VAC:EXEC_NO_CORE):EXEC_OK;
 for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;if(status!=EXEC_OK)io.executive_alarm=(agc_word)(machine.Fixed[1][get(RegQ)&01777u]&077777u);cm_priority_display_dump(n,phase,status,&s,&io);if(status!=EXEC_OK)break;
}}return 0;}
