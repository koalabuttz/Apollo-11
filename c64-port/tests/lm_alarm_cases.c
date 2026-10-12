/* SPDX-License-Identifier: MIT */
#include "lm_alarm_cases.h"
#include "lm_interbank_routines.h"
#define LM_ALARM_SITE_LIST
#include "../src/lm_alarm_sites.inc"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static const agc_word callee[9]={LM_AL_ALARM,LM_AL_ALARM2,LM_AL_BAILOUT,LM_AL_POODOO,LM_AL_ABORT2,
 LM_AL_BAILOUT1,LM_AL_POODOO1,LM_AL_ALARM1,LM_AL_ALMNCADR};
static const agc_word other[18]={LM_AL_CCSHOLE,LM_AL_CURTAINS,LM_AL_VARALARM,LM_AL_WHIMPER,LM_AL_GOPOODOO,
 LM_AL_PRIOLARM,LM_AL_FLAGS,LM_AL_BAILOUT_RET,LM_AL_POODOO_RET,LM_AL_FLAGS_RET,LM_AL_V37KLEAN_RET,
 LM_AL_MRKLEAN_RET,LM_AL_SUPERSW_RET,LM_AL_CURTAINS_RET,LM_AL_VARALARM_RET,LM_AL_PRIOLARM_RET,LM_AL_ALMCADR,
 LM_AL_ALMNCADR};
uint16_t lm_al_kinds(void){return (uint16_t)(LM_ALARM_SITE_COUNT+18u);}
/* A switched bank other than 2 and 3 (fixed-fixed) and 10 and 13 (which hold this file's own code). */
static unsigned free_bank(uint32_t *seed){unsigned b;do b=4u+(rnd(seed)>>20)%034u;while(b==010u||b==013u);return b;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
/* A return address in 02000-03777 (for ALMCADR and the callers' Q). */
static agc_word ret(uint32_t *seed){return (agc_word)(02000u+(rnd(seed)>>16)%02000u);}
void lm_al_input(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw,uint16_t n){
 unsigned i,k=n%lm_al_kinds(),bank,eb;uint32_t seed=UINT32_C(0x5567)+(uint32_t)n*UINT32_C(7919);agc_word fb,q,z;
 for(i=0;i<2048u;++i)s->words[i]=word(&seed);
 s->words[7]=0;for(i=010;i<=060;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {p66_bias_io zero={0};*io=zero;}
 {agc_hardware zero={0};*hw=zero;}
 io->interrupt_enabled=(agc_word)((rnd(&seed)>>20)&1u);hw->in_interrupt=(uint16_t)((rnd(&seed)>>20)&1u);
 for(i=0;i<3u;++i){unsigned f=(rnd(&seed)>>20)&3u;agc_word v=(agc_word)(1u+(rnd(&seed)>>17)%037776u);
  s->words[0375u+i]=f==0u?0u:f==1u?077777u:f==2u?v:(agc_word)(v|040000u);}
 bank=free_bank(&seed);q=ret(&seed);
 if(k<LM_ALARM_SITE_COUNT||k==lm_al_kinds()-1u){
  /* ALMNCADR has no TC site; it reads the code at a random site's operand. */
  unsigned j=k<LM_ALARM_SITE_COUNT?k:(rnd(&seed)>>16)%LM_ALARM_SITE_COUNT,at=lm_alarm_site_list[j].physical;
  z=k<LM_ALARM_SITE_COUNT?callee[lm_alarm_site_list[j].callee]:LM_AL_ALMNCADR;
  if(at<2048u||at>=4096u){bank=at/1024u;q=(agc_word)(02000u+at%1024u);}else q=(agc_word)(04000u+(at-2048u));
  /* ALARM2 and ABORT2 callers hand their return address in A. */
  if(z==LM_AL_ALARM2||z==LM_AL_ABORT2)s->words[0]=ret(&seed);}
 else{z=other[k-LM_ALARM_SITE_COUNT];
  if(z==LM_AL_PRIOLARM||z==LM_AL_PRIOLARM_RET)bank=010u;
  /* PRIOLARM and VARALARM callers hand an alarm code (positive) in A. */
  if(z==LM_AL_PRIOLARM||z==LM_AL_VARALARM)s->words[0]=(agc_word)((rnd(&seed)>>17)%040000u);
  if(z==LM_AL_FLAGS){bank=013u;q=(agc_word)(02000u+(rnd(&seed)>>16)%01700u);}
  if(z==LM_AL_ALMCADR||z==LM_AL_CURTAINS_RET||z==LM_AL_VARALARM_RET)s->words[LM_AL_ALMCADR]=ret(&seed);}
 if(bank>=040u){fb=(agc_word)((bank-010u)<<10);io->superbank=0100;}
 else{fb=(agc_word)(bank<<10);if(bank>=030u)io->superbank=(agc_word)(((rnd(&seed)>>20)&1u)?0060u:0u);}
 eb=(rnd(&seed)>>20)&7u;
 s->words[4]=fb;s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(fb|eb);s->words[2]=q;s->words[5]=z;
}
enum exec_status lm_al_run(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,lm_alarm_operands,LM_ALARM_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=lm_alarm_routines;tables[1]=lm_interbank_routines;
 program.basic=tables;program.basic_count=2;program.hardware=hw;
 lm_al_input(s,io,hw,n);
 /* The run ends at the first point no table owns, or at SWRETURN after a
  * BANKCALL'd FLAGS (TC Q leaves Q = 3, outside lm_interbank's domain). */
 st=agc_transfer_run(&program,s,io,&io->superbank,LM_AL_SWRETURN_POINT,24,steps);
 return st==EXEC_UNMAPPED?EXEC_OK:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_al_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){p66_bias_io io;agc_hardware hw;uint16_t steps=0;unsigned i;
 *status=lm_al_run(s,&io,&hw,n,&steps);sum_a=sum_b=0;
 mix(io.interrupt_enabled);mix(io.superbank);mix(hw.in_interrupt);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
