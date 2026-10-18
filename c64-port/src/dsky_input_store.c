/* SPDX-License-Identifier: MIT */
#include "dsky_input_store.h"
#include "dsky_noun_check.h"
static int dsky_store_entry(uint16_t pc){return pc==03075u ? 0 : pc==03137u ? 1 : pc==03130u ? 2 : pc==03155u ? 3 : -1;}
static int dsky_store_dp(uint16_t code,uint8_t cm){return code==4u||code==5u||code==7u||code==10u||(cm&&code==13u);}
static uint16_t dsky_store_address(uint16_t logical,uint16_t eb){return logical<01400u ? logical : (uint16_t)(eb+(logical&0377u));}
static int dsky_store_domain(const apollo_overlay_state *s,const dsky_lamp_io *io,uint8_t cm){const uint16_t *w;uint16_t k,mix,address,code,dec;int e;if(!s||!io)return 0;w=s->words;e=dsky_store_entry(w[5]);if(e<0||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||io->control.superbank>0160u||(io->control.superbank&017u)||!(io->control.superbank&0100u)||io->control.interrupt_enabled>1u||io->restart_light>1u)return 0;
 if(e==0){if(w[0]>2u||w[2]<0100u||w[2]>07777u||(w[0140]!=1u&&w[0140]!=2u))return 0;}else if(w[0115]<0100u||w[0115]>07777u)return 0;
 if(e==3)return 1;
 k=e==0 ? w[0] : w[0117];if(e!=1&&k>2u)return 0;
 if(e==2){address=dsky_store_address(w[0145],w[3]);return w[0145]>=0100u&&w[0145]<=01774u&&address+k>=010u&&address+k<=03775u;}
 mix=(uint16_t)(e==0&&w[0140]==2u);dec=w[01000];if(dec==0u&&(w[0140]!=1u&&w[0140]!=2u))return 0;
 if(dec==0u&&((w[w[0140]==1u ? 0147u : 0146u]>>10)&010u))return 1;
 code=(uint16_t)(mix ? (w[0153]>>(k*5u))&037u : (w[0147]>>5)&037u);
 if(dec!=077777u&&!(dec&&!(dec&040000u))&&code>13u)return 0;
 if(dsky_store_dp(code,cm)&&!(dec&&!(dec&040000u))&&dec!=077777u){address=mix ? (uint16_t)(w[0150u+k]&03777u) : (uint16_t)(w[01017]&03777u);if(address<010u||address>03775u||(address&0377u)<(mix ? k : 0u)||(address&0377u)>0375u)return 0;}
 return 1;
}
static void dsky_store_cadr(uint16_t *w,uint16_t address,uint8_t save){if(save)w[01017]=address;w[3]=(uint16_t)(address&03400u);w[6]=(uint16_t)(062000u|(w[3]>>8));w[0]=(uint16_t)(01400u+(address&0377u));w[0145]=w[0];w[2]=3;}
static void dsky_store_body(apollo_overlay_state *s,dsky_lamp_io *io,uint8_t cm,enum dsky_input_store_route *route){uint16_t *w=s->words,k=w[0117],mix,dec,address,value,major;int e=dsky_store_entry(w[5]),dp=0;enum dsky_noun_check_route check;enum dsky_input_select_route select;w[1]=agc_extend(agc_correct(w[1]));
 if(e==0){k=w[0];w[0117]=k;w[0115]=w[2];w[2]=k;w[0162]=0;value=w[01006u+k];major=w[01003u+k];w[01006u+k]=0;w[0155]=value;w[01003u+k]=value;w[0154]=major;}
 mix=(uint16_t)(e==0&&w[0140]==2u);
 if(e<2){if(mix){address=(uint16_t)(w[0150u+k]&03777u);dsky_store_cadr(w,address,1);w[0]=agc_add_register(w[0],agc_extend(agc_negate(k)));w[0145]=agc_correct(w[0]);}else dsky_store_cadr(w,w[01017],0);
  dec=w[01000];w[0]=agc_extend(agc_diminished_abs(dec));if(dec&&!(dec&040000u)){w[2]=mix ? 03124u : 03142u;w[5]=03174;*route=DSKY_INPUT_STORE_DECIMAL;return;}
  if(dec==0u){w[2]=mix ? 03125u : 03143u;w[5]=02452;(void)dsky_noun_check(s,io,&check);if(check==DSKY_NOUN_CHECK_RECYCLE){w[5]=cm ? 04161u : 04145u;*route=DSKY_INPUT_STORE_RECYCLE;return;}}
  if(dec!=077777u){w[2]=mix ? 03126u : 03144u;w[5]=mix ? 03034u : 03026u;if(cm){cm_native_io c={0};c.superbank=io->control.superbank;c.interrupt_enabled=io->control.interrupt_enabled;(void)cm_input_select(s,&c,&io->wrote_channels,&io->restart_light,&select);}else (void)dsky_input_select(s,io,&select);}
  dp=dsky_store_dp(agc_correct(w[0]),cm);w[2]=dp ? 02303u : 3u;
  if(dp){if(!mix)w[0117]=0;e=2;}else if(!mix){w[2]=03146;w[0]=agc_add_register(agc_extend(w[0146]),1);if(w[0]==0u||w[0]==0177777u){w[5]=03161;*route=DSKY_INPUT_STORE_CHANNEL;return;}}
 }
 if(e==2){w[0145]=agc_correct(agc_add_register(agc_extend(w[0145]),1));w[0]=agc_add_register(agc_extend(w[0145]),agc_extend(w[0117]));w[0117]=agc_correct(w[0]);address=dsky_store_address((uint16_t)((w[0117]-1u)&07777u),w[3]);w[address]=0;w[0]=0;w[2]=03137;}
 value=w[0154];w[0154]=agc_correct(w[0]);w[0]=agc_extend(value);w[5]=w[0115];w[2]=0116;*route=DSKY_INPUT_STORE_CALLER;
}
enum p66_bias_status dsky_input_store(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_input_store_route *route){if(!route||!dsky_store_domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;dsky_store_body(s,io,0,route);return P66_BIAS_OK;}
enum exec_status cm_input_store(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_input_store_route *route){dsky_lamp_io p;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.restart_light=*restart;if(!dsky_store_domain(s,&p,1))return EXEC_OUT_OF_SCOPE;dsky_store_body(s,&p,1,route);return EXEC_OK;}
