/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_monitor_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){uint32_t n;unsigned i,c,pc,cmode,route;enum dsky_nv_mission mission;enum dsky_monitor_stage stage;dsky_charin_io io;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_MONITOR_CASES;++n){dsky_monitor_input(&s,&io,(uint16_t)n,&mission,&stage);cmode=mission==DSKY_NV_CM;machine=cmode?cm:lm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;machine.InputChannel[011]=io.channel11;route=99;
  for(c=0;c<150u;++c){pc=get(RegZ);if(c&&!machine.PendFlag&&!machine.ExtraDelay){
   if(stage==DSKY_MONITOR_SETUP){if(pc==(cmode?04516u:04502u))route=DSKY_MONITOR_RELEASE;else if(pc==03262u)route=DSKY_MONITOR_CYCLE_ENTRY;else if(pc==03263u)route=DSKY_MONITOR_RAW_ENTRY;}
   else if(stage==DSKY_MONITOR_DISPLAY){if(pc==(cmode?05217u:05155u))route=DSKY_MONITOR_END;else if(pc==03351u)route=DSKY_MONITOR_BUSY_ENTRY;else if(pc==02317u)route=DSKY_MONITOR_UPDATE;}
   else if(stage==DSKY_MONITOR_DISPLAY_AFTER&&pc==02046u)route=DSKY_MONITOR_TESTNN;
   else if(stage==DSKY_MONITOR_BUSY&&pc==(cmode?05217u:05155u))route=DSKY_MONITOR_END;
   else if(stage!=DSKY_MONITOR_DISPLAY_AFTER&&stage!=DSKY_MONITOR_BUSY){if(pc==0136u)route=DSKY_MONITOR_ENTRET;else if(pc==(cmode?05245u:05203u))route=DSKY_MONITOR_WAITLIST;else if(pc==(cmode?05134u:05072u))route=DSKY_MONITOR_NOVAC;else if(pc==(cmode?05324u:05261u))route=DSKY_MONITOR_TASKOVER;else if(pc==03275u&&(stage==DSKY_MONITOR_CYCLE||stage==DSKY_MONITOR_CYCLE_RAW))route=DSKY_MONITOR_REQUEST_ENTRY;}
   if(route!=99u)break;
  }machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(c==150u){fprintf(stderr,"monitor timeout n%lu stage%u PC%o\n",(unsigned long)n,(unsigned)stage,pc);return 2;}
  printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],stage==DSKY_MONITOR_BUSY?1u:io.wrote_channel11,route);
 }return 0;}
