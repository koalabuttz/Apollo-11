/* SPDX-License-Identifier: GPL-2.0-or-later
 * Independent original-source oracle; input constructors only, no native
 * numerical code linked. The GPL engine is a host-only test dependency. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "lm_torque_cases.h"
static void emit(unsigned w){putchar((int)(w&255u));putchar((int)(w>>8));}
int main(int argc,char **argv){static agc_t pristine;static apollo_overlay_state s;
 lm_torque_io io;unsigned n,i,cycles;unsigned long visits[1024]={0};FILE *coverage;
 if(argc!=2 || agc_load_binfile(&pristine,argv[1]))return 2;
 for(n=0;n<LM_TORQUE_CASE_COUNT;++n){memcpy(&machine,&pristine,sizeof machine);
 lm_torque_prepare(&s,&io,(uint16_t)n);InhibitAlarms=1;
 for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)s.words[i];
 machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=0;
 for(cycles=0;cycles<2000;++cycles){if(!machine.PendFlag && !machine.ExtraDelay){
 if(get(RegZ)==LM_TORQUE_BACKP)break;
 if(get(RegZ)>=02000 && get(RegZ)<04000)++visits[get(RegZ)-02000];}
 machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==2000){fprintf(stderr,"torque %u timeout %05o\n",n,get(RegZ));return 2;}
 emit(n);emit(1);for(i=0;i<APOLLO_SHARED_WORDS;++i)
 emit((unsigned)*memory(i)&(i<=2?0177777u:077777u));
 emit((unsigned)machine.OutputChannel7);emit((unsigned)machine.AllowInterrupt);}
 coverage=fopen("build/lm-torque-visits.json","w");if(!coverage)return 2;
 fprintf(coverage,"{");for(i=03641;i<=03734;++i)fprintf(coverage,"%s\"%05o\":%lu",i==03641?"":",",i,visits[i-02000]);fprintf(coverage,"}\n");fclose(coverage);
 return ferror(stdout)?2:0;}
