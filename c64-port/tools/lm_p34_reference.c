/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent host-only original engine; never part of native C64 port.
 * Runs the unchanged Luminary099 from each lm-p34-transfer case's image
 * (tests/lm_p34_cases.c) to the case's first stop: an external entry the
 * file transfers to, an alarm frontier, a NEWOPS outside the file's
 * interpretive code, or the case's own basic return. Emits n lo, n hi, IRQ,
 * SUPER, tag (1 stopped, 0 a case the native routine must refuse, which is
 * not run) and the 2048 erasable words. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_p34_cases.h"
static agc_t pristine;
static int emit(uint32_t n,unsigned tag){uint16_t row[2053];unsigned i;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=(uint16_t)tag;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));return fwrite(row,sizeof row,1,stdout)==1;}
static int stopped(const lm_p34_case *c,unsigned pc,unsigned fb,unsigned q,unsigned loc){const lm_p34_stop *t;
 for(t=c->stop;t->z;++t){
  if(pc!=t->z||(t->fb!=LM_P34_ANY_FB&&fb!=t->fb)||(t->q!=LM_P34_ANY_Q&&q!=t->q))continue;
  if(t->z==06070u&&lm_p34_owned_newops((agc_word)fb,(agc_word)loc))continue;
  return 1;}
 return 0;
}
int main(int argc,char **argv){uint32_t n,first,limit;if(argc<3||agc_load_binfile(&pristine,argv[1]))return 2;first=argc>3?(uint32_t)strtoul(argv[3],0,0):0u;limit=(uint32_t)strtoul(argv[2],0,0);InhibitAlarms=1;
 for(n=first;n<limit;++n){apollo_overlay_state m;p66_bias_io io;lm_p34_case c;unsigned i,cycles;
  lm_p34_input(&m,&io,&c,n);machine=pristine;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];
  if(c.refused){if(!emit(n,0))return 2;continue;}
  for(cycles=0;cycles<2000000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&stopped(&c,(unsigned)get(5),(unsigned)get(4)&076000u,(unsigned)get(2),(unsigned)get(0164)))break;
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000000u){fprintf(stderr,"lm-p34 case %u timeout %o\n",n,get(5));return 2;}
  if(!emit(n,1))return 2;}
 return 0;}
