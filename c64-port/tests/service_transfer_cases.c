/* SPDX-License-Identifier: MIT */
#define AGC_SITES_CALLEES
#include "service_transfer_cases.h"
#include "service_transfer.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#include "../src/lm_service_sites.inc"
#include "../src/cm_service_sites.inc"
static uint32_t rnd(uint32_t *x){*x=*x*1103515245u+12345u;return *x>>8;}
/* Site rows that are the first operand word after the TC (GENTRAN's second ADRES is not a call). */
static uint16_t first_rows(const agc_site_operand *t,const agc_word *callee,uint16_t count,uint16_t *rows){uint16_t i,k=0;
 for(i=0;i<count;++i){if(i&&callee[i]==callee[i-1u]&&t[i].physical==t[i-1u].physical+1u&&(callee[i]==05544u||callee[i]==05606u))continue;if(rows)rows[k]=i;++k;}return k;}
static uint16_t lm_rows[LM_SERVICE_SITES_COUNT],cm_rows[CM_SERVICE_SITES_COUNT],lm_n,cm_n;
static void init(void){if(lm_n)return;lm_n=first_rows(lm_service_sites,lm_service_sites_callee,LM_SERVICE_SITES_COUNT,lm_rows);cm_n=first_rows(cm_service_sites,cm_service_sites_callee,CM_SERVICE_SITES_COUNT,cm_rows);}
uint16_t svt_cases(void){init();return (uint16_t)(2u*lm_n+2u*cm_n+2u*SVT_ENTRY_CASES);}
uint8_t svt_mission(uint16_t n){init();if(n<2u*lm_n)return 0;if(n<2u*(lm_n+cm_n))return 1;return (uint8_t)((n-2u*(lm_n+cm_n))%2u);}
/* Entry kinds: z, FB bank required (0xff: any), reads Q. */
static const struct {uint16_t z;uint8_t bank,q,cm_only;} kinds[2][9]={
 {{03544u,010u,1,0},{03554u,010u,1,0},{03735u,0u,1,0},{03761u,0u,0,0},{03765u,0u,0,0},{03771u,0u,0,0},{05563u,0xffu,0,0},{05522u,0xffu,1,0},{05511u,0xffu,0,0}},
 {{07722u,0xffu,1,0},{07740u,0xffu,1,0},{03732u,0u,1,0},{03755u,0u,0,0},{03761u,0u,0,0},{03765u,0u,0,0},{05625u,0xffu,0,0},{05631u,0xffu,1,1},{05643u,0xffu,1,1}}};
void svt_input(apollo_overlay_state *s,uint16_t *superbank,uint16_t *inhint,uint16_t n){uint32_t x=0x5e41u+n*2654435761u;uint16_t i,bank,fb,eb,m,k,row,phys,q,z;
 const agc_site_operand *t;const agc_word *callee;
 init();for(i=0;i<APOLLO_SHARED_WORDS;++i)s->words[i]=(uint16_t)(rnd(&x)&077777u);
 m=svt_mission(n);fb=(uint16_t)(rnd(&x)%040u);eb=(uint16_t)(rnd(&x)%8u);*superbank=(rnd(&x)&1u)?0100u:0060u;*inhint=(uint16_t)(rnd(&x)&1u);
 s->words[0]=(agc_word)(rnd(&x)&0177777u);s->words[1]=(agc_word)(rnd(&x)&0177777u);
 if(n<2u*(lm_n+cm_n)){
  k=(uint16_t)(m?(n-2u*lm_n)/2u:n/2u);row=m?cm_rows[k]:lm_rows[k];t=m?cm_service_sites:lm_service_sites;callee=m?cm_service_sites_callee:lm_service_sites_callee;
  phys=t[row].physical;bank=(uint16_t)(phys/1024u);z=callee[row];
  if(bank==2u||bank==3u)q=(uint16_t)(04000u+(phys-2048u));
  else{q=(uint16_t)(02000u+phys%1024u);if(bank>=040u){fb=(uint16_t)(bank-8u);*superbank=0100u;}else{fb=bank;if(bank>=030u)*superbank=0060u;}}
  /* GENTRAN: a short block (A = items - 1). */
  if(z==05544u||z==05606u)s->words[0]=(agc_word)(rnd(&x)%8u);
 }else{
  k=(uint16_t)((n-2u*(lm_n+cm_n))/2u%9u);
  z=kinds[m][k].z;if(kinds[m][k].bank!=0xffu)fb=kinds[m][k].bank;
  if(kinds[m][k].q||(rnd(&x)&1u))q=(uint16_t)(02000u+rnd(&x)%(07776u-02000u));else q=(uint16_t)(rnd(&x)%0100u);
  if(rnd(&x)&1u){uint16_t delayloc=m?01141u:01326u,count=m?3u:2u;for(i=0;i<=count;++i)if(rnd(&x)&1u)s->words[delayloc+i]=0;}
  s->words[070]=(agc_word)(rnd(&x)%(m?4u:3u));
 /* LM DEBIT: A at an ADRES word, a genuine site's (fixed) or an erasable one,
  * sometimes ITEMP1/ITEMP3 or an overflowing A. COMFLAG: ITEMP1 a flag word
  * index, ITEMP3 or L the return TC, A sometimes overflowing. */
  if(!m&&z==05522u){
   if(rnd(&x)&1u){row=lm_rows[rnd(&x)%lm_n];phys=lm_service_sites[row].physical;bank=(uint16_t)(phys/1024u);
    if(bank==2u||bank==3u)s->words[0]=(agc_word)(04000u+(phys-2048u));
    else{s->words[0]=(agc_word)(02000u+phys%1024u);if(bank>=040u){fb=(uint16_t)(bank-8u);*superbank=0100u;}else{fb=bank;if(bank>=030u)*superbank=0060u;}}}
   else{i=(uint16_t)(rnd(&x)%16u);s->words[0]=(agc_word)(i==0u?061u:i==1u?063u:0100u+rnd(&x)%01300u);
    if(i>=2u)s->words[s->words[0]]=(agc_word)(rnd(&x)%(i==2u?077777u:14460u));
    if(i==3u)s->words[0]|=0100000u;}
  }
  if(!m&&z==05511u){
   s->words[061]=(agc_word)(rnd(&x)%4u?rnd(&x)%0100u:rnd(&x)%02000u);
   s->words[063]=(agc_word)(rnd(&x)%8u?02000u+rnd(&x)%(07776u-02000u):rnd(&x)%040000u);
   s->words[1]=(agc_word)(02000u+rnd(&x)%(07776u-02000u));
   if(rnd(&x)%4u==0u)s->words[0]=(agc_word)((rnd(&x)&1u)?0100000u|(rnd(&x)&037777u):040000u|(rnd(&x)&037777u));
  }
 }
 s->words[2]=q;s->words[3]=(agc_word)(eb<<8);s->words[4]=(agc_word)(fb<<10);s->words[6]=(agc_word)((fb<<10)|eb);s->words[5]=z;s->words[7]=0;
}
unsigned svt_run(apollo_overlay_state *s,uint16_t *superbank,uint16_t *inhint,uint16_t n){static agc_routine_table t[1];agc_native_program p={0};
 p66_bias_io lio={0};cm_native_io cio={0};void *io;enum exec_status st;uint16_t steps,count;
 svt_input(s,superbank,inhint,n);
 if(svt_mission(n)){t[0]=cm_service_routines;cio.superbank=*superbank;cio.interrupt_enabled=!*inhint;io=&cio;p.operands=cm_service_site_table(&count);}
 else{t[0]=lm_service_routines;lio.superbank=*superbank;lio.interrupt_enabled=(uint16_t)!*inhint;io=&lio;p.operands=lm_service_site_table(&count);}
 p.operand_count=count;p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;
 st=agc_transfer_run(&p,s,io,superbank,AGC_TRANSFER_NONE,SVT_MAX_STEPS,&steps);
 *inhint=(uint16_t)!(svt_mission(n)?cio.interrupt_enabled:lio.interrupt_enabled);
 return st==EXEC_UNMAPPED?1u:0u;
}
uint32_t svt_digest(apollo_overlay_state *s,uint16_t n){uint16_t superbank,inhint,i,a=0,b=0,v;unsigned k=svt_run(s,&superbank,&inhint,n);
 a=(uint16_t)(a+k);b=(uint16_t)(b+a);
 if(k){a=(uint16_t)(a+inhint);b=(uint16_t)(b+a);for(i=0;i<APOLLO_SHARED_WORDS;++i){v=(uint16_t)(s->words[i]&(i<3u?0177777u:077777u));a=(uint16_t)(a+v);b=(uint16_t)(b+a);}}
 return (uint32_t)b<<16|a;
}
