/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_waitlist_image_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_waitlist_image_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned i;uint32_t seed=0x5245055u+n;cm_waitlist w;
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[3]=(agc_word)((n&7u)<<8);s->words[4]=(agc_word)(((n>>3)&31u)<<10);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=05245;
 s->words[0]=(agc_word)(1u+(n%400u));s->words[1]=agc_extend(s->words[1]);s->words[2]=0705;
 memset(io,0,sizeof *io);io->superbank=(agc_word)((n&7u)<<4);io->interrupt_enabled=(agc_word)((n>>3)&1u);
 cm_wl_init(&w);w.time3=(agc_word)(037700u+(n%64u));
 for(i=0;i<8u;++i){w.interval[i]=agc_negate((agc_word)(50u));w.task[i].location=(agc_word)(02000u+i*4u);w.task[i].bank=032066;}
 cm_native_commit_waitlist(s,io,&w);s->words[0705]=03207;s->words[0706]=032066;
 if(n%3u!=0){s->words[3]=03400;s->words[4]=050000;s->words[6]=050007;
  s->words[2]=n%3u==1?03277:02576;
 }
}
