/* SPDX-License-Identifier: GPL-2.0-or-later; uninterrupted actual original R52 caller. */
#define main executive_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_r52_priority_alarm_cases.h"
#include "cm_r52_priority_alarm_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned phase,i,cycles;static const unsigned target[7]={03766,02726,05160,03107,02202,05217,02770};if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
for(n=0;n<CM_R52_PRIORITY_ALARM_CASES;++n){apollo_overlay_state s;cm_native_io io;machine=baseline;cm_r52_priority_alarm_input(&s,&io,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(phase=0;phase<7u;++phase){unsigned status=EXEC_OK;for(cycles=0;cycles<1000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&((get(RegZ)==target[phase]&&(phase!=6u||get(RegFB)==020000u))||get(RegZ)==07755u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==1000u){fprintf(stderr,"r52 timeout%lu phase%u PC%o\n",(unsigned long)n,phase,get(RegZ));return 2;}
 if(get(RegZ)==07755u){status=get(RegQ)==02642u?EXEC_NO_VAC:EXEC_NO_CORE;io.executive_alarm=(agc_word)(machine.Fixed[1][get(RegQ)&01777u]&077777u);}
 for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_r52_priority_alarm_dump(n,phase,status,&s,&io);if(status!=EXEC_OK)break;
 }}return 0;}
