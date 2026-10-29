/* SPDX-License-Identifier: MIT
 * Literal LM/CM ERROR key source stores, channel outputs and TSTAB loop. */
#include "dsky_charin_reset.h"
static enum p66_bias_status reset_body(apollo_overlay_state *s,dsky_reset_io *io,uint8_t cm){
 uint16_t *w=s->words,i,mode30=cm ? 01320u : 01302u,mode33=cm ? 01321u : 01303u;
 if(w[0154]!=022||w[5]!=(cm ? 02000u : 02077u)||w[3]>03400||(w[3]&0377u)||w[4]!=060000||w[6]!=(060000u|(w[3]>>8))||io->control.superbank!=0100||io->control.interrupt_enabled>1||io->channel11>077777||io->channel13>077777||io->restart_light>1||(io->channel13&034000u)!=0)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 /* CCS CADRSTOR's -0 skip lands on AD MPAC, bypassing CS ELRCODE1;
  * that exceptional path really lights key-release even for RSET.
  * Other occupied CADRSTOR values cancel the key and leave MPAC -0.
  * ERROR restores old DSPLOCK and exchanges022 into 21/22REG. */
 if(w[01042]==077777u){io->channel11=(uint16_t)(io->channel11|020u);w[0154]=io->channel11;}
 else w[0154]=w[01042]!=0 ? 077777u : 0;
 w[0115]=022;
 io->channel11=(uint16_t)((io->channel11|01000u)&077673u);
 io->channel13=(uint16_t)(io->channel13&076777u);io->wrote_channels=3;io->restart_light=0;
 w[01036]=(uint16_t)((w[01036]&050u)|040000u);
 w[mode33]=(uint16_t)((w[mode33]&061777u)|016000u);
 w[mode30]=(uint16_t)(w[mode30]|01000u);
 if(cm)w[01331]=(uint16_t)(w[01331]|0100u);
 else w[0110]=(uint16_t)((w[0110]&077447u)|0100u);
 /* CCS/AD/CS distinctions preserve both signed zeros. Positive entries
  * set bit12; negative entries clear its complemented representation.
  * Clean +0 becomes04000 and -0 becomes073777. NOUT is unchanged. */
 for(i=0;i<11;++i){uint16_t old=w[01023+i];w[01023+i]=(old&040000u)!=0 ? (uint16_t)(old&073777u) : (uint16_t)(old|04000u);}
 w[0117]=0;w[0375]=0;w[0376]=0;w[0377]=0;w[01357]=0;
 w[0]=0;w[2]=cm ? 03574u : 03670u;w[5]=cm ? 05217u : 05155u;w[7]=0;io->control.interrupt_enabled=1;
 return P66_BIAS_OK;
}
enum p66_bias_status dsky_charin_reset(apollo_overlay_state *s,dsky_reset_io *io){return reset_body(s,io,0);}
enum exec_status cm_charin_reset(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,uint16_t *restart_light){dsky_reset_io p;enum p66_bias_status status;
 if(wrote==0||restart_light==0)return EXEC_OUT_OF_SCOPE;
 p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.channel13=io->channel13;p.wrote_channels=0;p.restart_light=*restart_light;
 status=reset_body(s,&p,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;
 io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;io->channel13=p.channel13;*wrote=p.wrote_channels;*restart_light=p.restart_light;return EXEC_OK;
}
