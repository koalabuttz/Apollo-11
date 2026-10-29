/* SPDX-License-Identifier: GPL-2.0-or-later; continuous original CM task ISR services. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_service_loop_cases.h"
static agc_t baseline;
static int boundary(enum cm_service_group g,unsigned caller,unsigned bb){unsigned pc=get(RegZ),bank=get(RegBB);
 switch(g){
 case CM_SERVICE_T3_BEGIN:return (pc==03207&&bank==032006)||(pc==03260&&bank==050007)||(pc==05306&&bank==004003);
 case CM_SERVICE_TASK_FINISH:return pc==05341;
 case CM_SERVICE_SETJTAG:case CM_SERVICE_CLOCK_ENTRY:return pc==05324||pc==05245;
 case CM_SERVICE_CLOCK_RETURN:return pc==05324||pc==05134||pc==05147;
 case CM_SERVICE_SVCT3_ENTRY:return pc==05324||pc==05134||pc==05267;
 case CM_SERVICE_SVCT3_RETURN:case CM_SERVICE_FIXDELAY:return pc==05324&&bank==bb;
 case CM_SERVICE_WAITLIST:case CM_SERVICE_NOVAC:case CM_SERVICE_FINDVAC:return pc==caller+2u&&bank==bb;
 default:return 0;
 }
}
int main(int argc,char **argv){unsigned n,i,step,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_SERVICE_LOOP_CASES;++n){apollo_overlay_state s;cm_native_io io;machine=baseline;cm_service_loop_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=1;
  for(step=0;step<16u;++step){cm_service_call c;unsigned caller=get(RegQ),bb=get(RegBB);enum cm_service_status status;
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;
   status=cm_service_identify(&s,&io,&c);if(status!=CM_SERVICE_ADVANCED&&status!=CM_SERVICE_RESUME){fprintf(stderr,"CM runtime unmapped %u/%u PC%05o BB%05o\n",n,step,get(RegZ),get(RegBB));return 2;}
   if(status==CM_SERVICE_ADVANCED){for(cycles=0;cycles<2000;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&boundary(c.group,caller,bb))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
    if(cycles==2000){fprintf(stderr,"CM runtime timeout %u/%u group%u PC%05o BB%05o\n",n,step,(unsigned)c.group,get(RegZ),get(RegBB));return 2;}}
   printf("%u %u %u %u",n,step,(unsigned)c.group,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt);
   if(status==CM_SERVICE_RESUME)break;
  }if(step==16u)return 2;
 }return 0;
}
