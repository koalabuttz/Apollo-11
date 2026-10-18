/* SPDX-License-Identifier: MIT */
#define AGC_SITES_CALLEES
#include "interbank_oracle_cases.h"
#include "lm_interbank_routines.h"
#include "cm_interbank_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#include "../src/lm_interbank_sites.inc"
#include "../src/cm_interbank_sites.inc"
#include "interbank_oracle_data.h"
static uint32_t rnd(uint32_t *x){*x=*x*1103515245u+12345u;return *x>>8;}
#define IBO_SITES (LM_INTERBANK_SITES_COUNT+CM_INTERBANK_SITES_COUNT)
uint16_t ibo_cases(void){return (uint16_t)(IBO_SITES+2u*IBO_ENTRIES*IBO_PER_ENTRY);}
uint8_t ibo_mission(uint16_t n){if(n<LM_INTERBANK_SITES_COUNT)return 0;if(n<IBO_SITES)return 1;return (uint8_t)((n-IBO_SITES)%2u);}
/* Entry addresses, table order (SWCALL -1 last): BANKCALL SWCALL SWRETURN
 * POSTJUMP BANKJUMP QJUMP MAKECADR SUPDACAL IBNKCALL ISWCALL ISWRETRN USPRCADR
 * SUPERSW SWCALL-1. */
static const uint16_t entry_z[2][IBO_ENTRIES]={
 {04616u,04622u,04631u,04635u,04640u,04643u,04645u,04651u,04674u,04700u,04707u,04713u,04727u,04621u},
 {04662u,04666u,04675u,04701u,04704u,04707u,04711u,04715u,04740u,04744u,04753u,04757u,04773u,04665u}};
uint16_t ibo_data_physical(uint8_t cm,uint16_t i){uint32_t x=0xda7au+cm*977u+i*2654435761u;uint16_t bank=(uint16_t)(rnd(&x)%044u);
 /* Every fourth in banks 040-043 (SUPDACAL with superbank 0100), which the draw alone may miss (#198). */
 if(i%4u==3u)bank=(uint16_t)(040u+bank%4u);
 return (uint16_t)(bank*1024u+rnd(&x)%02000u);}
static uint8_t data_cm;
static bool data_word(const agc_native_program *p,uint16_t physical,agc_word *word){uint16_t i;uint8_t cm=data_cm;(void)p;
 for(i=0;i<IBO_DATA;++i)if(ibo_data_physical(cm,i)==physical){*word=ibo_data[cm][i];return true;}
 return false;}
/* Each entry case draws from its own index among the entry cases (after the
 * 802 site cases there were at first), so a site added to a table leaves the
 * entry cases as they were. */
#define IBO_ENTRY_SEED 802u
void ibo_input(apollo_overlay_state *s,uint16_t *superbank,uint16_t *inhint,uint16_t n){
 uint32_t x=0x1b0u+(n<IBO_SITES?n:(uint32_t)IBO_ENTRY_SEED+n-IBO_SITES)*2654435761u;
 uint16_t i,bank,fb,eb,phys,q,z,k;uint8_t m=ibo_mission(n);
 /* The image from a 16-bit xorshift (cheap on the 6502), the rest from the LCG. */
 {uint16_t r=(uint16_t)(rnd(&x)|1u);for(i=0;i<APOLLO_SHARED_WORDS;++i){r^=(uint16_t)(r<<7);r^=(uint16_t)(r>>9);r^=(uint16_t)(r<<8);s->words[i]=(uint16_t)(r&077777u);}}
 fb=(uint16_t)(rnd(&x)%040u);eb=(uint16_t)(rnd(&x)%8u);*superbank=(rnd(&x)&1u)?0100u:0060u;*inhint=(uint16_t)(rnd(&x)&1u);
 s->words[0]=(agc_word)(rnd(&x)&0177777u);s->words[1]=(agc_word)(rnd(&x)&0177777u);
 /* A callee CADR in banks 04-37 (sign-extended in A) for the entries that take one in A. */
 if(rnd(&x)%4u)s->words[0]=agc_extend((agc_word)(((4u+rnd(&x)%034u)<<10)|(rnd(&x)&01777u)));
 if(n<IBO_SITES){
  const agc_site_operand *t=m?cm_interbank_sites:lm_interbank_sites;const agc_word *callee=m?cm_interbank_sites_callee:lm_interbank_sites_callee;
  k=(uint16_t)(m?n-LM_INTERBANK_SITES_COUNT:n);phys=t[k].physical;bank=(uint16_t)(phys/1024u);z=callee[k];
  if(bank==2u||bank==3u)q=(uint16_t)(04000u+(phys-2048u));
  else{q=(uint16_t)(02000u+phys%1024u);if(bank>=040u){fb=(uint16_t)(bank-8u);*superbank=0100u;}else{fb=bank;if(bank>=030u)*superbank=0060u;}}
 }else{
  k=(uint16_t)((n-IBO_SITES)/2u%IBO_ENTRIES);z=entry_z[m][k];
  q=(uint16_t)(0100u+rnd(&x)%(07775u-0100u));
  /* Inline-CADR entries: the operand word in unswitched erasable. */
  if(k==0u||k==3u||k==8u||k==11u){q=(uint16_t)(0100u+rnd(&x)%01200u);s->words[q]=(agc_word)(((4u+rnd(&x)%034u)<<10)|(rnd(&x)&01777u));}
  /* SUPDACAL: A the data CADR, L's bits 7-5 the superbank bits. */
  if(k==7u){phys=ibo_data_physical(m,(uint16_t)(rnd(&x)%IBO_DATA));bank=(uint16_t)(phys/1024u);
   s->words[1]=(agc_word)(rnd(&x)&077437u);if(bank>=040u){bank=(uint16_t)(bank-8u);s->words[1]|=0100u;}else if(bank>=030u)s->words[1]|=0060u;
   s->words[0]=agc_extend((agc_word)((bank<<10)|(phys%1024u)));}
  /* SWRETURN and ISWRETRN return through the 2CADR at BUF2 or RUPTREG3. */
  /* (A return into the file's own code would stop the oracle mid-routine: those words are skipped.) */
  if(k==2u||k==10u){uint16_t r=(uint16_t)(0100u+rnd(&x)%07700u);if(r>=04600u&&r<05000u)r=(uint16_t)(r+01000u);s->words[k==2u?0133u:072u]=r;s->words[k==2u?0134u:073u]=(agc_word)((rnd(&x)%040u)<<10);}
  /* QJUMP: Q a bank-relative address. */
  if(k==5u)q=(uint16_t)(0100u+rnd(&x)%01700u);
  /* SWRETURN, ISWRETRN, BANKJUMP: Q anywhere half the time. */
  if((k==2u||k==4u||k==10u)&&(rnd(&x)&1u))q=(uint16_t)(rnd(&x)%010000u);
 }
 /* A return into the file's own code would stop the oracle mid-routine (it stops at the first Z equal to
  * the native one): such Q move one bank-page up. Site Q are genuine. */
 if(n>=IBO_SITES&&q>=04600u&&q<05000u)q=(uint16_t)(q+01000u);
 s->words[2]=q;s->words[3]=(agc_word)(eb<<8);s->words[4]=(agc_word)(fb<<10);s->words[6]=(agc_word)((fb<<10)|eb);s->words[5]=z;s->words[7]=0;
}
unsigned ibo_run(apollo_overlay_state *s,uint16_t *superbank,uint16_t *inhint,uint16_t n){static agc_routine_table t[1];agc_native_program p={0};
 p66_bias_io lio={0};cm_native_io cio={0};void *io;enum exec_status st;uint16_t steps,count;uint8_t cm=ibo_mission(n);
 ibo_input(s,superbank,inhint,n);
 if(cm){t[0]=cm_interbank_routines;cio.superbank=*superbank;cio.interrupt_enabled=!*inhint;io=&cio;p.operands=cm_interbank_site_table(&count);}
 else{t[0]=lm_interbank_routines;lio.superbank=*superbank;lio.interrupt_enabled=(uint16_t)!*inhint;io=&lio;p.operands=lm_interbank_site_table(&count);}
 p.operand_count=count;p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;p.fixed_data=data_word;data_cm=cm;
 st=agc_transfer_run(&p,s,io,cm?&cio.superbank:&lio.superbank,AGC_TRANSFER_NONE,IBO_MAX_STEPS,&steps);
 *superbank=cm?(uint16_t)cio.superbank:lio.superbank;*inhint=(uint16_t)!(cm?cio.interrupt_enabled:lio.interrupt_enabled);
 return st==EXEC_UNMAPPED&&steps?1u:0u;
}
uint32_t ibo_digest(apollo_overlay_state *s,uint16_t n){uint16_t superbank,inhint,i,a=0,b=0,v;unsigned k=ibo_run(s,&superbank,&inhint,n);
 a=(uint16_t)(a+k);b=(uint16_t)(b+a);
 if(k){a=(uint16_t)(a+inhint);b=(uint16_t)(b+a);a=(uint16_t)(a+superbank);b=(uint16_t)(b+a);
  for(i=0;i<APOLLO_SHARED_WORDS;++i){v=(uint16_t)(s->words[i]&(i<3u?0177777u:077777u));a=(uint16_t)(a+v);b=(uint16_t)(b+a);}}
 return (uint32_t)b<<16|a;
}
