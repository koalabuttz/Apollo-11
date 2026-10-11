/* SPDX-License-Identifier: MIT */
#include "cm_quality.h"
#include "interpreted_square.h"
#include "guidance_product.h"
#include "guidance_sum.h"
#include "guidance_shift.h"
static void bank(agc_word *w,agc_word fb,agc_word eb){w[3]=eb;w[4]=fb;w[6]=(agc_word)(fb|(eb>>8));}
static void dca(p66_preparation *p,const agc_word *v){p->rate.registers.a=agc_extend(v[0]);p->rate.registers.l=agc_extend(v[1]);}
static void dxch(p66_preparation *p,agc_word *v){agc_register a=p->rate.registers.a,l=p->rate.registers.l;dca(p,v);v[0]=agc_correct(a);v[1]=agc_correct(l);}
static void round_product(p66_preparation *p,p66_scale_scratch *sc){agc_register a=agc_extend(p->mpac[2]);int ov;p->mpac[2]=agc_correct(p->rate.registers.a);a=agc_add_register(a,a);p->rate.registers.l=agc_extend(agc_correct(a));ov=agc_overflow(a);if(ov){a=agc_add_register(ov>0?1u:0177776u,agc_extend(p->mpac[1]));p->mpac[1]=agc_correct(a);ov=agc_overflow(a);if(ov){a=agc_add_register(ov>0?1u:0177776u,agc_extend(p->mpac[0]));p->mpac[0]=agc_correct(a);ov=agc_overflow(a);if(ov){a=ov>0?1u:0177776u;sc->ovfind=agc_correct(a);}}}p->rate.registers.a=agc_extend(agc_correct(a));}
static void scalar_times_vector(p66_preparation *p,p66_scale_scratch *sc,const agc_word *v){agc_dp factor;unsigned i;dca(p,p->mpac);dxch(p,p->mpac+3);for(i=0;i<3u;++i){factor.high=v[i*2u];factor.low=v[i*2u+1u];(void)p66_scalar_product(p,sc,&factor);round_product(p,sc);if(i==0u){dca(p,p->mpac+3);dxch(p,p->mpac);dxch(p,p->mpac+5);}else if(i==1u){dxch(p,p->mpac+3);dxch(p,p->mpac);dxch(p,p->mpac+3);}}dxch(p,p->mpac);dxch(p,p->mpac+5);dxch(p,p->mpac);p->mpac[7]=077776;p->rate.registers.a=0177776;}
uint8_t cm_quality_step(apollo_overlay_state *m,p66_bias_io *io,uint8_t phase)
{
#ifdef CM_QUALITY_ONLY_PHASE
 if(phase!=CM_QUALITY_ONLY_PHASE)return 2;
 phase=CM_QUALITY_ONLY_PHASE;
#endif
 static const agc_word loc[20]={03772,03773,03774,03755,03755,03756,03760,03761,03762,03763,03764,03765,03767,03770,03771,03772,03774,03774,03776,03146};
 static const agc_word edop[20]={032,0,0127,061,0,0,016,0,052,0,0162,0,0,0156,0,0,016,0,0161,0};
 static const agc_word cyr[20]={020016,020006,3,020012,3,020032,020036,020003,010001,020012,034000,020034,020006,034000,020033,020033,020010,020003,020012,7};
 static const agc_word fb[20]={02000,026000,0,024000,0,024000,026000,024000,0,024000,0,024000,024000,0,024000,024000,024000,024000,024000,022000};
 static const uint8_t vector_mode[20]={1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0};
 static const uint8_t push_offset[20]={2,2,2,2,4,4,2,2,2,2,8,8,8,8,8,8,8,8,2,8};
 p66_preparation p={0};p66_scale_scratch sc;p66_elapsed_state el={0};agc_dp operand;agc_word *w,eb,base,at;unsigned i;uint8_t terminal=0;
 if(!m||!io||phase>19u)return 2;
 w=m->words;base=w[0120];
 if(w[7]||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||io->interrupt_enabled>1u||(io->superbank&~0160u)||base<0401u||base>0661u||(base-0401u)%054u||w[base+052u]!=023146u||w[067]||w[0115])return 2;
 if((w[0165]&077770u)!=(phase<4u?026000u:024000u)||w[0163]!=(vector_mode[phase]?077776u:0u)||w[0166]!=base+push_offset[phase])return 2;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return 2;
 if(phase==0u){if(w[5]!=06037u||w[4]!=026000u||w[0164]!=03770u||w[023]||w[0163]!=077776u||w[0166]!=base+2u)return 2;}
 else if(w[5]!=06030u||w[0164]!=loc[phase-1u]||w[023]!=edop[phase-1u]||w[4]!=fb[phase-1u])return 2;
 eb=(agc_word)((w[0165]&7u)<<8);for(i=0;i<8u;++i)p.mpac[i]=w[0154u+i];p.rate.registers.a=w[0];p.rate.registers.l=w[1];sc.mptemp=w[0135];sc.ovfind=w[0121];el.buf=w[0130];
 if(phase==0u){w[0130]=0;w[0131]=04000;w[2]=w[074];w[0117]=0343;w[1]=030000;io->interrupt_enabled=1;w[0]=03777;
  if(!(w[074]&04000u)){bank(w,022000,eb);w[0165]=w[6];w[0164]=03220;w[0117]=023220;w[020]=020016;w[023]=0;w[0115]=0;w[0]=0;w[5]=06037;return 1;}
 }
 else if(phase==1u){at=(agc_word)(base+016u);p.rate.registers.l=agc_extend(p.mpac[1]);p.mpac[0]=w[at];p.mpac[1]=w[at+1u];p.mpac[2]=0;p.mpac[7]=0;p.rate.registers.a=0;w[0116]=at;}
 else if(phase==2u||phase==4u){bank(w,0,eb);w[5]=03174;(void)interpreted_square(m,INTERPRETED_SQUARE_CM,INTERPRETED_DSQ);for(i=0;i<8u;++i)p.mpac[i]=w[0154u+i];p.rate.registers.a=w[0];p.rate.registers.l=w[1];}
 else if(phase==3u||phase==9u||phase==18u){agc_word oldmode=p.mpac[7];at=(agc_word)(base+(phase==3u?020u:phase==9u?016u:022u));dca(&p,w+at);dxch(&p,p.mpac);dxch(&p,w+w[0166]);w[0166]=(agc_word)(w[0166]+(oldmode==077776u?6u:2u));p.mpac[2]=0;p.mpac[7]=0;p.rate.registers.a=0;if(oldmode==077776u){p.rate.registers.a=0;dxch(&p,p.mpac+3);dxch(&p,w+w[0166]-4u);dxch(&p,p.mpac+5);dxch(&p,w+w[0166]-2u);}w[0116]=at;if(phase==3u){w[0117]=025754;w[0165]=(agc_word)(024000u|(eb>>8));}}
 else if(phase==5u||phase==11u||phase==14u){at=(agc_word)(base+(phase==14u?4u:2u));if(phase==5u)w[0166]=(agc_word)(w[0166]-2u);operand.high=w[at];operand.low=w[at+1u];if(phase==5u)(void)p66_scalar_subtract(&p,&sc,&el,&operand);else if(phase==11u)(void)p66_scalar_add(&p,&sc,&el,&operand);else(void)p66_scalar_reverse_subtract(&p,&sc,&el,&operand);w[0116]=at;}
 else if(phase==6u){operand.high=024000;operand.low=0;(void)p66_scalar_product(&p,&sc,&operand);w[0116]=03754;}
 else if(phase==7u||phase==17u){at=(agc_word)(base+(phase==7u?016u:2u));if(phase==17u)w[0166]=(agc_word)(w[0166]-6u);scalar_times_vector(&p,&sc,w+at);w[0116]=(agc_word)(at+4u);}
 else if(phase==8u)(void)p66_vector_shift_left(&p,&sc,&el,3,1);
 else if(phase==10u||phase==13u){(void)p66_scalar_shift_right(&p,&sc,1,1);w[021]=0;}
 else if(phase==12u){dca(&p,p.mpac);dxch(&p,w+base+2u);at=(agc_word)(base+020u);dca(&p,w+at);dxch(&p,p.mpac);p.rate.registers.a=0;p.mpac[2]=0;p.mpac[7]=0;w[0116]=at;}
 else if(phase==15u){dca(&p,p.mpac);dxch(&p,w+base+4u);p.rate.registers.a=0;}
 else if(phase==16u){p.rate.registers.l=agc_extend(p.mpac[1]);p.mpac[0]=w[01776];p.mpac[1]=p.mpac[2]=p.mpac[7]=0;p.rate.registers.a=0;w[0116]=01776;}
 else{w[0117]=023146;w[0165]=(agc_word)(022000u|(eb>>8));p.rate.registers.a=0;terminal=1;}
 if(phase!=0u){for(i=0;i<8u;++i)w[0154u+i]=p.mpac[i];w[0]=p.rate.registers.a;w[1]=p.rate.registers.l;w[0135]=sc.mptemp;w[0121]=sc.ovfind;w[0130]=el.buf;}
 bank(w,fb[phase],phase==16u?01400u:eb);w[0164]=loc[phase];w[020]=cyr[phase];w[023]=edop[phase];w[5]=terminal?06037u:06030u;if(phase>=2u)w[2]=3;return terminal;
}
