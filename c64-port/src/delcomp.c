/* SPDX-License-Identifier: MIT
 * Direct compiled DELCOMP source arithmetic; no AGC opcode fetching/decoder. */
#include "delcomp.h"
#include "guidance_trig.h"
#include "guidance_product.h"
#include "guidance_sum.h"
#include "guidance_shift.h"
typedef struct {apollo_overlay_state *image;p66_preparation p;p66_scale_scratch sc;
 p66_elapsed_state el;p66_divide_scratch d;p66_norm_scratch n;p66_trig_scratch t;
 enum apollo_fixed_mission mission;} working;
static agc_dp pair(const working *s,uint16_t address){agc_dp p={s->image->words[address],s->image->words[address+1u]};return p;}
static void load(working *s,agc_dp p){s->p.rate.registers.l=agc_extend(s->p.mpac[1]);s->p.rate.registers.a=0;s->p.mpac[0]=p.high;s->p.mpac[1]=p.low;s->p.mpac[2]=0;s->p.mpac[7]=0;}
static void push(working *s){uint16_t *w=s->image->words,k=w[0166];s->p.rate.registers.l=agc_extend(w[k+1u]);s->p.rate.registers.a=0;w[k]=s->p.mpac[0];w[k+1u]=s->p.mpac[1];w[0166]=(uint16_t)(k+2u);}
static agc_dp pop(working *s){uint16_t *w=s->image->words;w[0166]=(uint16_t)(w[0166]-2u);return pair(s,w[0166]);}
static void pushload(working *s,agc_dp p){push(s);s->p.mpac[0]=p.high;s->p.mpac[1]=p.low;s->p.mpac[2]=0;s->p.mpac[7]=0;}
static void store(working *s,uint16_t at){uint16_t *w=s->image->words;s->p.rate.registers.l=agc_extend(w[at+1u]);s->p.rate.registers.a=0;w[at]=s->p.mpac[0];w[at+1u]=s->p.mpac[1];}
static void doubled(agc_register_pair *r){agc_register hi=agc_add_register(r->a,r->a),lo=agc_add_register(r->l,r->l);int carry=agc_overflow(lo);if(carry)hi=agc_add_register(hi,carry>0?1:0177776);r->a=hi;r->l=agc_extend(agc_correct(lo));}
static void square(working *s){agc_word hi=s->p.mpac[0],lo=s->p.mpac[1];agc_register_pair r;agc_dp p;
 r=agc_multiply(agc_extend(lo),lo);s->p.mpac[2]=agc_correct(r.a);s->p.mpac[1]=0;
 r=agc_multiply(agc_extend(lo),hi);doubled(&r);p.high=0;p.low=s->p.mpac[2];agc_double_add(&r,&p);
 s->p.mpac[1]=p.high;s->p.mpac[2]=p.low;s->p.mpac[0]=agc_correct(r.a);
 r=agc_multiply(agc_extend(hi),hi);p.high=s->p.mpac[0];p.low=s->p.mpac[1];agc_double_add(&r,&p);
 s->p.mpac[0]=p.high;s->p.mpac[1]=p.low;s->p.rate.registers=r;
}
static void product(working *s,agc_dp operand){(void)p66_scalar_product(&s->p,&s->sc,&operand);}
static void add(working *s,agc_dp operand){(void)p66_scalar_add(&s->p,&s->sc,&s->el,&operand);}
static void reverse_subtract(working *s,agc_dp operand){(void)p66_scalar_reverse_subtract(&s->p,&s->sc,&s->el,&operand);}
static void left(working *s,uint8_t count){(void)p66_scalar_shift_left(&s->p,&s->sc,count,1);s->image->words[021]=(uint16_t)(count-1u);}
static void overflow_branch(working *s){agc_word flag=s->sc.ovfind;if(flag==0)return;
 /* Original CCS(-0 OVFIND) skips its TS and still reaches SIGNMPAC. */
 if(flag!=077777u)s->sc.ovfind=agc_diminished_abs(flag);
 /* RTB enters SIGNMPAC through SWCALL-1; its bank-return pair is visible
  * in BUF2 even though the compiled arithmetic returns through C. */
 s->t.buf2.high=s->mission==APOLLO_FIXED_LM?02453:02500;
 s->t.buf2.low=02000;
 if(s->p.mpac[0]&040000u)s->p.mpac[0]=s->p.mpac[1]=040000;
 else s->p.mpac[0]=s->p.mpac[1]=037777;
 s->p.mpac[2]=0;s->p.mpac[7]=0;
}
static void off_diagonal(working *s,uint16_t cof,uint16_t matrix,uint16_t base,
 uint8_t i,uint8_t j,uint8_t k,uint8_t positive,uint8_t negative){
 load(s,pair(s,(uint16_t)(cof+2u*i)));product(s,pair(s,(uint16_t)(cof+2u*j)));
 product(s,pair(s,(uint16_t)(base+4u)));left(s,1);
 pushload(s,pair(s,(uint16_t)(cof+2u*k)));product(s,pair(s,base));push(s);
 add(s,pair(s,(uint16_t)(base+6u)));left(s,2);overflow_branch(s);store(s,(uint16_t)(matrix+positive));
 load(s,pop(s));reverse_subtract(s,pop(s));left(s,2);overflow_branch(s);store(s,(uint16_t)(matrix+negative));
}
uint16_t delcomp_native(apollo_overlay_state *image,gimbal_io *io,enum apollo_fixed_mission mission){
 working s={0};uint16_t *w,base,cof,matrix,entry,ret,i;agc_dp half={020000,0},operand;
 if(image==0||io==0||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return 0;
 w=image->words;base=w[0120];entry=mission==APOLLO_FIXED_LM?02527:02530;
 if(w[3]>03400u||(w[3]&0377u)!=0||w[4]!=044000||w[6]!=(uint16_t)(044000u|(w[3]>>8))||w[0165]!=w[6]||
 w[5]!=(mission==APOLLO_FIXED_LM?06070u:06037u)||w[0164]!=entry||w[0115]!=040000||w[023]!=0||w[067]!=0||w[7]!=0||w[0163]!=0||
 base<0401u||base>0661u||(uint16_t)(base-0401u)%054u!=0||io->interrupt_enabled>1u||(io->superbank&~0160u)!=0)return 0;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return 0;
 ret=w[base+052u];if(ret<040000u)return 0;
 s.image=image;s.mission=mission;for(i=0;i<8u;++i)s.p.mpac[i]=w[0154u+i];s.p.rate.registers.a=w[0];s.p.rate.registers.l=w[1];
 s.sc.mptemp=w[0135];s.sc.ovfind=w[0121];s.el.buf=w[0130];s.d.maxdvsw=w[0140];
 s.n.addrwd=w[0116];s.n.tem1=w[0141];for(i=0;i<6u;++i)s.n.vbuf[i]=w[0122u+i];s.t.buf2.high=w[0133];s.t.buf2.low=w[0134];s.t.polish=w[0117];
 cof=mission==APOLLO_FIXED_LM?03270:03342;matrix=mission==APOLLO_FIXED_LM?02230:03425;
 w[0166]=base;push(&s);
 (void)p66_sine(&s.p,&s.sc,&s.d,&s.n,&s.t);operand=pop(&s);pushload(&s,operand);
 (void)p66_cosine(&s.p,&s.sc,&s.d,&s.n,&s.t);push(&s);
 (void)p66_scalar_shift_right(&s.p,&s.sc,2,1);w[021]=1;operand=pop(&s);pushload(&s,operand);
 reverse_subtract(&s,half);overflow_branch(&s);pushload(&s,pair(&s,cof));
 for(i=0;i<3u;++i){
  if(i!=0)load(&s,pair(&s,(uint16_t)(cof+2u*i)));
  square(&s);product(&s,pair(&s,(uint16_t)(base+4u)));add(&s,pair(&s,(uint16_t)(base+2u)));left(&s,3);overflow_branch(&s);
  store(&s,(uint16_t)(matrix+8u*i));
 }
 off_diagonal(&s,cof,matrix,base,0,1,2,6,2);
 off_diagonal(&s,cof,matrix,base,0,2,1,4,12);
 off_diagonal(&s,cof,matrix,base,1,2,0,14,10);
 for(i=0;i<8u;++i){w[0154u+i]=s.p.mpac[i];}
 w[0]=agc_extend((uint16_t)(ret&040000u));w[1]=s.p.rate.registers.l;w[2]=3;
 /* Final unary RVQ decode and the preceding short SL2 editing write. */
 w[020]=7;w[021]=0;
 w[0135]=s.sc.mptemp;w[0121]=s.sc.ovfind;w[0130]=s.el.buf;w[0140]=s.d.maxdvsw;
 w[0141]=mission==APOLLO_FIXED_LM?s.n.tem1:03572;for(i=0;i<6u;++i)w[0122u+i]=s.n.vbuf[i];w[0133]=s.t.buf2.high;w[0134]=s.t.buf2.low;
 w[0116]=(uint16_t)(01400u+((matrix+10u)&0377u));w[0117]=ret;
 w[4]=(uint16_t)(ret&076000u);w[6]=(uint16_t)(w[4]|(w[0165]&7u));w[3]=(uint16_t)((w[6]&7u)<<8);w[0165]=w[6];w[0115]=(uint16_t)(w[4]&040000u);
 w[0164]=(uint16_t)(02000u+(ret&01777u));
 return 1;
}
