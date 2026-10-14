/* SPDX-License-Identifier: MIT */
#include "standard_lead_oracle_cases.h"
#include "standard_lead_routines.h"
#include "standard_lead_cases.h"
#include "lm_interbank_routines.h"
#include "cm_interbank_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"
static uint32_t rnd(uint32_t *x){*x=*x*1103515245u+12345u;return *x>>8;}
uint8_t slo_mission(uint16_t n){return (uint8_t)(n%2u);}
uint8_t slo_kind(uint16_t n){return (uint8_t)((n/2u)%SLO_KINDS);}
/* Kinds 0-6 are standard_lead_input's phases (E/BKCALL, RETURN0-2, E/CALL,
 * resume, E/JOBWAK's return) at the registered entry; 7 INTPRET from E/CALL,
 * 8 INTPRET+2, 9 EXIT and 10 CALL at the fetch point. */
void slo_input(apollo_overlay_state *s,uint16_t *superbank,uint16_t *inhint,uint16_t n){
 uint8_t cm=slo_mission(n),kind=slo_kind(n);uint16_t j=(uint16_t)(n/(2u*SLO_KINDS)),k,address,ebuf=cm?0364u:01167u,eb;
 uint32_t x=0x51d0u+n*2654435761u;enum standard_lead_phase phase;agc_word fixed,fb=cm?026000u:010000u,callee;
 k=(uint16_t)(j*4u+1u+j%3u);
 standard_lead_input(s,cm?STANDARD_LEAD_CM:STANDARD_LEAD_LM,(uint16_t)((kind<7u?kind:4u)*512u+k),&phase,&fixed);
 *superbank=(rnd(&x)&1u)?0u:0060u;*inhint=(uint16_t)(rnd(&x)&1u);
 callee=(agc_word)(((020u+rnd(&x)%020u)<<10)|(rnd(&x)%02000u));
 eb=(uint16_t)(s->words[3]>>8);
 address=kind==0u?(uint16_t)(s->words[0133]&07777u):(uint16_t)((s->words[0164]+1u)&07777u);
 if(address>=01400u&&address<02000u)address=(uint16_t)(s->words[3]+(address&0377u));
 if((kind==0u||kind==4u)&&address<02000u)s->words[address]=callee;
 if(kind==4u||kind==5u)s->words[0120]=(agc_word)(0401u+054u*(j%5u));
 /* E/JOBWAK: called inhibited, returning through RUPTREG3. */
 if(kind==6u){uint16_t i;s->words[5]=cm?02606u:02631u;*inhint=1;
  /* E/JOBWAK is in bank 4 in both missions (FB 010000, BBANK coherent). */
  s->words[4]=010000u;s->words[6]=(agc_word)(010000u|(s->words[3]>>8));s->words[072]=(agc_word)(02000u+rnd(&x)%06000u);
  /* A return inside JOBWAKE (bank-0 Executive code, 05000-05777) would stop the oracle mid-routine (it
   * stops at the first Z equal to the native one): such returns move to 04000-04777. */
  if(s->words[072]>=05000u&&s->words[072]<06000u)s->words[072]=(agc_word)(s->words[072]-01000u);
  /* Nor inside E/JOBWAK itself (CM 02606-02612, LM 02631-02635, FB bank 4): those move up 040. */
  if(s->words[072]>=02606u&&s->words[072]<02636u)s->words[072]=(agc_word)(s->words[072]+040u);
  /* Core sets in use (nonzero priority), half of them sleeping; A names one of their LOCs or none.
   * BANKSET a plain EB with the superbank bits, so a found job can be woken (#198). */
  for(i=0;i<8u;++i){s->words[0167u+12u*i]=(agc_word)((rnd(&x)&1u)?040000u|(1u+rnd(&x)%037777u):1u+rnd(&x)%037777u);s->words[0164u+12u*i]=(agc_word)(rnd(&x)%077777u);
   s->words[0165u+12u*i]=(agc_word)((rnd(&x)&7u)|*superbank);}
  s->words[067]=(agc_word)(rnd(&x)%4u?(rnd(&x)%(cm?7u:8u))*12u:AGC_NEG_ZERO);
  s->words[0]=agc_extend((agc_word)(rnd(&x)%4u?s->words[0164u+12u*(rnd(&x)%8u)]:rnd(&x)%077777u));}
 if(kind>=7u){
  s->words[0120]=(agc_word)(0401u+054u*(j%5u));s->words[ebuf]=callee;s->words[ebuf+1u]=(agc_word)(0300u+j);
  s->words[4]=fb;s->words[6]=(agc_word)(fb|eb);
  if(kind==7u){s->words[5]=cm?06006u:06037u;s->words[2]=cm?02064u:02624u;}
  if(kind==8u){s->words[5]=cm?06010u:06041u;s->words[1]=(agc_word)(0300u+j);}
  if(kind>=9u){s->words[5]=cm?06037u:06070u;s->words[0164]=(agc_word)(kind==9u?(cm?02066u:02626u):(cm?02064u:02624u));
   /* At the fetch point BBANK has been loaded from BANKSET. */
   eb=(uint16_t)(rnd(&x)%8u);s->words[0165]=(agc_word)(fb|eb);s->words[3]=(agc_word)(eb<<8);s->words[6]=s->words[0165];}
 }
}
static agc_routine_table basic[2][3],interp[2][1];static agc_native_program prog[2];static int ready;
static void setup(void){
 basic[0][0]=standard_lead_routines;basic[0][1]=lm_interbank_routines;basic[0][2]=lm_interpreter_routines;interp[0][0]=standard_lead_interpretive_routines;
 basic[1][0]=cm_standard_lead_routines;basic[1][1]=cm_interbank_routines;basic[1][2]=cm_interpreter_routines;interp[1][0]=cm_standard_lead_interpretive_routines;
 prog[0].basic=basic[0];prog[1].basic=basic[1];prog[0].basic_count=prog[1].basic_count=3;
 prog[0].interpretive=interp[0];prog[1].interpretive=interp[1];prog[0].interpretive_count=prog[1].interpretive_count=1;
 prog[0].interpreter_fetch=LM_INTERPRETER_FETCH;prog[1].interpreter_fetch=CM_INTERPRETER_FETCH;
 prog[0].loc=prog[1].loc=0164;prog[0].bankset=prog[1].bankset=0165;ready=1;}
unsigned slo_run(apollo_overlay_state *s,uint16_t *superbank,uint16_t *inhint,uint16_t n){
 p66_bias_io lio={0};cm_native_io cio={0};void *io;enum exec_status st;uint16_t steps;uint8_t cm=slo_mission(n);agc_word sb;
 if(!ready)setup();
 slo_input(s,superbank,inhint,n);sb=*superbank;
 if(cm){cio.superbank=sb;cio.interrupt_enabled=(agc_word)!*inhint;io=&cio;}else{lio.superbank=sb;lio.interrupt_enabled=(uint16_t)!*inhint;io=&lio;}
 st=agc_transfer_run(&prog[cm],s,io,cm?&cio.superbank:&lio.superbank,AGC_TRANSFER_NONE,SLO_MAX_STEPS,&steps);
 *superbank=cm?(uint16_t)cio.superbank:lio.superbank;*inhint=(uint16_t)!(cm?cio.interrupt_enabled:lio.interrupt_enabled);
 /* EXEC_OK: the run ended where no point applies (an erasable LOC at the fetch point);
  * EXEC_NOT_FOUND: E/JOBWAK returned after a JOBWAKE that found no sleeping job. */
 return (st==EXEC_UNMAPPED||st==EXEC_OK||st==EXEC_NOT_FOUND)&&steps?1u:0u;
}
uint32_t slo_digest(apollo_overlay_state *s,uint16_t n){uint16_t superbank,inhint,i,a=0,b=0,v;unsigned k=slo_run(s,&superbank,&inhint,n);
 a=(uint16_t)(a+k);b=(uint16_t)(b+a);
 if(k){a=(uint16_t)(a+inhint);b=(uint16_t)(b+a);a=(uint16_t)(a+superbank);b=(uint16_t)(b+a);
  for(i=0;i<APOLLO_SHARED_WORDS;++i){v=(uint16_t)(s->words[i]&(i<3u?0177777u:077777u));a=(uint16_t)(a+v);b=(uint16_t)(b+a);}}
 return (uint32_t)b<<16|a;
}
