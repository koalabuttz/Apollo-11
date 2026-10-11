/* SPDX-License-Identifier: MIT */
#include "p66_direct_launch_cases.h"
#include "p66_launch_cases.h"
#include "p66_vertical_start_cases.h"
void p66_direct_launch_prepare(apollo_overlay_state *s,uint16_t n){
 uint8_t pending=(uint8_t)(1u+n%7u),i;
 if(n<1024u){p66_launch_prepare(s,n);return;}
 p66_vertical_start_prepare(s,n);
 s->words[0167]=(uint16_t)(035000+s->words[0120]);
 for(i=1;i<8;++i)s->words[0154+i*12u+11u]=(uint16_t)(010110+(n%8u)*01000u);
 s->words[067]=n%8u==0 ? 0 : (uint16_t)(pending*12u);
 s->words[0154+pending*12u+11u]=(uint16_t)(010110+((n/8u)%24u)*01000u);
}
