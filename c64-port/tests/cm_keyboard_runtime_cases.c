/* SPDX-License-Identifier: MIT */
#include "cm_keyboard_runtime_cases.h"
#include "cm_charin_number_cases.h"
#include "cm_charin_nv_cases.h"
void cm_keyboard_runtime_input(apollo_overlay_state *s,cm_native_io *io,uint16_t *isr,uint16_t *key,uint16_t n){uint16_t i,wrote;
 if(n%4u<2u)cm_charin_nv_input(s,io,&wrote,n);else cm_charin_number_input(s,io,&wrote,n);
 *key=s->words[0154];s->words[3]=(uint16_t)((n%8u)<<8);s->words[4]=02000;s->words[6]=(uint16_t)(02000u|n%8u);s->words[5]=04024;s->words[015]=03232;s->words[017]=010067;s->words[7]=0;
 for(i=0;i<7;++i)s->words[0154+i*12u+11u]=077777;
 for(i=0;i<5;++i)s->words[0400+i*44u]=(uint16_t)(0400u+i*44u);
 s->words[067]=077777;io->executive_alarm=0;io->superbank=(uint16_t)((n/8u)%8u*020u);io->interrupt_enabled=0;*isr=1;
}
