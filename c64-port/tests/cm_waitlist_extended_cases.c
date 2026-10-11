/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_extended_cases.h"
#include "cm_waitlist_image_cases.h"
void cm_waitlist_extended_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned i,domain=n%8u;static const agc_register bad[4]={0,0177777,0177776,0140205};cm_waitlist_image_input(s,io,n);s->words[2]=0705;s->words[5]=n<1024u?05245:05235;s->words[0705]=03207;s->words[0706]=032066;
 if(domain<4u)s->words[0]=bad[domain];else if(domain==4u){s->words[01430]=02210;s->words[01431]=040060;s->words[0]=1;}else if(domain==5u){for(i=0;i<8u;++i)s->words[01400u+i]=1;s->words[0]=500;}else if(domain==6u){for(i=0;i<8u;++i)s->words[01400u+i]=077777;s->words[0]=20;}else{s->words[0]=(agc_word)(16250u+(n/8u)%134u);s->words[01431]=(agc_word)((((n>>3)&31u)<<10)|(n&7u)|((n&7u)<<4));}
}
