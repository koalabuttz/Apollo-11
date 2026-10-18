/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "dsky_input_scale.h"
#include "guidance_product.h"
static int dsky_scalein_entry(uint16_t pc,uint8_t cm){static const uint16_t lm[]={03076,03106,03111,03114,03116,03141,03146,03125},c[]={02766,02776,03001,03021,03023,03046,03053,03032};uint8_t i;for(i=0;i<8u;++i)if(pc==(cm ? c[i] : lm[i]))return i;return -1;}
static uint16_t dsky_scalein_address(uint16_t logical,uint16_t eb){return logical<01400u ? logical : (uint16_t)(eb+(logical&0377u));}
static int dsky_scalein_domain(const apollo_overlay_state *s,const dsky_lamp_io *io,uint8_t cm){const uint16_t *w;uint16_t target;int e;if(!s||!io)return 0;w=s->words;e=dsky_scalein_entry(w[5],cm);if(e<0||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=060000u||w[6]!=(uint16_t)(060000u|(w[3]>>8))||io->control.superbank>0160u||(io->control.superbank&017u)||!(io->control.superbank&0100u)||io->control.interrupt_enabled>1u||io->restart_light>1u)return 0;
 if(e>=3){if(w[0140]!=1u&&w[0140]!=2u)return 0;if(w[0140]==2u&&w[0117]>2u)return 0;if(w[0145]>01774u)return 0;target=(uint16_t)(w[0145]+(w[0140]==2u ? w[0117] : 0u));target=dsky_scalein_address(target,w[3]);if(target<010u||(target>=017u&&target<=023u)||target>03775u||(target&0377u)>0375u)return 0;}return 1;
}
static void dsky_scalein_product(uint16_t *w){p66_preparation p;p66_scale_scratch sc;agc_dp f={w[0123],w[0124]};uint8_t i;memset(&p,0,sizeof p);for(i=0;i<3u;++i)p.mpac[i]=w[0154u+i];sc.mptemp=w[0135];sc.ovfind=w[0121];(void)p66_scalar_product(&p,&sc,&f);for(i=0;i<3u;++i)w[0154u+i]=p.mpac[i];w[0135]=sc.mptemp;w[0116]=0123;w[0]=p.rate.registers.a;w[1]=p.rate.registers.l;}
static enum dsky_input_arithmetic_route dsky_scalein_arithmetic(apollo_overlay_state *s,dsky_lamp_io *io,uint8_t cm,uint16_t pc,uint16_t q){enum dsky_input_arithmetic_route r;s->words[5]=pc;s->words[2]=q;if(cm){cm_native_io c={0};c.superbank=io->control.superbank;c.interrupt_enabled=io->control.interrupt_enabled;(void)cm_input_arithmetic(s,&c,&io->wrote_channels,&io->restart_light,&r);}else (void)dsky_input_arithmetic(s,io,&r);return r;}
static void dsky_scalein_body(apollo_overlay_state *s,dsky_lamp_io *io,uint8_t cm,enum dsky_input_scale_route *route){uint16_t *w=s->words,bin=cm ? 02776u : 03106u,dpstart=cm ? 03021u : 03114u,value,logical,target;int e=dsky_scalein_entry(w[5],cm),overflow;enum dsky_input_arithmetic_route r;w[1]=agc_extend(agc_correct(w[1]));
 if(e==0||e==2||e==3||e==5||e==6)dsky_scalein_product(w);
 if(e==0){value=w[0156];w[0156]=agc_correct(w[0]);w[0]=agc_extend(value);value=w[0155];w[0155]=agc_correct(w[0]);w[0]=agc_extend(value);value=w[0154];w[0154]=agc_correct(w[0]);w[0]=agc_extend(value);if(w[0]!=0u&&w[0]!=0177777u){w[5]=cm ? 04161u : 04145u;w[2]=bin;*route=DSKY_INPUT_SCALE_RECYCLE;return;}}
 if(e==5||e==6){w[0]=e==5 ? 6u : 2u;(void)dsky_scalein_arithmetic(s,io,cm,cm ? 03060u : 03153u,(uint16_t)((e==5 ? (cm ? 03046u : 03141u) : (cm ? 03053u : 03146u))+4u));}
 if(e<=2){(void)dsky_scalein_arithmetic(s,io,cm,cm ? 03070u : 03163u,(uint16_t)(bin+1u));r=dsky_scalein_arithmetic(s,io,cm,cm ? 03101u : 03174u,(uint16_t)(bin+2u));}
 else if(e!=7){value=w[0156];w[0156]=agc_correct(w[0]);w[0]=agc_add_register(agc_extend(value),agc_extend(value));w[0156]=agc_correct(w[0]);overflow=agc_overflow(w[0]);w[0]=agc_add_register(overflow ? (overflow>0 ? 1u : 0177776u) : 0u,agc_extend(w[0155]));(void)dsky_scalein_arithmetic(s,io,cm,cm ? 03072u : 03165u,(uint16_t)(dpstart+8u));r=dsky_scalein_arithmetic(s,io,cm,cm ? 03101u : 03174u,(uint16_t)(dpstart+9u));}
 else r=DSKY_INPUT_ARITHMETIC_CALLER;
 if(r!=DSKY_INPUT_ARITHMETIC_CALLER){*route=r==DSKY_INPUT_ARITHMETIC_RECYCLE ? DSKY_INPUT_SCALE_RECYCLE : DSKY_INPUT_SCALE_FALLTHROUGH;return;}
 if(e>=3){logical=(uint16_t)(w[0145]+(w[0140]==2u ? w[0117] : 0u));value=w[0155];w[0155]=logical;w[0]=agc_extend(value);target=dsky_scalein_address((uint16_t)(logical+1u),w[3]);w[target]=value;}
 w[4]=062000;w[6]=(uint16_t)(062000u|(w[3]>>8));w[2]=01155;w[5]=03155;*route=DSKY_INPUT_SCALE_COMPONENT;
}
enum p66_bias_status dsky_input_scale(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_input_scale_route *route){if(!route||!dsky_scalein_domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;dsky_scalein_body(s,io,0,route);return P66_BIAS_OK;}
enum exec_status cm_input_scale(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_input_scale_route *route){dsky_lamp_io p;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.restart_light=*restart;if(!dsky_scalein_domain(s,&p,1))return EXEC_OUT_OF_SCOPE;dsky_scalein_body(s,&p,1,route);return EXEC_OK;}
