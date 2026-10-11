/* SPDX-License-Identifier: MIT */
#include "cm_formatter_cases.h"
#include "cm_pinball_cases.h"
void cm_formatter_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){uint16_t i,k=(uint16_t)(n%4096u),stage=(uint16_t)(n/4096u);
 static const uint16_t corners[8][2]={{0,0},{077777,077777},{037777,037777},{040000,040000},{0,1},{077777,077776},{016252,016252},{061525,061525}};
 cm_pinball_input(s,io,n);for(i=0;i<11u;++i)s->words[01023u+i]|=04000u;
 if(stage==0u){s->words[0]=(uint16_t)(k%100u);s->words[0777]=(uint16_t)(1u+2u*((k/100u)%11u));s->words[5]=03213;}
 else{s->words[0777]=(uint16_t)(14u-(k%3u)*5u);s->words[5]=(uint16_t)(stage==1u?03133u:03167u);
  if(k<128u){s->words[0154]=corners[(k/3u)%8u][0];s->words[0155]=corners[(k/3u)%8u][1];}
  else s->words[0155]=(uint16_t)((s->words[0155]&037777u)|(s->words[0154]&040000u));
 }
}
enum cm_formatter_status cm_formatter_run(apollo_overlay_state *s,cm_native_io *io,uint16_t n){return n<4096u?cm_two_digit(s,io):cm_decimal_display(s,io,(uint8_t)(n<8192u));}
