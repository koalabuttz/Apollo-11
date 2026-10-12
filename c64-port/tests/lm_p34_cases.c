/* SPDX-License-Identifier: MIT */
#include "lm_p34_cases.h"
#include "lm_vn1645_cases.h"
#include "lm_midgim_cases.h"
#ifndef LM_P34_NO_TRAMPOLINE
#include "lm_interbank_routines.h"
#define LM_INTERBANK_SITES_MAX 448u
#endif
#define W m->words
#define FB35 072000u
/* The points this file transfers to and the alarm frontiers its steps reach. */
static const lm_p34_stop common[]={
 {02432u,FB35,LM_P34_ANY_Q},          /* COMPTGO */
 {03735u,0u,LM_P34_ANY_Q},            /* DELAYJOB, through BANKCALL */
 {02476u,020000u,LM_P34_ANY_Q},       /* GOFLASH, through BANKCALL */
 {05353u,LM_P34_ANY_FB,LM_P34_ANY_Q}, /* PHASCHNG */
 {05504u,LM_P34_ANY_FB,LM_P34_ANY_Q}, /* UPFLAG */
 {06001u,LM_P34_ANY_FB,LM_P34_ANY_Q}, /* GOTOPOOH */
 {06037u,LM_P34_ANY_FB,03671u},       /* INTPRET from CLUPDATE */
 {05726u,LM_P34_ANY_FB,03404u},       /* POODOO1 from UNIT (alarm 1302) */
 {03723u,0u,LM_P34_ANY_Q},            /* ARCSIN 1301 */
 {06070u,LM_P34_ANY_FB,LM_P34_ANY_Q}, /* NEWOPS outside this file */
 {0,0,0}};
#define COMMON LM_P34_COMMON_STOPS
typedef char lm_p34_common_check[(sizeof common/sizeof *common-1u)==COMMON?1:-1];
int lm_p34_owned_newops(agc_word fb,agc_word loc){
 return (fb==FB35&&((loc>=03606u&&loc<=03634u)||(loc>=03671u&&loc<=03673u)))
  ||(fb==014000u&&loc>=03715u&&loc<=03752u)||(fb==020000u&&loc>=02000u&&loc<=02027u);
}
#if !defined LM_P34_C64_GROUP || LM_P34_C64_GROUP<=2
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
#endif
#if !defined LM_P34_C64_GROUP || LM_P34_C64_GROUP==1
static const agc_word entries[LM_P34_BASIC_ROUTINES]={03635,03636,03643,03646,03647,03650,03651,03654,03662,03664,
 03670,03674,03701,03702,03703,03704,03712,03720,03721,03722};
static void add(lm_p34_case *c,agc_word z,agc_word fb){unsigned i;
 for(i=0;i<LM_P34_STOPS-1u&&c->stop[i].z;++i);
 c->stop[i].z=z;c->stop[i].fb=fb;c->stop[i].q=LM_P34_ANY_Q;c->stop[i+1u].z=0;
}
/* A return a caller in bank 35 or elsewhere leaves: VN1645's own (35,3626)
 * or any bank-35 word below this file's routines. */
static agc_word caller(uint32_t *seed,unsigned v){return v%4u==0u?03626u:(agc_word)(02000u+(rnd(seed)>>16)%01600u);}
static void basic_input(apollo_overlay_state *m,p66_bias_io *io,lm_p34_case *c,uint32_t n){
 uint32_t seed=0x34e5ul+n*11u;unsigned i,r=n%LM_P34_BASIC_ROUTINES,v=n/LM_P34_BASIC_ROUTINES;agc_word z=entries[r],eb;
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;for(i=010;i<=061u;++i)W[i]=0;
 W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 /* Overflowing A in one state in eight (VNPOOH's TS skips). */
 if(v%8u==5u)W[0]=(agc_word)((v&16u?0100000u:040000u)|((rnd(&seed)>>18)&037777u));
 eb=(agc_word)(((rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=FB35;W[6]=(agc_word)(FB35|(eb>>8));W[5]=z;
 W[076]=(agc_word)((W[076]&~040u)|((v&1u)?040u:0u));            /* FINALFLG */
 {static const agc_word modes[4]={34,74,0,0};unsigned k=(v/2u)%4u;
  W[01011]=k==3u?(agc_word)((rnd(&seed)>>17)&AGC_MASK):(agc_word)(modes[k]+(k<2u?(rnd(&seed)>>20)%4u:0u));}
 io->superbank=(uint16_t)(((v/8u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/4u)&1u);
 c->refused=0;
 for(i=0;i<=COMMON;++i){c->stop[i]=common[i];}
 if(z==03704u){W[2]=caller(&seed,v);add(c,W[2],FB35);add(c,(agc_word)(W[2]+1u),FB35);}
 else W[2]=caller(&seed,v+1u);
 if(z==03702u||z==03721u){unsigned cell=(z==03702u?01463u:01465u)-01400u+eb;agc_word w=caller(&seed,v/2u);
  if(v%16u==7u){w=(agc_word)((rnd(&seed)>>16)&01777u);c->refused=1;}
  else if(v%16u==9u){w=(agc_word)(010000u|((rnd(&seed)>>16)&067777u));c->refused=1;}
  W[cell]=w;add(c,w,FB35);
 }
}
#endif
#if !defined LM_P34_C64_GROUP || LM_P34_C64_GROUP==2
/* VN1645's interpretive returns, or a CADR in a bank this file has no
 * interpretive code in. */
static agc_word cadr(uint32_t *seed,unsigned v){unsigned bank;
 if(v%4u==0u)return 073634u;
 if(v%4u==1u)return 073606u;
 do bank=4u+(rnd(seed)>>16)%034u;while(bank==06u||bank==010u||bank==035u);
 return (agc_word)((bank<<10)|((rnd(seed)>>16)&01777u));
}
#endif
#if defined LM_P34_C64_GROUP && LM_P34_C64_GROUP==1
/* The C64 basic group builds no interpretive-entry case. */
#elif defined LM_P34_C64_GROUP
/* The C64 has no floating point for the lm-vn1645 and lm-midgim orbit
 * words: each sampled case's debris plus the words its generator changes,
 * frozen by tools/lm_p34_differential.py. */
#include "lm_p34_c64_inputs.h"
static void frozen(apollo_overlay_state *m,uint32_t n){uint16_t k,i;
 for(k=0;k<LM_P34_C64_INPUTS;++k)if(lm_p34_c64_input_case[k]==n){
  for(i=lm_p34_c64_input_first[k];i<lm_p34_c64_input_first[k+1u];++i)W[lm_p34_c64_delta[i].address]=lm_p34_c64_delta[i].word;
  return;}
 W[5]=0; /* not a sampled case: no routine accepts it */
}
#if LM_P34_C64_GROUP==2
static void vn_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n,uint32_t k){lm_vn1645_debris(m,io,k);frozen(m,n);}
#endif
static void mg_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n,uint32_t k){lm_midgim_debris(m,io,k);frozen(m,n);}
#else
static void vn_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n,uint32_t k){(void)n;lm_vn1645_input(m,io,k);}
static void mg_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n,uint32_t k){(void)n;lm_midgim_input(m,io,k);}
#endif
/* A C64 group compiles only the case kinds it samples (C64 RAM). */
#if !defined LM_P34_C64_GROUP
#define BASIC_KINDS 1
#define VN_KINDS 1
#define MG_KINDS 1
#elif LM_P34_C64_GROUP==1
#define BASIC_KINDS 1
#define VN_KINDS 0
#define MG_KINDS 0
#else
#define BASIC_KINDS 0
#define VN_KINDS (LM_P34_C64_GROUP==2)
#define MG_KINDS 1
#endif
void lm_p34_input(apollo_overlay_state *m,p66_bias_io *io,lm_p34_case *c,uint32_t n){unsigned i;uint32_t all=n;
#if BASIC_KINDS
 if(n<LM_P34_BASIC_CASES){basic_input(m,io,c,n);return;}
#endif
 for(i=0;i<=COMMON;++i){c->stop[i]=common[i];}
 c->refused=0;n-=LM_P34_BASIC_CASES;
#if VN_KINDS
 if(n<LM_P34_CLUPDATE_CASES){uint32_t seed=0x3671ul+n*13u;
  vn_input(m,io,all,n);W[0164]=03671u;W[03464]=cadr(&seed,n);return;}
 n-=LM_P34_CLUPDATE_CASES;
 if(n<LM_P34_VN1645_CASES){vn_input(m,io,all,n);return;}
 n-=LM_P34_VN1645_CASES;
#else
 n-=LM_P34_CLUPDATE_CASES+LM_P34_VN1645_CASES;
#endif
#if MG_KINDS
 mg_input(m,io,all,n);
#else
 (void)all;W[5]=0; /* not built in this C64 group: no routine accepts it */
#endif
}
#ifndef LM_P34_NO_TRAMPOLINE
#ifndef LM_P34_C64_GROUP
/* lm_interbank_sites and this file's TCR sites merged by address. */
static agc_site_operand operands[LM_INTERBANK_SITES_MAX+LM_P34_SITES];
static const agc_site_operand *merged(uint16_t *count){static uint16_t total;
 if(!total){uint16_t a,b,i=0,j=0;const agc_site_operand *x=lm_interbank_site_table(&a),*y=lm_p34_site_table(&b);
  if(a>LM_INTERBANK_SITES_MAX){*count=0;return operands;}
  while(i<a||j<b)operands[total++]=(j>=b||(i<a&&x[i].physical<y[j].physical))?x[i++]:y[j++];}
 *count=total;return operands;
}
static agc_routine_table basic[2],interp[1];
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
const agc_native_program *lm_p34_program(void){
 if(!program.basic){basic[0]=lm_p34_basic_routines;basic[1]=lm_interbank_routines;interp[0]=lm_p34_interpretive_routines;
  program.basic=basic;program.basic_count=2;program.interpretive=interp;program.interpretive_count=1;
  program.operands=merged(&program.operand_count);program.interpreter_fetch=LM_P34_FETCH;program.loc=0164;program.bankset=0165;}
 return &program;
}
unsigned lm_p34_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){return lm_p34_run_program(lm_p34_program(),s,io,n);}
#endif
unsigned lm_p34_run_program(const agc_native_program *p,apollo_overlay_state *s,p66_bias_io *io,uint32_t n){lm_p34_case c;enum exec_status st;uint16_t steps;
 lm_p34_input(s,io,&c,n);
 st=agc_transfer_run(p,s,io,&io->superbank,0xfffeu,64,&steps);
 if(st==EXEC_UNMAPPED)return LM_P34_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_P34_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_p34_digest(const agc_native_program *p,apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_p34_run_program(p,s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_P34_STOPPED?1u:kind==LM_P34_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
