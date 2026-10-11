/* SPDX-License-Identifier: MIT */
#include "cm_alarm_sites_cases.h"
#include "cm_alarm_complete_cases.h"
#include "cm_alarm2_image_cases.h"
#include "cm_abort_word_cases.h"
static uint16_t sites(void){uint16_t n;(void)cm_alarm_site_table(&n);return n;}
uint32_t cm_alarm_sites_cases(void){return 3ul*sites()*CM_ALARM_SITES_CONTEXTS;}
/* Logical address of physical word p, setting FB/SUPERBNK when it is banked. */
static agc_word place(apollo_overlay_state *s,cm_native_io *io,uint16_t p){uint16_t bank=(uint16_t)(p/1024u),offset=(uint16_t)(p%1024u);
 if(bank==2u||bank==3u)return (agc_word)(bank*02000u+offset);
 if(bank>=040u){s->words[4]=(agc_word)((bank-8u)<<10);io->superbank=(agc_word)(0100u|(io->superbank&060u));}
 else{s->words[4]=(agc_word)(bank<<10);if(bank>=030u)io->superbank=(agc_word)(io->superbank&060u);}
 s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));return (agc_word)(02000u+offset);
}
enum cm_alarm_sites_kind cm_alarm_sites_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){uint16_t count;const agc_site_operand *t=cm_alarm_site_table(&count);
 enum cm_alarm_sites_kind kind=(enum cm_alarm_sites_kind)(n%3u);uint32_t site=(n/3u)%count,ctx=n/(3u*count);uint16_t p=t[site].physical;
 if(kind==CM_SITES_ALARM){cm_alarm_complete_input(s,io,((ctx*61u)%4030u)*8u+(ctx&1u)*4u+CM_RECORD_ALARM);s->words[2]=place(s,io,p);}
 else if(kind==CM_SITES_ALARM2){cm_alarm2_image_input(s,io,(ctx*509u)%32768u);s->words[2]=place(s,io,p);}
 else{cm_abort_word_input(s,io,(ctx*7u)*3u+(ctx&1u));s->words[01363]=place(s,io,p);}
 return kind;
}
enum exec_status cm_alarm_sites_run(apollo_overlay_state *s,cm_native_io *io,enum cm_alarm_sites_kind kind){
 if(kind==CM_SITES_ALARM)return cm_alarm_site_call(s,io,CM_RECORD_ALARM);
 if(kind==CM_SITES_ALARM2)return cm_alarm2_site_call(s,io);
 return cm_abort_site_call(s,io);
}
