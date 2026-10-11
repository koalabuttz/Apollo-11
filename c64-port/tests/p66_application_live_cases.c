/* SPDX-License-Identifier: MIT */
#include "p66_application_live_cases.h"
#include "waitlist.h"
void pal_prepare(apollo_overlay_state *m,executive *s,p66_bias_io *io,agc_word *c14,agc_word *c11,uint16_t n){
 uint8_t i;pcl_prepare(m,s,io,c14,c11,n);
 m->words[5]=02477;m->words[3]=03400;m->words[4]=062000;m->words[6]=062007;
 m->words[0167]=(agc_word)(035000u+m->words[0120]);m->words[0166]=m->words[0120];m->words[m->words[0120]-1u]=0;
 m->words[026]=037777u;m->words[0734]=AGC_NEG_ZERO;
 for(i=0;i<8u;++i)m->words[01400u+i]=WL_DEFAULT_INTERVAL;
 for(i=0;i<9u;++i){m->words[01410u+2u*i]=WL_END_LOCATION;m->words[01411u+2u*i]=WL_END_BANK;}
 p66_native_import_executive(s,m);
}
