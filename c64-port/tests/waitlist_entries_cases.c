/* SPDX-License-Identifier: MIT */
#include "waitlist_entries_cases.h"
#include "lm_exec_jobs_cases.h"
#include "cm_exec_services_cases.h"
#include "service_transfer_cases.h"
#include "service_transfer.h"
#include "lm_exec_routines.h"
#include "cm_exec_routines.h"
#include "lm_t6rupt_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"
/* WLE_ONLY=k (the C64 programs) builds only kind k's generator and tables. */
#ifdef WLE_ONLY
#define HAS(k) (WLE_ONLY==(k))
#else
#define HAS(k) 1
#endif
/* The CM DLY2 -1 C64 program links only the two entries it enters (a case
 * leaving them stops early and fails its digest). */
#if defined(CM_EXEC_DLY2_ONLY)&&defined(CM_SERVICE_DELAYJOB_ONLY)
#define WLE_CM_SERVICE cm_service_delayjob_routines
#define WLE_CM_EXEC cm_exec_dly2_routines
#else
#define WLE_CM_SERVICE cm_service_routines
#define WLE_CM_EXEC cm_exec_routines
#endif
#ifndef WLE_SCAN
#include "waitlist_entries_vectors.h"
static const uint16_t counts[WLE_KINDS]={WLE_COUNT_0,WLE_COUNT_1,WLE_COUNT_2,WLE_COUNT_3,WLE_COUNT_4};
uint16_t wle_cases(void){return WLE_CASES;}
uint8_t wle_kind(uint16_t n){uint8_t k=0;while(k<WLE_KINDS-1u&&n>=counts[k]){n=(uint16_t)(n-counts[k]);++k;}return k;}
static uint16_t index_in_kind(uint16_t n){uint8_t k=0;while(k<WLE_KINDS-1u&&n>=counts[k]){n=(uint16_t)(n-counts[k]);++k;}return n;}
uint16_t wle_case_base_of(uint16_t n){uint16_t i=index_in_kind(n);switch(wle_kind(n)){
#if HAS(0)
 case 0:return wle_base_0[i];
#endif
#if HAS(1)
 case 1:return wle_base_1[i];
#endif
#if HAS(2)
 case 2:return wle_base_2[i];
#endif
#if HAS(3)
 case 3:return wle_base_3[i];
#endif
#if HAS(4)
 case 4:return wle_base_4[i];
#endif
 default:return 0;}}
uint32_t wle_frozen_digest(uint16_t n){uint16_t i=index_in_kind(n);switch(wle_kind(n)){
#if HAS(0)
 case 0:return wle_digest_0[i];
#endif
#if HAS(1)
 case 1:return wle_digest_1[i];
#endif
#if HAS(2)
 case 2:return wle_digest_2[i];
#endif
#if HAS(3)
 case 3:return wle_digest_3[i];
#endif
#if HAS(4)
 case 4:return wle_digest_4[i];
#endif
 default:return 0;}}
#endif
#if HAS(4)
static uint32_t rnd(uint32_t *x){*x=*x*1103515245u+12345u;return *x>>8;}
#endif
void wle_io(const void *io,uint8_t kind,uint16_t *superbank,uint16_t *enabled){
 if(kind==1u||kind==3u){const cm_native_io *c=io;*superbank=(uint16_t)c->superbank;*enabled=(uint16_t)c->interrupt_enabled;}
 else{const p66_bias_io *l=io;*superbank=l->superbank;*enabled=l->interrupt_enabled;}}
#if HAS(0)||HAS(1)
/* The T3 vector's own words: DXCH ARUPT; CAF T3RPTBB; XCH BBANK. */
static void vector_words(apollo_overlay_state *s,agc_word z){agc_word a=s->words[0],l=s->words[1];
 s->words[0]=agc_extend(s->words[010]);s->words[1]=agc_extend(s->words[011]);s->words[010]=agc_correct(a);s->words[011]=agc_correct(l);
 s->words[0]=agc_extend(s->words[6]);s->words[6]=02003u;s->words[3]=01400u;s->words[4]=02000u;s->words[5]=z;}
#endif
void wle_input(apollo_overlay_state *s,void *io,agc_hardware *hw,uint8_t kind,uint16_t base){agc_hardware zero={0};*hw=zero;(void)kind;
 if(0){}
#if HAS(0)
 else if(kind==0u){static lm_exj_case c;lm_exj_input(s,io,&c,base);*hw=c.hw;vector_words(s,03407u);}
#endif
#if HAS(1)
 else if(kind==1u){static cm_exs_case c;cm_exs_input(s,io,&c,base);*hw=c.hw;vector_words(s,03416u);}
#endif
#if HAS(2)||HAS(3)
 else if(kind==2u||kind==3u){uint16_t superbank,inhint;svt_input(s,&superbank,&inhint,base);
  if(kind==2u){p66_bias_io *l=io;l->superbank=superbank;l->interrupt_enabled=(uint16_t)!inhint;}
  else{cm_native_io *c=io,cz={0};*c=cz;c->superbank=superbank;c->interrupt_enabled=!inhint;}}
#endif
#if HAS(4)
 else{uint32_t x=0x2e5u+base*2654435761u;uint16_t i,fb,eb;p66_bias_io *l=io;
  for(i=0;i<APOLLO_SHARED_WORDS;++i)s->words[i]=(uint16_t)(rnd(&x)&077777u);
  fb=(uint16_t)(rnd(&x)%040u);eb=(uint16_t)(rnd(&x)%8u);
  s->words[0]=(agc_word)(rnd(&x)&0177777u);s->words[1]=agc_extend((agc_word)(rnd(&x)&077777u));s->words[2]=(agc_word)(rnd(&x)&0177777u);
  s->words[3]=(agc_word)(eb<<8);s->words[4]=(agc_word)(fb<<10);s->words[6]=(agc_word)((fb<<10)|eb);s->words[5]=05275u;s->words[7]=0;
  s->words[015]=(agc_word)(1u+rnd(&x)%010000u);
  /* The interrupt saved the instruction at ZRUPT-1 in BRUPT (#144). */
  {agc_word at=(agc_word)(s->words[015]-1u);if(at<01400u)s->words[017]=s->words[at];else if(at<02000u)s->words[017]=s->words[eb*0400u+(at-01400u)];}
  l->superbank=(rnd(&x)&1u)?0100u:0060u;l->interrupt_enabled=0;hw->in_interrupt=1;}
#endif
}
unsigned wle_run_base(apollo_overlay_state *s,void *io,agc_hardware *hw,uint8_t kind,uint16_t base){agc_routine_table t[2];agc_native_program p={0};
 enum exec_status st;uint16_t steps,superbank,enabled;
 wle_input(s,io,hw,kind,base);p.basic=t;p.interpreter_fetch=AGC_TRANSFER_NONE;p.hardware=hw;
 switch(kind){
#if HAS(0)
  case 0:t[0]=lm_exec_task_routines;p.basic_count=1;break;
#endif
#if HAS(1)
  case 1:t[0]=cm_exec_routines;p.basic_count=1;p.operands=cm_exec_site_table(&p.operand_count);break;
#endif
#if HAS(2)
  case 2:t[0]=lm_service_routines;t[1]=lm_exec_task_routines;p.basic_count=2;p.operands=lm_service_site_table(&p.operand_count);break;
#endif
#if HAS(3)
  case 3:t[0]=WLE_CM_SERVICE;t[1]=WLE_CM_EXEC;p.basic_count=2;p.operands=cm_service_site_table(&p.operand_count);break;
#endif
#if HAS(4)
  case 4:t[0]=lm_t6rupt_routines;p.basic_count=1;break;
#endif
  default:return 0;}
 /* T3RUPT runs from the T3RUPT word itself, never from the vector. */
 wle_io(io,kind,&superbank,&enabled);
 st=agc_transfer_run(&p,s,io,(kind==1u||kind==3u)?(agc_word *)&((cm_native_io *)io)->superbank:&((p66_bias_io *)io)->superbank,AGC_TRANSFER_NONE,16,&steps);
 (void)superbank;(void)enabled;
 return st==EXEC_UNMAPPED&&steps>0u?1u:0u;
}
#ifndef WLE_SCAN
unsigned wle_run(apollo_overlay_state *s,void *io,agc_hardware *hw,uint16_t n){return wle_run_base(s,io,hw,wle_kind(n),wle_case_base_of(n));}
uint32_t wle_digest(apollo_overlay_state *s,uint16_t n){static wle_io_storage io;agc_hardware hw;uint16_t i,a=0,b=0,v,superbank,enabled;unsigned k=wle_run(s,&io,&hw,n);
#define MIX(x) do{a=(uint16_t)(a+(x));b=(uint16_t)(b+a);}while(0)
 MIX(k);
 if(k){wle_io(&io,wle_kind(n),&superbank,&enabled);MIX(superbank);MIX(enabled);MIX(hw.in_interrupt);
  for(i=0;i<APOLLO_SHARED_WORDS;++i){v=(uint16_t)(s->words[i]&(i<3u?0177777u:077777u));MIX(v);}}
 return (uint32_t)b<<16|a;
}
#endif
