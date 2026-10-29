/* SPDX-License-Identifier: MIT */
#include "p66_descent_live_cases.h"
const uint16_t pdl_extra_addresses[PDL_EXTRA_WORDS]={01244,01245,02542,02543,02546,02547,02544,02545,03610,03611,03757,03760,03465,03466,03467,03470,03612,03613,03615,03620,03606,03614,03617,03607,055,02506,02507,01011,01442,01443,03733,03734,03735,03736,03737,03740,02262,02263,020};
void pdl_prepare(apollo_overlay_state *m,executive *s,p66_bias_io *io,agc_word *channel,uint16_t n){
 unsigned i;pgl_prepare(m,s,n);
 m->words[01244]=(agc_word)(01000u+(n&0377u));m->words[01245]=0;
 m->words[02542]=01000;m->words[02543]=0;m->words[02546]=01000;m->words[02547]=0;m->words[02544]=0100;m->words[02545]=0;
 m->words[03610]=0;m->words[03611]=0;
 m->words[03757]=m->words[024];m->words[03760]=agc_add_word(m->words[025],agc_negate(0100));
 for(i=12;i<30;++i)m->words[pdl_extra_addresses[i]]=0;
 m->words[03620]=02000;m->words[02506]=0700;m->words[02507]=03500;m->words[01011]=0102;
 for(i=30;i<36;++i)m->words[pdl_extra_addresses[i]]=(agc_word)(n*31u+i);
 m->words[0104]|=01000;m->words[03621]=2;
 io->superbank=n&1u?060:0;io->interrupt_enabled=1;*channel=(agc_word)(n*17u);
 p66_native_import_executive(s,m);
}
void pdl_words(const apollo_overlay_state *m,p66_bias_io io,agc_word channel,uint16_t w[PDL_WORDS],unsigned stage){unsigned i;
 pgl_words(m,w,stage);
 for(i=0;i<PDL_EXTRA_WORDS;++i)w[PGL_FULL_WORDS+i]=i==PDL_EXTRA_WORDS-1u&&stage<23u?0:m->words[pdl_extra_addresses[i]];
 w[PGL_FULL_WORDS+PDL_EXTRA_WORDS]=channel;w[PGL_FULL_WORDS+PDL_EXTRA_WORDS+1u]=io.superbank;w[PGL_FULL_WORDS+PDL_EXTRA_WORDS+2u]=io.interrupt_enabled;
}
