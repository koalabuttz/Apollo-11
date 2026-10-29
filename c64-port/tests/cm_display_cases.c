/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_display_cases.h"
void cm_display_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){
 uint16_t i,k=(uint16_t)(n%4096u),stage=(uint16_t)(n/4096u);uint32_t seed=(uint32_t)n+8731u;
 static const uint16_t raw[8]={0,0177777,037777,040000,0100000,0140000,077777,0123456};
 for(i=0;i<2048u;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);s->words[i]=(uint16_t)((seed>>9)&077777u);}
 memset(io,0,sizeof *io);io->superbank=(uint16_t)(((k/512u)%8u)*020u);io->interrupt_enabled=(uint16_t)(k%2u);
 s->words[0]=(uint16_t)(((k%31u)+1u)*01000u);s->words[1]=raw[k%8u];s->words[2]=03300;
 s->words[3]=(uint16_t)(((k/288u)%8u)*0400u);s->words[4]=(uint16_t)(((k/128u)%32u)*02000u);
 if(stage!=0u)s->words[4]=020000;
 s->words[6]=(uint16_t)(s->words[4]|(s->words[3]>>8));s->words[5]=(uint16_t)(stage==0u?05210u:stage==1u?03261u:03267u);s->words[7]=0;
 s->words[067]=0;s->words[0167]=(uint16_t)(020000u+(k&0777u));s->words[0166]=0110;
 s->words[0121]=(uint16_t)(k%4u==3u?AGC_NEG_ZERO:k%3u);
 s->words[01071]=(uint16_t)((s->words[01071]&074377u)|((k%8u)*0400u));
 s->words[0366]=(uint16_t)((s->words[0]&037000u)|((k%8u)*020u)|(k&017u));
 s->words[0374]=(uint16_t)((((k/128u)%32u)*02000u)+01403u+(k%0370u));
 for(i=1;i<7u;++i){uint16_t a=(uint16_t)(0154u+i*12u);
  s->words[a+8u]=(uint16_t)(02000u+i);s->words[a+9u]=(uint16_t)(02000u*((i+k)%32u)|((i+k)%8u)|(((i+k)%8u)*020u));
  s->words[a+10u]=(uint16_t)(k%3u?0110u:077667u);
  s->words[a+11u]=(uint16_t)(01000u*(((uint32_t)k*13u+(uint32_t)i*7u)%31u+1u)+(i*17u));
  if((k+i)%5u==0u)s->words[a+11u]=AGC_NEG_ZERO;else if((k+i)%5u==1u)s->words[a+11u]=agc_negate(s->words[a+11u]);
 }
 if(k<128u){uint16_t packed=(uint16_t)(033000u+(k&0777u));s->words[0]=033000;s->words[0366]=(uint16_t)(033000u|((k%8u)*020u));
  for(i=1;i<7u;++i)s->words[0167u+i*12u]=AGC_NEG_ZERO;
  if(k>=16u&&k<64u){i=(uint16_t)(k%6u+1u);s->words[0167u+i*12u]=(uint16_t)(packed+(k<32u?1u:2u));}
  else if(k>=64u){s->words[0167u+12u]=037777;s->words[0167u+72u]=k<96u?037777:037776;}
 }
 if(stage==2u)s->words[2]=raw[(k/8u)%8u];
}
enum exec_status cm_display_run(apollo_overlay_state *s,cm_native_io *io,uint16_t n,exec_address *next){return n<4096u?cm_basic_priority_change(s,io,next):n<8192u?cm_init_display(s,io,next):cm_init_display_return(s,io,next);}
