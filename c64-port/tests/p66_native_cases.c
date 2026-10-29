/* SPDX-License-Identifier: MIT */
#include "p66_native_cases.h"
#ifdef P66_NATIVE_C64_INPUT
#include "p66_native_inputs.h"
#endif
static const uint16_t addresses[41]={03746,03756,03644,03645,0,1,037,040,041,024,025,
 03763,03764,03765,070,071,072,03761,03762,01160,01161,01162,01254,01255,01256,
 0154,0155,0156,0157,0160,0161,0162,0163,03766,03767,03770,03771,03772,03773,0135,0121};
uint16_t pn_number(uint16_t n){return n<131u?(uint16_t)(n+16u):(uint16_t)(n-131u+2451u);}
#ifndef P66_NATIVE_REFERENCE_INPUT
void pn_prepare(apollo_overlay_state *m,executive *s,velocity_case *c,uint16_t n){
 uint16_t w[PN_WORDS];unsigned i;agc_word base;
#ifdef P66_NATIVE_C64_INPUT
 (void)c;for(i=0;i<PN_WORDS;++i)w[i]=pn_frozen_inputs[n][i];
#else
 vl_prepare(c,pn_number(n));vl_next(c,pn_number(n),0);vl_words(c,w);
#endif
 base=w[41];
 for(i=0;i<APOLLO_SHARED_WORDS;++i)m->words[i]=(agc_word)((i*73u+123u)&077777u);
 exec_init(s);s->core[0][EXEC_PRIORITY]=(agc_word)(01000u+base);
 s->core[0][EXEC_PUSH]=base;s->fixloc=base;s->newjob=0;
 s->vac[(base-0401u)/054u][0]=0;p66_native_export_executive(m,s);
 for(i=0;i<41;++i)m->words[addresses[i]]=w[i];
 m->words[0120]=base;m->words[0166]=w[42];
 for(i=0;i<43;++i)m->words[base+i]=w[43+i];
 m->words[01234]=w[86];m->words[01235]=w[87];m->words[0130]=w[88];
 m->words[0131]=w[89];m->words[0136]=w[90];m->words[0137]=w[91];m->words[0140]=w[92];
 for(i=0;i<6;++i){m->words[01236+i]=w[93+i];m->words[02630+i]=w[99+i];m->words[03526+i]=w[105+i];m->words[03520+i]=w[111+i];}
 m->words[0116]=0;m->words[0115]=0;m->words[023]=0;m->words[7]=0;
 m->words[3]=03400;m->words[4]=064000;m->words[6]=064007;
 p66_native_import_executive(s,m);
}
#endif
void pn_words(const apollo_overlay_state *m,agc_word base,uint16_t w[PN_WORDS]){
 unsigned i;for(i=0;i<41;++i)w[i]=m->words[addresses[i]];
 w[41]=m->words[0120];w[42]=m->words[0166];for(i=0;i<43;++i)w[43+i]=m->words[base+i];
 w[86]=m->words[01234];w[87]=m->words[01235];w[88]=m->words[0130];
 w[89]=m->words[0131];w[90]=m->words[0136];w[91]=m->words[0137];w[92]=m->words[0140];
 for(i=0;i<6;++i){w[93+i]=m->words[01236+i];w[99+i]=m->words[02630+i];w[105+i]=m->words[03526+i];w[111+i]=m->words[03520+i];}
}
