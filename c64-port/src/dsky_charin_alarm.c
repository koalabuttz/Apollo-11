/* SPDX-License-Identifier: MIT */
#include "dsky_charin_alarm.h"
static enum p66_bias_status alarm_body(apollo_overlay_state *s,dsky_charin_io *io,uint8_t cm){
 uint16_t *w=s->words,key=w[0154];uint8_t eightnine=(uint8_t)(key==010u||key==011u);
 if(!(key==0||(key>=012u&&key<=017u)||(key>=023u&&key<=030u)||key==035u||(eightnine&&(w[01000]&3u)==0)))return P66_BIAS_OUT_OF_SCOPE;
 if(w[5]!=(cm ? 02000u : 02077u)||w[3]>03400||(w[3]&0377u)||w[4]!=060000||w[6]!=(060000u|(w[3]>>8))||io->control.superbank!=0100||io->control.interrupt_enabled>1||io->channel11>077777)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0115]=w[01012];w[01012]=1;io->wrote_channel11=0;
 /* CADRSTOR=-0 skips CS ELRCODE1. A zero illegal key then bypasses
  * RELDSPON; other invalid keys use the ordinary key-release output. */
 if(w[01042]!=0&&!(w[01042]==077777u&&key==0)){io->channel11=(uint16_t)(io->channel11|020u);io->wrote_channel11=1;w[0154]=io->channel11;}else w[0154]=0;
 w[0117]=key;
 if(eightnine&&(w[0777]&040000u)!=0){w[0]=agc_extend(agc_diminished_abs(w[0777]));w[2]=w[0777]==077777u ? (cm ? 02067u : 02166u) : (cm ? 02066u : 02165u);}
 else{io->channel11=(uint16_t)(io->channel11|0100u);io->wrote_channel11=1;w[0]=agc_extend(io->channel11);w[2]=cm ? 03343u : 03436u;}
 w[5]=cm ? 05217u : 05155u;w[7]=0;return P66_BIAS_OK;
}
enum p66_bias_status dsky_charin_alarm(apollo_overlay_state *s,dsky_charin_io *io){return alarm_body(s,io,0);}
enum exec_status cm_charin_alarm(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote){dsky_charin_io p;enum p66_bias_status status;
 if(wrote==0)return EXEC_OUT_OF_SCOPE;
 p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;status=alarm_body(s,&p,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;
 io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;
}
