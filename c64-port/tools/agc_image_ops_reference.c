/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent host-only original engine; never part of native C64 port.
 * Executes one AGC instruction (with its EXTEND/INDEX prefix) placed in
 * unswitched erasable at 0700 from each input image, for the instruction
 * differential of src/agc_image_ops.c. Record: op, K, length, code[3],
 * then 2048 words; output: the 2048-word image after the instruction. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
static agc_t pristine;
int main(int argc,char **argv){
 uint16_t in[6+2048];InhibitAlarms=1;
 if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;
 while(fread(in,sizeof in,1,stdin)==1){
  unsigned i,cycles;uint16_t out[2048];
  machine=pristine;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)in[6+i];
  for(i=0;i<in[2];++i)*memory(0700u+i)=(int16_t)in[3+i];
  *memory(5)=0700;machine.AllowInterrupt=0;
  for(cycles=0;cycles<100u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.ExtraCode&&!machine.IndexValue)break;
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
  }
  if(cycles==100u)return 3;
  for(i=0;i<2048u;++i)out[i]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
  if(fwrite(out,sizeof out,1,stdout)!=1)return 2;
 }
 return 0;
}
