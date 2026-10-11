/* SPDX-License-Identifier: MIT */
#include "elapsed_return_control_cases.h"
const unsigned eq_addresses[41]={03746,03756,03644,03645,0,1,
 037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
 01160,01161,01162,01254,01255,01256,
 0154,0155,0156,0157,0160,0161,0162,0163,
 03766,03767,03770,03771,03772,03773,0135,0121};
static uint16_t word(uint32_t *seed){*seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);return(uint16_t)((*seed>>16)&AGC_MASK);}
void eq_prepare(apollo_overlay_state *image,uint16_t n){
 static const agc_word q[8]={0,077777,3,01434,02701,02314,07777,01030};
 static const agc_word corner[4]={0,077777,037777,040000};
 uint32_t seed=UINT32_C(0x33e1c031)^((uint32_t)n*UINT32_C(2654435761));
 unsigned i;uint16_t *w=image->words;
 for(i=0;i<2048u;++i)w[i]=word(&seed);
 w[0]=agc_extend(w[0]);w[1]=agc_extend(w[1]);w[2]=q[n&7u];
 w[3]=01000;w[4]=064000;w[5]=07005;w[6]=064002;w[7]=0;
 w[0120]=(uint16_t)(0400u+44u*(n%5u));w[0166]=(uint16_t)(w[0120]+6u);
 w[0163]=0;w[0164]=03360;w[0165]=064007;w[0116]=01634;w[0115]=040000;w[023]=0;
 if(n<2048u){unsigned c=n>>3;w[0154]=corner[c&3u];w[0155]=corner[(c>>2)&3u];w[01234]=corner[(c>>4)&3u];w[01235]=corner[(c>>6)&3u];}
}
void eq_gather(elapsed_case *c,const apollo_overlay_state *image){
 uint16_t v[EL_WORD_COUNT];unsigned i;const uint16_t *w=image->words;
 for(i=0;i<41u;++i)v[i]=w[eq_addresses[i]];
 v[41]=w[0120];v[42]=w[0166];
 for(i=0;i<P66_VAC_WORDS;++i)v[43u+i]=w[w[0120]+i];
 v[86]=w[01234];v[87]=w[01235];v[88]=w[0130];el_from_words(c,v);
}
void eq_commit(apollo_overlay_state *image,const elapsed_case *c){
 uint16_t v[EL_WORD_COUNT];unsigned i;uint16_t *w=image->words;el_words(c,v);
 for(i=0;i<41u;++i)w[eq_addresses[i]]=v[i];
 w[0120]=v[41];w[0166]=v[42];for(i=0;i<P66_VAC_WORDS;++i)w[w[0120]+i]=v[43u+i];
 w[01234]=v[86];w[01235]=v[87];w[0130]=v[88];
}
