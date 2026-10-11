/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Input-only host oracle, never linked into native code. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "sband_control_cases.h"
#include "sband_control_source.h"
static agc_t originals[2];
int main(int argc,char **argv){uint32_t n,start,end;unsigned i;if(argc<3||agc_load_binfile(&originals[0],argv[1])||agc_load_binfile(&originals[1],argv[2]))return 2;start=argc>3?(uint32_t)strtoul(argv[3],0,0):0;end=argc>4?(uint32_t)strtoul(argv[4],0,0):81920;InhibitAlarms=1;
 for(n=start;n<end;++n){apollo_overlay_state m;p66_bias_io io;enum apollo_fixed_mission mission;enum sband_control_entry entry;const sband_control_bindings *b;unsigned target,cycles;uint16_t row[2053];if(n/4096u==13u)continue;sband_control_input(&m,&io,n,&mission,&entry);b=&sband_control_source[mission];machine=originals[mission];for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.OutputChannel7=machine.InputChannel[7]=io.superbank;
 if(entry==SBAND_CONTROL_START)target=b->r02both;
 else if(entry==SBAND_CONTROL_INTERPRETER||entry==SBAND_CONTROL_REENTER)target=b->newops;
 else if(entry==SBAND_CONTROL_TERMINATE||entry==SBAND_CONTROL_PROCEED||entry==SBAND_CONTROL_RECYCLE)target=b->endofjob;
 else if(entry==SBAND_CONTROL_BLANK)target=b->blanket;
 else if(entry==SBAND_CONTROL_RESTART)target=mission==APOLLO_FIXED_LM?b->priochng:b->delayjob;
 else if(entry==SBAND_CONTROL_DISPLAY)target=(m.words[01044]&020u)?(mission==APOLLO_FIXED_LM?b->priochng:b->gomarkfr):b->endext;
 else target=b->gomarkfr;
 for(cycles=0;cycles<2000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(5)==target)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==2000u){fprintf(stderr,"control%u entry%d timeout %o target%o\n",n,entry,get(5),target);return 2;}
 row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=1;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<3u?0177777u:077777u));if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
