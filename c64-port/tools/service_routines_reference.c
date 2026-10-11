/* SPDX-License-Identifier: GPL-2.0-or-later; independent host oracle only. */
#define main prior_reference_main
#include "reference_runner.c"
#undef main
#include "service_routines_cases.h"
#include "service_routines_source.h"
#include "service_routines_dump.h"
static agc_t pristine[2];
int main(int argc,char **argv){uint32_t n,start=argc>3?(uint32_t)strtoul(argv[3],0,0):0,end=argc>4?(uint32_t)strtoul(argv[4],0,0):SERVICE_ROUTINES_CASES;unsigned i,cycles,target,q;bool seenccs;apollo_overlay_state s;p66_bias_io io;enum service_routines_mission mission;enum service_routines_entry entry;agc_word fixed[3];const service_routines_bindings *b;
 if(argc<3||agc_load_binfile(&pristine[0],argv[1])||agc_load_binfile(&pristine[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=start;n<end;++n){service_routines_input(&s,&io,n,&mission,&entry,fixed);b=&service_routines_source[mission];q=s.words[2];seenccs=false;target=0;if(entry==SERVICE_UPENT2||entry==SERVICE_DOWNENT2||entry==SERVICE_UPFLAG||entry==SERVICE_DOWNFLAG)target=q+1u;else if(entry==SERVICE_TRFAILOF||entry==SERVICE_TRFAILON)target=q;
  machine=pristine[mission];for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.OutputChannel7=machine.InputChannel[7]=io.superbank;
  for(cycles=0;cycles<4000u;++cycles){unsigned z=get(RegZ);if(z==b->gentran+12u)seenccs=true;if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&((target&&z==target)||((entry==SERVICE_GENTRAN||entry==SERVICE_GENTRAN_LOOP)&&(seenccs&&((service_routines_kind(n)<24u&&z==b->gentran+1u)||z==q+2u||z==b->endofjob)))||(entry==SERVICE_DELAYJOB&&(z==b->dly2-1u||z==b->bailout||z==b->bailout1||z==01104u))||(entry==SERVICE_TCGETCAD&&(z==b->jobsleep||z==b->jobwake))||(entry==SERVICE_WAKER&&z==b->jobwake)||((entry==SERVICE_WAKER_FINISH||entry==SERVICE_B5OFF)&&z==(entry==SERVICE_B5OFF?b->endofjob:b->taskover))))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==4000u){fprintf(stderr,"service timeout%lu entry%u PC%05o BB%05o\n",(unsigned long)n,(unsigned)entry,get(RegZ),get(RegBB));return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;service_routines_dump(n,(uint16_t)service_routines_kind(n),0,&s,&io);
 }return ferror(stdout)?2:0;}
