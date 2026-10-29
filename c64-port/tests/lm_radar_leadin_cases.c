/* SPDX-License-Identifier: MIT */
#include "lm_radar_leadin_cases.h"
#include "lm_admission_image_cases.h"
void lm_radar_input(apollo_overlay_state*m,p66_bias_io*io,uint16_t n){
 static const uint16_t pc[8]={02003,02011,02015,02023,02024,02026,02027,02047};
 static const uint16_t delays[16]={0,077777,1,077776,2,077775,101,077777-101,16251,077777-16251,16252,077777-16252,037777,040000,151,26};
 static const uint16_t offsets[5]={0,2,4,6,077777};
 static const uint16_t counter[4]={0,077777,037777,040000};
 waitlist wl;unsigned i;(void)lm_admission_image_input(m,io,(uint16_t)(n*4u));
 wl_init(&wl);wl.time3=(n&16u)?0:037777;
 if(n&32u){wl.task[8].location=02023;wl.task[8].bank=052064;}
 for(i=0;i<8u;++i)m->words[01400u+i]=wl.interval[i];
 for(i=0;i<9u;++i){m->words[01410u+2u*i]=wl.task[i].location;m->words[01411u+2u*i]=wl.task[i].bank;}
 m->words[026]=wl.time3;m->words[5]=pc[n%8u];m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[2]=01030;m->words[3]=02000;m->words[4]=052000;m->words[6]=052004;
 m->words[02351]=(n&64u)?AGC_MASK:(uint16_t)((n/8u)%6u);m->words[02352]=offsets[(n/48u)%5u];m->words[02353]=(n&128u)?2:0;m->words[02354]=offsets[(n/240u)%5u];m->words[02355]=delays[(n/8u)%16u];m->words[02356]=counter[(n/128u)%4u];
 io->superbank=(n&256u)?060:0;io->interrupt_enabled=(n&512u)?1:0;
}

uint16_t lm_radar_target_number(uint16_t n){return n<32u?(uint16_t)(n*8u):(uint16_t)((n*127u+3u)%LM_RADAR_CASES);}
