/* SPDX-License-Identifier: MIT
 * Explicit compiled original operations, no instruction decoder. */
#include "gimbal_complete.h"
#include "delcomp.h"
#include "guidance_product.h"
#include "guidance_dp_divide.h"
#include "guidance_shift.h"
#include "guidance_dot.h"
#if defined(__GNUC__)
#define GIMBAL_CALL __attribute__((noinline))
#else
#define GIMBAL_CALL
#endif
typedef struct {p66_preparation p;p66_scale_scratch sc;p66_elapsed_state el;
 p66_divide_scratch d;uint16_t third,vbuf[6],buf2[2];} scratch;
static agc_dp pair(const uint16_t *w,uint16_t at){agc_dp p={w[at],w[at+1u]};return p;}
static GIMBAL_CALL void read_state(scratch *s,const uint16_t *w){uint16_t i;
 for(i=0;i<8u;++i){s->p.mpac[i]=w[0154u+i];}s->p.rate.registers.a=w[0];s->p.rate.registers.l=w[1];
 s->sc.mptemp=w[0135];s->sc.ovfind=w[0121];s->el.buf=w[0130];s->d.buf_low=w[0131];s->third=w[0132];
 s->d.dvsign=w[0136];s->d.dvnormct=w[0137];s->d.maxdvsw=w[0140];
 for(i=0;i<6u;++i){s->vbuf[i]=w[0122u+i];}s->buf2[0]=w[0133];s->buf2[1]=w[0134];}
static GIMBAL_CALL void write_state(const scratch *s,uint16_t *w){uint16_t i;
 for(i=0;i<8u;++i){w[0154u+i]=s->p.mpac[i];}w[0]=s->p.rate.registers.a;w[1]=s->p.rate.registers.l;
 w[0135]=s->sc.mptemp;w[0121]=s->sc.ovfind;w[0130]=s->el.buf;w[0131]=s->d.buf_low;w[0132]=s->third;
 w[0136]=s->d.dvsign;w[0137]=s->d.dvnormct;w[0140]=s->d.maxdvsw;
 for(i=0;i<6u;++i){w[0122u+i]=s->vbuf[i];}w[0133]=s->buf2[0];w[0134]=s->buf2[1];}
static GIMBAL_CALL void load(scratch *s,agc_dp v){s->p.rate.registers.a=0;s->p.rate.registers.l=agc_extend(s->p.mpac[1]);s->p.mpac[0]=v.high;s->p.mpac[1]=v.low;s->p.mpac[2]=0;s->p.mpac[7]=0;}
static GIMBAL_CALL void vector_load(scratch *s,const uint16_t *w,uint16_t at){uint16_t i;static const uint8_t map[6]={0,1,3,4,5,6};
 for(i=0;i<6u;++i){s->p.mpac[map[i]]=w[at+i];}s->p.mpac[7]=077776;s->p.rate.registers.a=0;}
static GIMBAL_CALL void scalar_store(scratch *s,uint16_t *w,uint16_t at){s->p.rate.registers.l=agc_extend(w[at+1u]);s->p.rate.registers.a=0;w[at]=s->p.mpac[0];w[at+1u]=s->p.mpac[1];}
static GIMBAL_CALL void vector_store(scratch *s,uint16_t *w,uint16_t at){uint16_t i;static const uint8_t map[6]={0,1,3,4,5,6};
 s->p.rate.registers.l=agc_extend(w[at+5u]);s->p.rate.registers.a=0;for(i=0;i<6u;++i){w[at+i]=s->p.mpac[map[i]];}}
static GIMBAL_CALL void round_product(scratch *s){agc_register a;int overflow;
 a=agc_extend(s->p.mpac[2]);s->p.mpac[2]=agc_correct(s->p.rate.registers.a);
 a=agc_add_register(a,a);s->p.rate.registers.l=agc_extend(agc_correct(a));overflow=agc_overflow(a);
 if(overflow){a=agc_add_register(overflow>0?1:agc_extend(077776),agc_extend(s->p.mpac[1]));s->p.mpac[1]=agc_correct(a);overflow=agc_overflow(a);
 if(overflow){a=agc_add_register(overflow>0?1:agc_extend(077776),agc_extend(s->p.mpac[0]));s->p.mpac[0]=agc_correct(a);overflow=agc_overflow(a);
 if(overflow){a=overflow>0?1:agc_extend(077776);s->sc.ovfind=agc_correct(a);}}}
 s->p.rate.registers.a=agc_extend(agc_correct(a));}
static GIMBAL_CALL void scalar_vector(scratch *s,const uint16_t *w,uint16_t at){agc_dp scalar={s->p.mpac[0],s->p.mpac[1]},f;uint16_t result[6],third=0,i;static const uint8_t map[6]={0,1,3,4,5,6};
 for(i=0;i<3u;++i){s->p.mpac[0]=scalar.high;s->p.mpac[1]=scalar.low;s->p.mpac[7]=0;f=pair(w,(uint16_t)(at+2u*i));
 (void)p66_scalar_product(&s->p,&s->sc,&f);round_product(s);result[2u*i]=s->p.mpac[0];result[2u*i+1u]=s->p.mpac[1];third=s->p.mpac[2];}
 for(i=0;i<6u;++i){s->p.mpac[map[i]]=result[i];}s->p.mpac[2]=third;s->p.mpac[7]=077776;s->p.rate.registers.a=0;}
static GIMBAL_CALL void dca(scratch *s,const uint16_t *p){s->p.rate.registers.a=agc_extend(p[0]);s->p.rate.registers.l=agc_extend(p[1]);}
static GIMBAL_CALL void dxch(scratch *s,uint16_t *p){agc_register a=s->p.rate.registers.a,l=s->p.rate.registers.l;dca(s,p);p[0]=agc_correct(a);p[1]=agc_correct(l);}
static GIMBAL_CALL void matrix_vector(scratch *s,const uint16_t matrix[18]){uint16_t i;static const uint8_t map[6]={0,1,3,4,5,6};
 for(i=0;i<6u;++i){s->vbuf[i]=s->p.mpac[map[i]];}s->d.maxdvsw=2;
 (void)p66_dot(&s->p,&s->sc,&s->el,&s->d,&s->third,matrix);s->p.mpac[7]=077776;
 dca(s,s->vbuf);dxch(s,s->p.mpac);dxch(s,s->buf2);dca(s,s->vbuf+2);dxch(s,s->p.mpac+3);dca(s,s->vbuf+4);dxch(s,s->p.mpac+5);
 (void)p66_dot(&s->p,&s->sc,&s->el,&s->d,&s->third,matrix+6);s->p.mpac[7]=077776;
 /* DOTSUB returns its major accumulation in A. The public DOT boundary
  * instead clears A at DANZIG. This fixed bounded matrix cannot overflow. */
 s->p.rate.registers.a=agc_extend(s->p.mpac[0]);
 dxch(s,s->vbuf);dxch(s,s->p.mpac);dxch(s,s->vbuf+2);dxch(s,s->p.mpac+3);dxch(s,s->vbuf+4);dxch(s,s->p.mpac+5);
 (void)p66_dot(&s->p,&s->sc,&s->el,&s->d,&s->third,matrix+12);s->p.mpac[7]=077776;s->d.dvnormct=07342;
 s->p.rate.registers.a=agc_extend(s->p.mpac[0]);
 dxch(s,s->buf2);dxch(s,s->p.mpac);dxch(s,s->p.mpac+5);dxch(s,s->vbuf+2);dxch(s,s->p.mpac+3);s->p.rate.registers.a=0;}
GIMBAL_CALL uint16_t gimbal_complete(apollo_overlay_state *image,gimbal_io *io,enum apollo_fixed_mission mission){scratch s={0};uint16_t *w,rate,table,br,am,tm,bias,i,matrix[18];agc_dp selected,time,scale;
 if(!gimbal_entry(image,io,mission))return 0;
 (void)delcomp_native(image,io,mission);w=image->words;read_state(&s,w);
 rate=w[mission==APOLLO_FIXED_LM?01325:01130];table=mission==APOLLO_FIXED_LM?18937:18936;
 (void)gimbal_fixed_read(mission,(uint16_t)(table+rate),&selected.high);(void)gimbal_fixed_read(mission,(uint16_t)(table+rate+1u),&selected.low);
 (void)gimbal_fixed_read(mission,(uint16_t)(table+8u),&time.high);(void)gimbal_fixed_read(mission,(uint16_t)(table+9u),&time.low);
 load(&s,selected);scalar_vector(&s,w,mission==APOLLO_FIXED_LM?03270:03342);
 if(mission==APOLLO_FIXED_CM){for(i=0;i<18u;++i)(void)gimbal_fixed_read(mission,(uint16_t)(table+10u+i),&matrix[i]);matrix_vector(&s,matrix);}
 br=mission==APOLLO_FIXED_LM?03325:03310;am=mission==APOLLO_FIXED_LM?03335:03364;tm=mission==APOLLO_FIXED_LM?03333:03316;
 vector_store(&s,w,br);load(&s,pair(w,am));(void)p66_scalar_product(&s.p,&s.sc,&time);
 (void)p66_dp_divide(&s.p,&s.sc,&s.el,&s.d,selected,0);(void)p66_scalar_shift_right(&s.p,&s.sc,5,0);scalar_store(&s,w,tm);
 if(mission==APOLLO_FIXED_CM){vector_load(&s,w,br);(void)gimbal_fixed_read(mission,(uint16_t)(table+28u),&scale.high);(void)gimbal_fixed_read(mission,(uint16_t)(table+29u),&scale.low);
 (void)p66_vector_multiply(&s.p,&s.sc,&scale);bias=03274;vector_store(&s,w,bias);}
 write_state(&s,w);w[2]=agc_extend(w[076]);w[076]=(uint16_t)(w[076]|2u);w[0130]=2;w[0131]=2;io->interrupt_enabled=1;
 w[0116]=mission==APOLLO_FIXED_LM?01733:01674;
 w[0164]=mission==APOLLO_FIXED_LM?03010:03032;w[0117]=(uint16_t)(044000u+w[0164]-02000u);w[020]=020016;w[0]=agc_extend(040000);w[1]=010000;w[021]=0;
 return 1;}
