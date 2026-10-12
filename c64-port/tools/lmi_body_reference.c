/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent host-only original engine; never part of native C64 port.
 * Runs unchanged Luminary099 from each input image (its Z, banks and
 * erasable as given) until Z first reaches one of up to four stop
 * addresses at a whole-instruction boundary, for the interpreter body
 * differential (tools/lmi_body_differential.py). Record: 16 header words
 * (word1-4 stops, word5 IRQ, word6 SUPER) then 2048 words; output: stop
 * index then the 2048-word image. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
static agc_t pristine;
int main(int argc,char **argv){
 uint16_t in[16+2048];unsigned record=0;InhibitAlarms=1;
 if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;
 while(fread(in,sizeof in,1,stdin)==1){
  unsigned i,cycles,tag=4;uint16_t out[1+2048];
  machine=pristine;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)in[16+i];
  machine.AllowInterrupt=in[5];machine.InputChannel[7]=machine.OutputChannel7=in[6];
  for(cycles=0;cycles<200000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.ExtraCode&&!machine.IndexValue){
    unsigned z=get(5);for(i=0;i<4u;++i)if(in[1+i]&&z==in[1+i])tag=i;
    if(tag<4u)break;
   }
   if(getenv("LMI_TRACE")&&!machine.PendFlag)fprintf(stderr,"%05o A=%06o L=%06o Q=%06o EB=%05o\n",(unsigned)*memory(5)&07777u,(unsigned)*memory(0)&0177777u,(unsigned)*memory(1)&0177777u,(unsigned)*memory(2)&0177777u,get(3));
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
  }
  if(tag==4u){fprintf(stderr,"record %u timeout at %05o FB %05o\n",record,get(5),get(4));return 3;}
  ++record;
  out[0]=(uint16_t)tag;
  for(i=0;i<2048u;++i)out[1+i]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
  if(fwrite(out,sizeof out,1,stdout)!=1)return 2;
 }
 return 0;
}
