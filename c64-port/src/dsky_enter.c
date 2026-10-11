/* SPDX-License-Identifier: MIT */
#include "dsky_enter.h"
static enum p66_bias_status enter_body(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_enter_route *route,uint8_t cm){uint16_t *w=s->words,request=w[01013],mode,bank;
 bank=w[5]==02002u ? 062000u : 060000u;
 if(route==0||w[5]!=(bank==062000u ? 02002u : cm ? 02000u : 02077u)||(bank==060000u&&w[0154]!=034u)||w[4]!=bank||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(bank|(w[3]>>8))||w[7]!=0||io->control.superbank!=0100u||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));io->wrote_channel11=0;
 if(bank==060000u){w[0115]=w[01012];w[01012]=1;if(w[01042]!=0){io->channel11=(uint16_t)(io->channel11|020u);io->wrote_channel11=1;w[0154]=io->channel11;}else w[0154]=0;w[0117]=034;w[4]=062000;w[6]=(uint16_t)(062000u|(w[3]>>8));w[2]=2;}
 w[01015]=0;w[0136]=cm ? 05217u : 05155u;w[0]=agc_diminished_abs(request);
 if((request&040000u)==0){w[2]=request ? 02010u : 02011u;w[5]=02035;*route=DSKY_ENTER_PASS_ZERO;return P66_BIAS_OK;}
 if(request!=077777u)w[2]=02012;
 w[0]=agc_add_register(03431,agc_extend(request));
 if(agc_correct(w[0])!=0&&agc_correct(w[0])!=077777u){mode=(uint16_t)(w[01000]&3u);w[0]=mode ? (uint16_t)(mode-1u) : 0;if(mode!=0){w[0]=agc_diminished_abs(w[0777]);if((w[0777]&040000u)==0){w[2]=w[0777] ? 02025u : 02026u;w[5]=02351u;*route=DSKY_ENTER_ALARM;return P66_BIAS_OK;}}}
 w[01013]=agc_negate(request);io->channel11=(uint16_t)(io->channel11&077737u);io->wrote_channel11=1;w[0]=agc_extend(io->channel11);w[2]=02033;w[5]=01013;*route=DSKY_ENTER_REQUEST;return P66_BIAS_OK;
}
enum p66_bias_status dsky_enter(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_enter_route *route){return enter_body(s,io,route,0);}
enum exec_status cm_enter(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,enum dsky_enter_route *route){dsky_charin_io p;enum p66_bias_status status;if(wrote==0)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;status=enter_body(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;}
