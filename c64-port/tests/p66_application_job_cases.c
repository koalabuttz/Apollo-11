/* SPDX-License-Identifier: MIT */
#include "p66_application_job_cases.h"
#include "p66_mode_display_cases.h"
#include "dsky_charin_nv_cases.h"
#include "dsky_charin_number_cases.h"
#include "executive.h"
void p66_application_job_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *c11,agc_word *c14,uint16_t n){
 uint8_t i;uint16_t bank;
 if(n<1024u)p66_mode_display_prepare(s,n);
 else if(n<5120u)dsky_charin_nv_prepare(s,(uint16_t)(n-1024u));
 else dsky_charin_number_prepare(s,(uint16_t)(n-5120u));
 bank=n<1024u?060002u:060001u;
 s->words[3]=(uint16_t)((bank&7u)*0400u);s->words[4]=060000;s->words[6]=bank;
 s->words[7]=0;s->words[067]=0;s->words[0164]=s->words[5];s->words[0165]=bank;
 s->words[0166]=EXEC_FAKEPRET;s->words[0167]=030000u+EXEC_FAKEPRET;
 for(i=1;i<8u;++i)s->words[0167u+12u*i]=AGC_NEG_ZERO;
 io->superbank=0100;io->interrupt_enabled=(uint16_t)(n&1u);
 *c11=(agc_word)(((uint32_t)n*151u+04100u)&077777u);*c14=(agc_word)(n&0777u);
}
