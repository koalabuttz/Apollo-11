/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 Q_R-AXIS_RCS_AUTOPILOT oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_q_r_axis_cases.h"
static agc_t baseline;
/* Whether Z (with FB) is in this file: 17,2072-3251, MAKERUPT 07754-07755,
 * the central registers (a TC Q on its way to the word in Q, or EDRUPT's
 * vector to A), and RETJADR when LOOPER's TC RETJADR runs one of the file's
 * three return addresses there. */
static int in_file(unsigned pc,unsigned fb){
 if(pc<010u||pc==LM_QR_MAKERUPT||pc==LM_QR_MAKERUPT+1u)return 1;
 if(pc==LM_QR_RETJADR){unsigned w=(unsigned)*memory(03077u) /* RETJADR under EBANK6 */&077777u;return w==02273u||w==02532u||w==02671u;}
 return pc>=LM_QR_FIRST&&pc<=LM_QR_LAST&&fb==LM_QR_FB17;
}
/* Runs the unmodified rope from each lm-q-r-axis case entry until Z leaves
 * the file at an instruction boundary, and prints the outcome (LM_QR_NONE
 * after 40000 cycles), channel 13, the interrupt-enable and
 * interrupt-in-progress bits and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,kind;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_QR_CASES;++n){apollo_overlay_state s;lm_flight_io io;
  machine=baseline;lm_qr_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=0;machine.AllowInterrupt=(int)io.bias.interrupt_enabled;machine.InIsr=0;kind=LM_QR_NONE;
  machine.InputChannel[4]=(int16_t)io.hw.channel4;machine.InputChannel[5]=(int16_t)io.hw.channel5;
  machine.InputChannel[013]=(int16_t)io.hw.channel13;machine.InputChannel[031]=(int16_t)io.hw.channel31;
  for(cycles=0;cycles<40000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode
      &&!in_file((unsigned)get(RegZ),(unsigned)get(4)&076000u)){kind=LM_QR_LEFT;break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  printf("%u %u %u %u %u",n,kind,(unsigned)machine.InputChannel[013]&077777u,(unsigned)(machine.AllowInterrupt!=0),
   (unsigned)(machine.InIsr!=0));
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;
}
