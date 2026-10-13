/* SPDX-License-Identifier: MIT */
#include "cm_alarm_cases.h"
#include "cm_interbank_routines.h"
#define CM_ALARM_SITE_LIST
#include "../src/cm_alarm_operands.inc"
#define OTHERS 17u
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static const agc_word callee[4]={CM_AL_ALARM,CM_AL_ALARM2,CM_AL_BAILOUT,CM_AL_POODOO};
static const agc_word other[OTHERS]={CM_AL_CCSHOLE,CM_AL_CURTAINS,CM_AL_VARALARM,CM_AL_PRIOLARM,
 CM_AL_POODOO_RET,CM_AL_POODOO_ABORT,CM_AL_DOWNFLAG1_RET,CM_AL_DOWNFLAG2_RET,CM_AL_DOWNFLAG3_RET,
 CM_AL_MRKLEAN_RET,CM_AL_CCSHOLE_RET,CM_AL_CURTAINS_RET,CM_AL_VARALARM_RET,CM_AL_BAILOUT_RET,
 CM_AL_WHIMPER_M1,CM_AL_PRIOLARM_RET,CM_AL_ALMCADR};
uint16_t cm_al_kinds(void){return (uint16_t)(CM_ALARM_SITE_COUNT+OTHERS);}
/* A switched bank other than 2 and 3 (fixed-fixed) and 10 (which holds PRIOLARM). */
static unsigned free_bank(uint32_t *seed){unsigned b;do b=4u+(rnd(seed)>>20)%034u;while(b==010u);return b;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
/* A return address in 02000-03777 (for ALMCADR and the callers' Q). */
static agc_word ret(uint32_t *seed){return (agc_word)(02000u+(rnd(seed)>>16)%02000u);}
/* Q (and bank) a TC at physical site `at` leaves. */
static void site_q(unsigned at,unsigned *bank,agc_word *q){
 if(at<2048u||at>=4096u){*bank=at/1024u;*q=(agc_word)(02000u+at%1024u);}else *q=(agc_word)(04000u+(at-2048u));
}
void cm_al_input(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw,uint16_t n){
 unsigned i,k=n%cm_al_kinds(),bank,eb;uint32_t seed=UINT32_C(0x5650)+(uint32_t)n*UINT32_C(7919);agc_word fb,q,z;
 for(i=0;i<2048u;++i)s->words[i]=word(&seed);
 s->words[7]=0;for(i=010;i<=060;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {cm_native_io zero={0};*io=zero;}
 {agc_hardware zero={0};*hw=zero;}
 io->interrupt_enabled=(agc_word)((rnd(&seed)>>20)&1u);hw->in_interrupt=(uint16_t)((rnd(&seed)>>20)&1u);
 for(i=0;i<3u;++i){unsigned f=(rnd(&seed)>>20)&3u;agc_word v=(agc_word)(1u+(rnd(&seed)>>17)%037776u);
  s->words[0375u+i]=f==0u?0u:f==1u?077777u:f==2u?v:(agc_word)(v|040000u);}
 bank=free_bank(&seed);q=ret(&seed);
 if(k<CM_ALARM_SITE_COUNT){
  z=callee[cm_alarm_site_list[k].callee];site_q(cm_alarm_site_list[k].physical,&bank,&q);
  /* ALARM2 callers hand their return address in A. */
  if(z==CM_AL_ALARM2)s->words[0]=ret(&seed);}
 else{z=other[k-CM_ALARM_SITE_COUNT];
  if(z==CM_AL_PRIOLARM||z==CM_AL_PRIOLARM_RET)bank=010u;
  /* PRIOLARM and VARALARM callers hand an alarm code (positive) in A. */
  if(z==CM_AL_PRIOLARM||z==CM_AL_VARALARM)s->words[0]=(agc_word)((rnd(&seed)>>17)%040000u);
  /* After VAC5STOR, BAILOUT and POODOO read the code at their caller's Q, saved in ALMCADR. */
  if(z==CM_AL_BAILOUT_RET||z==CM_AL_POODOO_RET){unsigned j;agc_word at;
   do j=(rnd(&seed)>>16)%CM_ALARM_SITE_COUNT;while(cm_alarm_site_list[j].callee<CM_AS_BAILOUT);
   site_q(cm_alarm_site_list[j].physical,&bank,&at);s->words[CM_AL_ALMCADR]=at;}
  if(z==CM_AL_ALMCADR||z==CM_AL_CURTAINS_RET||z==CM_AL_VARALARM_RET)s->words[CM_AL_ALMCADR]=ret(&seed);}
 if(bank>=040u){fb=(agc_word)((bank-010u)<<10);io->superbank=0100;}
 else{fb=(agc_word)(bank<<10);if(bank>=030u)io->superbank=(agc_word)(((rnd(&seed)>>20)&1u)?0060u:0u);}
 eb=(rnd(&seed)>>20)&7u;
 s->words[4]=fb;s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(fb|eb);s->words[2]=q;s->words[5]=z;
}
enum exec_status cm_al_run(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,cm_alarm_operands,CM_ALARM_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=cm_alarm_routines;tables[1]=cm_interbank_routines;
 program.basic=tables;program.basic_count=2;program.hardware=hw;
 cm_al_input(s,io,hw,n);
 /* The run ends at the first point no table owns. */
 st=agc_transfer_run(&program,s,io,&io->superbank,AGC_TRANSFER_NONE,24,steps);
 return st==EXEC_UNMAPPED?EXEC_OK:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_al_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){cm_native_io io;agc_hardware hw;uint16_t steps=0;unsigned i;
 *status=cm_al_run(s,&io,&hw,n,&steps);sum_a=sum_b=0;
 mix(io.interrupt_enabled);mix(io.superbank);mix(hw.in_interrupt);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
