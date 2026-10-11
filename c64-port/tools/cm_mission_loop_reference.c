/* SPDX-License-Identifier: GPL-2.0-or-later; continuous original CM task ISR services. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_mission_loop_cases.h"
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
 for(n=0;n<CM_MISSION_LOOP_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_mission_loop r={&s,&io,0,0,{0,0},{{0,0},0},0,1,0};machine=baseline;cm_mission_loop_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=0;
  for(step=0;step<20u;++step){cm_mission_call c;unsigned caller=get(RegQ),bb=get(RegBB);enum cm_service_status status;
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;r.irq.in_isr=(uint8_t)machine.InIsr;r.irq.pending_danzig=(uint8_t)machine.SubstituteInstruction;
   status=cm_mission_identify(&r,&c);if(status!=CM_SERVICE_ADVANCED){fprintf(stderr,"CM mission unmapped %u/%u PC%05o BB%05o\n",n,step,get(RegZ),get(RegBB));return 2;}
   if(c.group==CM_MISSION_INTERRUPT)machine.InterruptRequests[3]=1;
   for(cycles=0;cycles<2000;++cycles){unsigned pc=get(RegZ);int ready=!machine.PendFlag&&!machine.ExtraDelay;
    if(cycles&&ready&&((c.group==CM_MISSION_INTERRUPT&&pc==03423u&&get(RegBB)==02003u)||(c.group==CM_MISSION_RESUME&&machine.SubstituteInstruction)||(c.group==CM_MISSION_DANZIG&&(pc==06037u||(pc==03332u&&get(RegBB)==050006u)||(pc==03542u&&get(RegBB)==014003u)))||(c.group<100u&&boundary(c.service.group,caller,bb))))break;
    machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==2000u){fprintf(stderr,"CM mission timeout %u/%u group%u PC%05o BB%05o\n",n,step,c.group,get(RegZ),get(RegBB));return 2;}
   if(c.group==CM_MISSION_INTERRUPT)r.pending_t3=0;
   if(c.group==CM_MISSION_DANZIG){r.transfer.interpretive=(uint8_t)(get(RegZ)==06037u);r.transfer.address.location=r.transfer.interpretive?get(0164):get(RegZ);r.transfer.address.bank=(agc_word)(get(RegBB)|(unsigned)machine.OutputChannel7);}
   printf("%u %u %u %u",n,step,c.group,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt,machine.InIsr,machine.SubstituteInstruction,r.transfer.interpretive,r.transfer.address.location,r.transfer.address.bank);
   if(c.group==CM_MISSION_DANZIG)break;
  }if(step==20u)return 2;
 }return 0;}
