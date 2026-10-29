/* SPDX-License-Identifier: MIT */
#include "p66_complete_live_cases.h"
void pcl_prepare(apollo_overlay_state*m,executive*s,p66_bias_io*io,agc_word*channel14,agc_word*channel11,uint16_t n){unsigned i;pdl_prepare(m,s,io,channel14,n);m->words[0104]&=(agc_word)~01000u;m->words[0100]=0;m->words[01012]=0;m->words[01042]=0;m->words[01043]=0;m->words[01021]&=057777u;m->words[03534]=(agc_word)(01000u+n);m->words[03535]=0;for(i=01023;i<=01035;++i)m->words[i]=073777u;m->words[01016]=013;m->words[0777]=077754u;*channel11=0;p66_native_import_executive(s,m);}
void pcl_words(const apollo_overlay_state*m,p66_bias_io io,agc_word channel14,agc_word channel11,uint16_t w[PCL_WORDS],unsigned stage){unsigned i;for(i=0;i<2048u;++i)w[i]=m->words[i];
#ifndef PCL_RAW_CONTROL
 if(stage<5u){w[5]=0;w[2]=0;}if(stage<23u)w[020]=0;
#else
 (void)stage;
#endif
 w[2048]=io.superbank;w[2049]=io.interrupt_enabled;w[2050]=channel14;w[2051]=channel11;}
