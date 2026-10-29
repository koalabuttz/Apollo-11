/* SPDX-License-Identifier: MIT */
#include "dsky_charin_alarm_cases.h"
#include "p66_bias_cases.h"
void dsky_charin_alarm_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n){
 static const uint16_t keys[]={0,012,013,014,015,016,017,023,024,025,026,027,030,035,010,011},raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},cadrs[]={0,077777,1,077776},count[]={0,077777,1,077776,037777,040000,19,077754};
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/2u)%8u*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/2u)%8u));s->words[5]=n<4096u ? 02077 : 02000;s->words[0154]=keys[n%16u];s->words[1]=raw[(n/64u)%8u];s->words[01042]=cadrs[(n/96u)%4u];s->words[0777]=count[(n/16u)%8u];s->words[01000]=(uint16_t)(s->words[01000]&077774u);
 io->control.superbank=0100;io->control.interrupt_enabled=(uint16_t)((n/16u)%2u);io->channel11=(uint16_t)((n*151u+04100u)&077777u);io->wrote_channel11=0;
}
