/* SPDX-License-Identifier: MIT */
#include "p66_mode_display_cases.h"
#include "p66_bias_cases.h"
void p66_mode_display_prepare(apollo_overlay_state *s,uint16_t n){
 static const uint16_t modes[]={077777,077776,040000,040001};
 static const uint16_t codes[]={025,003,031,033,017,036,034,023,035,037};
 uint16_t mode=(uint16_t)(n%104u),same=(uint16_t)(04000u+((uint32_t)codes[(mode%100u)/10u]<<5)+codes[mode%10u]);
 p66_bias_prepare(s,n);s->words[3]=01000;s->words[4]=060000;s->words[5]=03534;s->words[6]=060002;
 s->words[01011]=mode<100 ? mode : modes[mode-100u];
 switch((n/104u)%8u){
 case 0:s->words[01035]=same;break;
 case 1:s->words[01035]=(uint16_t)(same^077777u);break;
 case 2:s->words[01035]=04321;break;
 case 3:s->words[01035]=073456;break;
 case 4:s->words[01035]=077777;break;
 case 5:s->words[01035]=04000;break;
 case 6:s->words[01035]=040000;break;
 default:s->words[01035]=037777;break;
 }
}
