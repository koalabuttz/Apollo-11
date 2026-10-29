/* SPDX-License-Identifier: GPL-2.0-or-later; original post-snapshot data/control oracle. */
#define main executive_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_abort_word_cases.h"
#include "cm_abort_word_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ABORT_WORD_CASES;++n){apollo_overlay_state s;cm_native_io io;machine=baseline;cm_abort_word_input(&s,&io,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<100u;++cycles){if(cycles&&get(RegZ)==05655u&&!machine.PendFlag&&!machine.ExtraDelay)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==100u){fprintf(stderr,"abortword timeout%lu PC%o\n",(unsigned long)n,get(RegZ));return 2;}
 for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_abort_word_dump(n,EXEC_OK,&s,&io);
 }return 0;}
