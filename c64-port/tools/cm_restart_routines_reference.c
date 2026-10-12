/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 RESTARTS/NEWPHASE oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_restart_routines_cases.h"
static agc_t baseline;
static unsigned word(unsigned address){unsigned bank;
 if(address<02000u)return get(address);
 bank=address>=04000u?address/02000u:(unsigned)(get(4)>>10);
 return (unsigned)machine.Fixed[bank][address%02000u]&077777u;
}
static void load(const apollo_overlay_state *s){unsigned i;
 machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s->words[i];
 machine.InputChannel[7]=machine.OutputChannel7=0;machine.AllowInterrupt=0;
}
static void image(void){unsigned i;for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');}
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode;}
/* select: from each entry to TC GOLOC-1 (Z 0704) or the X.1 TC FINDVAC.
 * compose: from RESTARTS; at each service entry records the registers and
 * the 2CADR at Q, resumes at Q+2 without running the service; stops at
 * SWRETURN. newphase: from NEWPHASE (Q at the caller's group operand) to the
 * caller's L+2. Each prints all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,cycles,pc=0,i;apollo_overlay_state s;
 if(argc!=3)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[2]))return 2;baseline=machine;InhibitAlarms=1;
 if(!strcmp(argv[1],"select"))for(n=0;n<CM_RR_SELECT_CASES;++n){unsigned status;
  (void)cm_rr_select_input(&s,(uint16_t)n);load(&s);
  for(cycles=0;cycles<800;++cycles){pc=get(RegZ);if(cycles&&quiet()&&(pc==0704||(pc==CM_RR_FINDVAC&&get(RegQ)==03540)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==800){fprintf(stderr,"CM restart select %u timeout %05o\n",n,pc);return 2;}
  status=pc==CM_RR_FINDVAC?3u:get(0704)==CM_RR_LONGCALL?2u:get(0704)==CM_RR_WAITLIST?1u:0u;
  printf("%u %u",n,status);image();}
 else if(!strcmp(argv[1],"compose"))for(n=0;n<CM_RR_COMPOSE_CASES;++n){unsigned calls=0,log[4][6];
  (void)cm_rr_select_input(&s,cm_rr_compose_case((uint16_t)n));load(&s);
  for(cycles=0;cycles<2000u;++cycles){
   if(cycles&&quiet()){pc=get(RegZ);
    if(pc==CM_RR_SWRETURN)break;
    if(pc==CM_RR_FINDVAC||pc==CM_RR_NOVAC||pc==CM_RR_WAITLIST||pc==CM_RR_LONGCALL){unsigned q=get(RegQ);
     if(calls==4u){fprintf(stderr,"CM restart composition %u: too many calls\n",n);return 2;}
     log[calls][0]=pc;log[calls][1]=(unsigned)*memory(0)&0177777u;log[calls][2]=(unsigned)*memory(1)&0177777u;log[calls][3]=q;log[calls][4]=word(q);log[calls][5]=word(q+1u);++calls;
     *memory(RegZ)=(int16_t)(q+2u);continue;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000u){fprintf(stderr,"CM restart composition %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u",n,calls);for(i=0;i<calls;++i)printf(" %u %u %u %u %u %u",log[i][0],log[i][1],log[i][2],log[i][3],log[i][4],log[i][5]);image();}
 else if(!strcmp(argv[1],"newphase"))for(n=0;n<CM_RR_NEWPHASE_CASES;++n){unsigned ret;cm_native_io io;
  cm_rr_newphase_input(&s,&io,(uint16_t)n);ret=(unsigned)s.words[2]+1u;load(&s);machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<200u;++cycles){pc=get(RegZ);if(cycles&&quiet()&&pc==ret)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==200u){fprintf(stderr,"CM NEWPHASE %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u",n,(unsigned)machine.AllowInterrupt);image();}
 else return 2;
 return 0;
}
