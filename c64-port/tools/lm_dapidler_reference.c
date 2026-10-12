/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 DAPIDLER_PROGRAM oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_dapidler_cases.h"
#include "lm_exec_routines.h"
static agc_t baseline;
/* Runs the unmodified rope from each lm-dapidler case entry to RESUME, the
 * IBNKCALL target QERRCALC, CALCPERR or ZATTEROR, BAILOUT1/POODOO1 or CHEKBITS's
 * return, and prints the outcome, channels 5, 6, 12, 14 and all 2048
 * erasable words. The CDU command counters are printed as the program left
 * them when it reached CAF 13,14,15 (20,2425) before WOR CHAN14: the
 * channel 14 drive bits it then sets start yaAGC's coarse-alignment pulse
 * output, which empties those counters into the IMU outside the program. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_DI_CASES;++n){apollo_overlay_state s;lm_flight_io io;lm_di_case c;int drive=0;unsigned cmd[3]={0,0,0};
  machine=baseline;lm_di_input(&s,&io,&c,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=0;kind=LM_DI_NONE;
  machine.InputChannel[05]=(int16_t)io.chan5;machine.InputChannel[06]=(int16_t)io.chan6;machine.InputChannel[012]=(int16_t)io.chan12;
  machine.InputChannel[014]=(int16_t)io.chan14;machine.InputChannel[030]=(int16_t)io.chan30;machine.InputChannel[031]=(int16_t)io.chan31;
  for(cycles=0;cycles<40000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(pc==c.stop_z&&(c.stop_fb==0177777u||fb==c.stop_fb)){kind=LM_DI_RETURNED;break;}
    if(pc==LM_EXEC_BAILOUT1||pc==LM_EXEC_POODOO1){kind=LM_DI_ALARM;break;}
    if(pc==LM_DI_RESUME){kind=LM_DI_RESUMED;break;}
    if(pc==LM_DI_QERRCALC_Z&&fb==036000u){kind=LM_DI_QERRCALC;break;}
    if(pc==LM_DI_CALCPERR_Z&&fb==LM_DI_FB16){kind=LM_DI_CALCPERR;break;}
    if(pc==LM_DI_ZATTEROR_Z&&fb==LM_DI_FB20){kind=LM_DI_ZATTEROR;break;}
    if(pc==02425u&&fb==LM_DI_FB20){drive=1;for(i=0;i<3u;++i)cmd[i]=(unsigned)*memory(050u+i)&077777u;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==LM_DI_NONE){fprintf(stderr,"lm-dapidler case %u timeout %05o\n",n,pc);return 2;}
  if(drive)for(i=0;i<3u;++i)*memory(050u+i)=(int16_t)cmd[i];
  printf("%u %u %u %u %u %u",n,kind,(unsigned)machine.InputChannel[05]&077777u,(unsigned)machine.InputChannel[06]&077777u,
         (unsigned)machine.InputChannel[012]&077777u,(unsigned)machine.InputChannel[014]&077777u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;
}
