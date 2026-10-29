/* SPDX-License-Identifier: MIT */
#include "cm_relay_cases.h"
#include "cm_pinball_cases.h"
void cm_relay_input(apollo_overlay_state *s,cm_native_io *io,cm_relay_output *out,uint16_t n){uint16_t i,cnt=(uint16_t)(n%12u),pattern=(uint16_t)((n/12u)%14u);
 cm_pinball_input(s,io,n);s->words[0]=(uint16_t)(n%16u);s->words[2]=(uint16_t)(n<2048u?02070u:04000u+n%03777u);s->words[3]=01000;s->words[4]=014000;s->words[5]=02024;s->words[6]=014002;s->words[0776]=(uint16_t)(cnt<11u?cnt:077777u);
 if(n<128u){static const uint16_t a[4]={0,1,037776,037777};s->words[0]=a[(n/12u)%4u];}
 io->superbank=(uint16_t)(((n/5u)%8u)*020u);out->wrote_channel10=0;out->channel10=012345;
 for(i=0;i<11u;++i){uint16_t mag=(uint16_t)((s->words[01023u+i]&01777u)|04000u);uint8_t dirty=(uint8_t)(pattern<11u?i==pattern:pattern==11u?1u:pattern==12u?(i&1u):0u);s->words[01023u+i]=(uint16_t)(dirty!=0u?agc_negate(mag):mag);}
 if(n==0u){for(i=0;i<11u;++i)s->words[01023u+i]=04000;s->words[01035]=agc_negate((uint16_t)(04000u|(035u<<5)|037u));s->words[0776]=10;s->words[0]=2;}
}
