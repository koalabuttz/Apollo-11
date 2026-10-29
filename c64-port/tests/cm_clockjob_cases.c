/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_clockjob_cases.h"
#include "cm_trig_cases.h"
void cm_clockjob_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){agc_word irq;uint16_t k=(uint16_t)(n%8192u);static const agc_word nv[8]={0,077777,1,077776,01474,01550,037777,040000};cm_trig_input(s,&irq,k,2);memset(io,0,sizeof *io);io->superbank=(agc_word)((k%4u)*020u);io->interrupt_enabled=irq;io->channel11=(agc_word)(((uint32_t)k*977u)&AGC_MASK);s->words[3]=03000;s->words[4]=n<8192u||n>=16384u?050000:046000;s->words[6]=(agc_word)(s->words[4]|6u);s->words[5]=(agc_word)(n<8192u?03332u:n<16384u?03605u:03342u);s->words[032]=s->words[0772];s->words[033]=s->words[0766];s->words[034]=s->words[0770];if(n>=8192u&&n<16384u){s->words[0]=04675;s->words[2]=3;s->words[0133]=03342;s->words[0134]=050000;s->words[1]=agc_extend(s->words[0741]);}s->words[01145]=k<128u?nv[k%8u]:s->words[01145];}
enum cm_clockjob_boundary cm_clockjob_apply(apollo_overlay_state *s,cm_native_io *io,uint16_t n){return n<8192u?cm_clockjob_enter(s,io):n<16384u?cm_clockjob_return(s,io):cm_clockjob_select(s,io);}
