/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "p66_guidance_live_cases.h"
#ifndef PGL_FROZEN_INPUT
#include "radial_cases.h"
#include "projection_cases.h"
#include "acceleration_prefix.h"
#else
#include "p66_guidance_live_inputs.h"
#endif
const agc_word pgl_boundaries[PGL_STAGES]={03353,03356,03360,03361,03364,03366,03370,03373,03374,03375,03377,03401,03404,03407,03411,03414,03416,03420,03421,03422,03427,03430,03433};
const agc_word pgl_mixed_boundaries[PGL_MIXED_STAGES]={03353,03356,03353,03356,03360,03361,03364,03366,03370,03373,03374,03375,03377,03401,03404,03407,03411,03414,03416,03420,03421,03422,03427,03430,03433};
void pgl_addresses(uint16_t map[PGL_WORDS],uint16_t base){
 static const uint16_t first[41]={03746,03756,03644,03645,0,1,037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,01160,01161,01162,01254,01255,01256,0154,0155,0156,0157,0160,0161,0162,0163,03766,03767,03770,03771,03772,03773,0135,0121};
 static const uint16_t tail[49]={021,022,0116,0132,0141,0122,0123,0124,0125,0126,0127,03473,03474,03774,03775,02332,02333,02540,02541,0133,0134,0117,0142,0143,0766,0767,0770,0771,0772,0773,0736,0737,0740,0741,0742,0743,0744,0745,0746,0747,0750,0751,032,033,034,0144,0145,03463,03464};
 unsigned i;memcpy(map,first,sizeof first);map[41]=0120;map[42]=0166;
 for(i=0;i<43;++i)map[43+i]=(uint16_t)(base+i);
 map[86]=01234;map[87]=01235;map[88]=0130;map[89]=0131;map[90]=0136;map[91]=0137;map[92]=0140;
 for(i=0;i<6;++i){map[93+i]=(uint16_t)(01236+i);map[99+i]=(uint16_t)(02630+i);map[105+i]=(uint16_t)(03526+i);map[111+i]=(uint16_t)(03520+i);}
 memcpy(map+117,tail,sizeof tail);
}
void pgl_input(uint16_t words[PGL_WORDS],uint16_t number){
#ifndef PGL_FROZEN_INPUT
 radial_case c;projection_case seed;uint16_t projection[PJ_WORDS],n=(uint16_t)(number+16u);agc_dp tau=acc_prefix_time(n);
 ra_prepare(&c,n);ra_next(&c,n,0);ra_words(&c,words);pj_input(&seed,n);pj_words(&seed,projection);
 words[134]=tau.high;words[135]=tau.low;memcpy(words+136,projection+104,30u*sizeof(uint16_t));
#else
 memcpy(words,pgl_inputs[number],sizeof pgl_inputs[0]);
#endif
}
void pgl_prepare(apollo_overlay_state *m,executive *s,uint16_t number){
 uint16_t words[PGL_WORDS],map[PGL_WORDS],base;unsigned i;
 pgl_input(words,number);base=words[41];pgl_addresses(map,base);
 for(i=0;i<APOLLO_SHARED_WORDS;++i)m->words[i]=(uint16_t)((i*73u+123u)&077777u);
 exec_init(s);s->core[0][EXEC_PRIORITY]=(agc_word)(01000u+base);s->core[0][EXEC_PUSH]=base;s->fixloc=base;s->newjob=0;
 s->vac[(base-0401u)/054u][0]=0;p66_native_export_executive(m,s);
 for(i=0;i<PGL_WORDS;++i)m->words[map[i]]=words[i];
 m->words[2]=01030;m->words[5]=03275;m->words[0115]=0;m->words[023]=0;m->words[7]=0;
 m->words[3]=03400;m->words[4]=064000;m->words[6]=064007;
 p66_native_import_executive(s,m);
}
void pgl_words(const apollo_overlay_state *m,uint16_t words[PGL_FULL_WORDS],unsigned stage){
 uint16_t map[PGL_WORDS];unsigned i,k=PGL_WORDS;static const uint16_t control[7]={0116,023,0115,7,3,4,6};
 pgl_addresses(map,m->words[0120]);for(i=0;i<PGL_WORDS;++i)words[i]=m->words[map[i]];
 for(i=0;i<96;++i)words[k++]=m->words[0154+i];
 for(i=0;i<220;++i)words[k++]=m->words[0400+i];
 words[k++]=m->words[0120];words[k++]=m->words[0121];for(i=0;i<7;++i)words[k++]=m->words[control[i]];
 words[k++]=stage<5?0:m->words[5];words[k]=stage<5?0:m->words[2];
}
void pgl_prepare_high(apollo_overlay_state *m,uint16_t number){
 unsigned i,base=m->words[0120]==0401u?0455u:0401u;
 m->words[base-1u]=0;for(i=0;i<43;++i)m->words[base+i]=(agc_word)((i*17u+number*31u)&077777u);
 m->words[0154+12+EXEC_LOC]=03275;m->words[0154+12+EXEC_BANK]=064007;
 m->words[0154+12+EXEC_PUSH]=(agc_word)base;m->words[0154+12+EXEC_PRIORITY]=(agc_word)(04000u+base);
}
