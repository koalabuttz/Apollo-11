/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "nv_render_cases.h"
#include "p66_nv_rows.h"
static apollo_overlay_state s;
static agc_t pristine;
int main(int argc,char **argv){unsigned n,i,cycles,pc,phase;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<1024u;++n){p66_bias_io io;nv_rows_input(&s,&io,(uint16_t)(n*4u));(void)p66_nv_data_prepare(&s,&io);machine=pristine;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;phase=0;
 for(cycles=0;cycles<20000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((phase%4u==0u&&get(RegFB)==060000u&&(pc==02710u||pc==02717u))||(phase%4u==1u&&get(RegFB)==062000u&&pc==02566u)||(phase%4u==2u&&get(RegFB)==062000u&&pc==02570u)||(phase%4u==3u&&((get(RegFB)==062000u&&pc==02537u)||(get(RegFB)==020000u&&pc==03122u))))){printf("%u %u 0",n,phase);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);if(++phase==12u)break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==20000){fprintf(stderr,"case%u timeout PC%05o\n",n,pc);return 2;}
}return 0;}
