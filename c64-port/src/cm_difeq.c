/* SPDX-License-Identifier: MIT */
#include "cm_difeq.h"
#include "guidance_product.h"
#include "guidance_shift.h"
#include "guidance_sum.h"
#include "guidance_vector.h"
typedef struct {p66_preparation p;p66_scale_scratch sc;p66_elapsed_state el;} arithmetic;
static uint8_t legal_base(uint16_t b){return (uint8_t)(b==0401||b==0455||b==0531||b==0605||b==0661);}
static void gather(const apollo_overlay_state *m,arithmetic *x){unsigned i;for(i=0;i<8u;++i)x->p.mpac[i]=m->words[0154+i];x->p.rate.registers.a=m->words[0];x->p.rate.registers.l=m->words[1];x->sc.mptemp=m->words[0135];x->sc.ovfind=m->words[0121];x->el.buf=m->words[0130];x->el.piptime.high=x->el.piptime.low=0;}
static void commit(apollo_overlay_state *m,const arithmetic *x){unsigned i;for(i=0;i<8u;++i)m->words[0154+i]=x->p.mpac[i];m->words[0]=x->p.rate.registers.a;m->words[1]=x->p.rate.registers.l;m->words[0135]=x->sc.mptemp;m->words[0121]=x->sc.ovfind;m->words[0130]=x->el.buf;}
static void load(apollo_overlay_state *m,uint16_t address,uint8_t vector){static const uint8_t at[6]={0,1,3,4,5,6};unsigned i;uint16_t old=m->words[vector?0162:0155];for(i=0;i<(vector?6u:2u);++i)m->words[0154+at[i]]=m->words[address+i];if(!vector)m->words[0156]=0;m->words[0163]=vector?077776:0;m->words[0]=vector?0177776:0;m->words[1]=agc_extend(old);}
static void store(apollo_overlay_state *m,uint16_t address){static const uint8_t at[6]={0,1,3,4,5,6};unsigned i;uint8_t count=m->words[0163]==077776?6u:2u;uint16_t hi=m->words[address+count-2u],lo=m->words[address+count-1u];for(i=0;i<count;++i)m->words[address+i]=m->words[0154+at[i]];m->words[0]=count==2u?0:agc_extend(hi);m->words[1]=agc_extend(lo);}
static void push(apollo_overlay_state *m){static const uint8_t at[6]={0,1,3,4,5,6};uint16_t ptr=m->words[0166];unsigned i,count=m->words[0163]==077776?6u:2u;for(i=0;i<count;++i)m->words[ptr+i]=m->words[0154+at[i]];m->words[0166]=(uint16_t)(ptr+count);}
static void shift(apollo_overlay_state *m,uint8_t count,uint8_t left){arithmetic x;gather(m,&x);if(left)(void)p66_vector_shift_left(&x.p,&x.sc,&x.el,count,1);else(void)p66_vector_shift_right(&x.p,&x.sc,count,1);commit(m,&x);}
static void add(apollo_overlay_state *m,uint16_t address,uint8_t vector){arithmetic x;agc_word operand[6];agc_dp pair;unsigned i;gather(m,&x);if(vector){for(i=0;i<6u;++i)operand[i]=m->words[address+i];(void)p66_vector_add(&x.p,&x.sc,&x.el,operand);}else{pair.high=m->words[address];pair.low=m->words[address+1u];(void)p66_scalar_add(&x.p,&x.sc,&x.el,&pair);}commit(m,&x);}
/* Source VROUND's TS skip chain, retaining raw A until a later DXCH. */
static void round_product(arithmetic *x,uint8_t scalar){agc_register a=agc_extend(x->p.mpac[2]);int ov;x->p.mpac[2]=scalar?0:agc_correct(x->p.rate.registers.a);a=agc_add_register(a,a);x->p.rate.registers.l=agc_extend(agc_correct(a));ov=agc_overflow(a);if(ov){a=agc_add_register(ov>0?1:agc_extend(077776),agc_extend(x->p.mpac[1]));x->p.mpac[1]=agc_correct(a);ov=agc_overflow(a);if(ov){a=agc_add_register(ov>0?1:agc_extend(077776),agc_extend(x->p.mpac[0]));x->p.mpac[0]=agc_correct(a);ov=agc_overflow(a);if(ov){a=ov>0?1:agc_extend(077776);x->sc.ovfind=agc_correct(a);}}}x->p.rate.registers.a=a;}
static void exchange(arithmetic *x,uint8_t at){agc_register a=agc_extend(x->p.mpac[at]),l=agc_extend(x->p.mpac[at+1u]);x->p.mpac[at]=agc_correct(x->p.rate.registers.a);x->p.mpac[at+1u]=agc_correct(x->p.rate.registers.l);x->p.rate.registers.a=a;x->p.rate.registers.l=l;}
/* DVXSC has different operand order and scratch from VVXSC. Preserve the
 * original scalar/component exchanges, not merely the commutative value. */
static void scalar_vector(apollo_overlay_state *m,uint16_t address){arithmetic x;agc_dp f;unsigned i;gather(m,&x);x.p.rate.registers.a=agc_extend(x.p.mpac[0]);x.p.rate.registers.l=agc_extend(x.p.mpac[1]);exchange(&x,3);
 for(i=0;i<3u;++i){f.high=m->words[address+2u*i];f.low=m->words[address+2u*i+1u];(void)p66_scalar_product(&x.p,&x.sc,&f);round_product(&x,0);if(i==0u){x.p.rate.registers.a=agc_extend(x.p.mpac[3]);x.p.rate.registers.l=agc_extend(x.p.mpac[4]);exchange(&x,0);exchange(&x,5);}else if(i==1u){exchange(&x,3);exchange(&x,0);exchange(&x,3);}}
 exchange(&x,0);exchange(&x,5);exchange(&x,0);x.p.mpac[7]=077776;x.p.rate.registers.a=0177776;commit(m,&x);}
static void vector_scalar(apollo_overlay_state *m,uint16_t address){arithmetic x;agc_dp f;gather(m,&x);f.high=m->words[address];f.low=m->words[address+1u];(void)p66_vector_multiply(&x.p,&x.sc,&f);commit(m,&x);}
static void dmpr(apollo_overlay_state *m){static const agc_dp two_thirds={025252,025253};arithmetic x;load(m,02211,0);gather(m,&x);(void)p66_scalar_product(&x.p,&x.sc,&two_thirds);round_product(&x,1);commit(m,&x);}
uint8_t cm_difeq_phase_count(uint8_t stage){return stage==1u?12u:stage<3u?9u:0u;}
enum cm_difeq_status cm_difeq_phase(apollo_overlay_state *m,uint8_t stage,uint8_t phase){
 static const uint16_t commonloc[7]={03630,03633,03634,03636,03641,03643,03644};
 static const uint16_t commoncyr[7]={020034,030002,030002,030000,020003,020024,020024};
 static const uint16_t commonaddr[7]={01611,01611,01611,01605,01611,031511,01551};
 static const uint16_t commonfb[7]={022000,02000,022000,0,022000,022000,022000};
 static const uint16_t commoneb[7]={02000,01400,02000,0,02000,01000,02000};
 static const uint16_t final_loc[9]={03465,03467,03471,03474,03476,03500,03501,03503,03504};
 static const uint16_t final_cyr[9]={020020,020003,020024,020024,020000,020024,010000,020024,020024};
 static const uint16_t final_addr[9]={03773,01571,031517,031511,01601,031573,0,031517,01517};
 static const uint16_t final_fb[9]={022000,022000,022000,022000,022000,022000,0,022000,022000};
 static const uint16_t final_eb[9]={0,02000,01000,01000,02000,02000,0,01000,01000};
 uint16_t base=m->words[0120],loc=0,cyr=0,addr=0,fb=0,eb=0;uint8_t common=(uint8_t)(stage==1u?5u:2u),index;
 if(stage>=3u||phase>=cm_difeq_phase_count(stage)||!legal_base(base)||(m->words[0165]&076000u)!=022000u||m->words[023]!=0)return CM_DIFEQ_OUT_OF_SCOPE;
 if(phase&&m->words[0163]!=(stage==2u&&phase==1u?0u:stage<2u&&phase>common&&phase<=common+3u?0u:077776u))return CM_DIFEQ_OUT_OF_SCOPE;
 if(m->words[0166]!=(uint16_t)(base+(stage==1u&&phase==2u?6u:stage==2u&&phase>=2u&&phase<=6u?2u:0u)))return CM_DIFEQ_OUT_OF_SCOPE;
 if(stage<2u&&phase<common){
  if(phase==0u){load(m,02201,1);shift(m,stage==0u?3u:1u,0);loc=stage==0u?03447:03453;cyr=stage==0u?030001:030000;addr=01601;}
  else if(stage==1u&&phase==1u){push(m);add(m,02165,1);loc=03455;cyr=020024;addr=031565;fb=022000;eb=02000;}
  else if(stage==1u&&phase==2u){store(m,02173);m->words[0166]=(uint16_t)(m->words[0166]-6u);load(m,m->words[0166],1);loc=03456;cyr=020000;addr=m->words[0166];fb=022000;eb=02000;}
  else if(stage==1u&&phase==3u){shift(m,1,0);add(m,02165,1);loc=03460;cyr=020024;addr=031565;fb=022000;eb=02000;}
  else{store(m,02165);m->words[base+052]=stage==0u?023452:023463;m->words[0117]=023626;m->words[0115]=0;m->words[0]=0;loc=03626;cyr=040006;addr=01565;fb=022000;eb=(uint16_t)((m->words[0165]&7u)<<8);}
 }else if(stage<2u){index=(uint8_t)(phase-common);loc=commonloc[index];cyr=commoncyr[index];addr=commonaddr[index];fb=commonfb[index];eb=commoneb[index];switch(index){case 0:load(m,02313,0);add(m,02211,0);break;case 1:m->words[0130]=base;m->words[0117]=01500;m->words[base+046]=agc_correct(agc_add_register(agc_extend(m->words[base+046]),agc_extend(077763)));m->words[01500]=m->words[base+046];m->words[0]=agc_extend(m->words[01500]);break;case 2:store(m,02211);break;case 3:scalar_vector(m,02201);shift(m,1,0);break;case 4:add(m,01117,1);vector_scalar(m,02211);break;case 5:add(m,01111,1);break;default:store(m,02151);break;}}
 else{loc=final_loc[phase];cyr=final_cyr[phase];addr=final_addr[phase];fb=final_fb[phase];eb=final_eb[phase];switch(phase){case 0:dmpr(m);eb=(uint16_t)((m->words[0165]&7u)<<8);break;case 1:push(m);scalar_vector(m,02165);break;case 2:shift(m,1,1);add(m,01117,1);break;case 3:vector_scalar(m,02211);add(m,01111,1);break;case 4:store(m,01111);load(m,02201,1);break;case 5:shift(m,3,0);add(m,02173,1);break;case 6:m->words[0166]=(uint16_t)(m->words[0166]-2u);addr=m->words[0166];vector_scalar(m,addr);shift(m,1,1);break;case 7:add(m,01117,1);break;default:store(m,01117);break;}}
 if(!fb)eb=(uint16_t)((m->words[0165]&7u)<<8);
 m->words[3]=eb;m->words[4]=fb;m->words[6]=(uint16_t)(fb|(eb>>8));m->words[2]=3;m->words[5]=(stage<2u&&phase==common-1u)?06037:06030;m->words[0164]=loc;m->words[020]=cyr;m->words[0116]=addr;m->words[023]=0;return CM_DIFEQ_OK;
}
