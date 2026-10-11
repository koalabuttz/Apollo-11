/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_ephemeris_time_cases.h"
int main(int argc,char **argv){apollo_overlay_state m;p66_bias_io io;unsigned i,c,n=argc>2?(unsigned)strtoul(argv[2],0,0):1u;agc_word previous[2048];unsigned count=0;
if(argc<2||agc_load_binfile(&machine,argv[1]))return 2;cm_ephemeris_time_input(&m,&io,n);m.words[0164]=02110;m.words[m.words[0120]+052]=023377;
for(i=0;i<2048;++i){*memory(i)=(int16_t)m.words[i];previous[i]=m.words[i];}machine.AllowInterrupt=io.interrupt_enabled;machine.OutputChannel7=machine.InputChannel[7]=io.superbank;InhibitAlarms=1;
for(c=0;c<100000;++c){if(!machine.PendFlag&&!machine.ExtraDelay){unsigned pc=get(5);if(c==0||pc==07350||pc==06720||pc==07541||pc==03516||pc==03527||pc==03023||pc==03024||pc==03031||pc==03043||pc==07427||pc==02121||(pc==06037&&get(0164)==02161)){fprintf(stderr,"FRONTIER%u c%u PC%o LOC%o FB%o\n",count++,c,pc,get(0164),get(4));for(i=0;i<2048;++i){agc_word v=(agc_word)((unsigned)*memory(i)&(i<2?0177777u:077777u));if(v!=previous[i])fprintf(stderr,"%04o:%06o->%06o ",i,previous[i],v);previous[i]=v;}fputc('\n',stderr);if(c&&pc==06037)return 0;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}return 2;}
