/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original SERVICER oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_servicer_cases.h"
/* Runs the unmodified Luminary099 rope (argv[1]) from each lm-servicer
 * case's image, channels and interrupt state until lmsv_leaves() and prints
 * the case, kind 0, the 2048 erasable words, channels 11, 12, 32 and 33
 * and the interrupt-enable state. The scalers stay frozen (ScalerCounter
 * 0).
 *
 * With a segment file (argv[2], a C64 group of src/lm_servicer.c: a line
 * of the group's key slots, then "B ph" for each instruction and "I ph"
 * for each order-pair or store word of the file outside the group), it
 * runs only those slots' cases and also stops before such an instruction
 * (Z there) or order pair (NEWOPS with LOC there), as the group build does. */
static unsigned char slot_in[LMSV_ROUTINES],seg_b[32768],seg_i[32768];
static int segment_end(unsigned z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset){uint16_t ph;
 if(z==06070u){ph=agc_fixed_physical(loc,bankset,superbank);return ph!=AGC_TRANSFER_NONE&&seg_i[ph];}
 if(z<02000u||z>=04000u)return 0;
 ph=agc_fixed_physical((agc_word)z,fb,superbank);return ph!=AGC_TRANSFER_NONE&&seg_b[ph];}
int main(int argc,char **argv){unsigned n,i;unsigned long cycles;agc_t baseline;int seg=0;InhibitAlarms=1;
 if(argc!=2&&argc!=3)return 2;
 if(argc==3){FILE *f=fopen(argv[2],"r");char k[2];unsigned v;int ch;if(!f)return 2;seg=1;
  while(fscanf(f,"%u",&v)==1){if(v>=LMSV_ROUTINES)return 2;slot_in[v]=1;if((ch=fgetc(f))=='\n')break;}
  while(fscanf(f,"%1s %u",k,&v)==2){if(v>=32768u)return 2;if(k[0]=='B')seg_b[v]=1;else seg_i[v]=1;}
  fclose(f);}
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;
 for(n=0;n<LMSV_CASES;++n){apollo_overlay_state s;lmsv_case c;unsigned pc=0,stopped=0;
  if(seg&&!slot_in[n%LMSV_ROUTINES])continue;
  lmsv_input(&s,&c,(uint16_t)n);
  machine=baseline;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=c.interrupt_enabled;
  machine.InputChannel[011]=(int16_t)c.chan11;machine.InputChannel[012]=(int16_t)c.chan12;
  machine.InputChannel[032]=(int16_t)c.chan32;machine.InputChannel[033]=(int16_t)c.chan33;
  for(cycles=0;cycles<4000000ul;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){
    pc=get(RegZ);
    if(seg&&segment_end(pc,(agc_word)(get(4)&076000u),(agc_word)machine.InputChannel[7],(agc_word)get(0164),
                        (agc_word)get(0165))){stopped=1;break;}
    if(lmsv_leaves((agc_word)pc,(agc_word)(get(4)&076000u),(agc_word)machine.InputChannel[7],(agc_word)get(2),
                   (agc_word)get(0164),(agc_word)get(0165))){stopped=1;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(!stopped)fprintf(stderr,"lm-servicer case %u timeout %05o\n",n,pc);
  printf("%u %u",n,stopped?0u:1u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));
  printf(" %u %u %u %u %u\n",(unsigned)machine.InputChannel[011]&077777u,(unsigned)machine.InputChannel[012]&077777u,
         (unsigned)machine.InputChannel[032]&077777u,(unsigned)machine.InputChannel[033]&077777u,(unsigned)machine.AllowInterrupt);
 }return 0;
}
