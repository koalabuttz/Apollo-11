/* SPDX-License-Identifier: MIT */
#include "dsky_pinbranch_cases.h"
#include "p66_bias_cases.h"
void dsky_pinbranch_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1};uint16_t cm=n>=8192u;
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/8u)%8u*0400u);s->words[4]=020000;s->words[6]=(uint16_t)(020000u|((n/8u)%8u));s->words[5]=cm ? 03272u : 03050u;if((n/4096u)%2u){static const uint16_t pc[2][4]={{03471,02377,02544,02615},{03713,02624,02766,03037}};s->words[5]=pc[cm][n%4u];}s->words[1]=raw[(n/64u)%8u];s->words[0100]=(uint16_t)((s->words[0100]&07777u)|((n%8u)*010000u));s->words[cm ? 01072u : 01073u]=raw[(n/128u)%4u];s->words[cm ? 01070u : 01071u]=(uint16_t)((s->words[cm ? 01070u : 01071u]&077757u)|((n/4u)%2u)*020u);s->words[0370]=raw[(n/256u)%4u];io->superbank=(uint16_t)(((n/16u)%8u)*020u);io->interrupt_enabled=(uint16_t)((n/2u)%2u);
}
