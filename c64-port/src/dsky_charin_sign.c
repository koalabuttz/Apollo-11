/* SPDX-License-Identifier: MIT */
#include "dsky_charin_sign.h"
static enum p66_bias_status sign_body(apollo_overlay_state *s,dsky_charin_io *io,uint8_t cm){
 static const uint16_t counts[]={14,9,4},minus[]={5,3,0},bits[]={020,010,4};
 uint16_t *w=s->words,key=w[0154],mode=(uint16_t)(w[01000]&3u),i,relative=0;uint8_t accepted=0;
 uint16_t plus=cm ? 02310u : 02407u,negative=cm ? 02275u : 02374u,sgn=cm ? 02323u : 02422u,test=cm ? 02347u : 02446u,finish=cm ? 05217u : 05155u;
 if((key!=032 && key!=033)||w[5]!=(cm ? 02000u : 02077u)||w[3]>03400||(w[3]&0377u)||w[4]!=060000||w[6]!=(060000u|(w[3]>>8))||io->control.superbank!=0100||io->control.interrupt_enabled>1||io->channel11>077777)return P66_BIAS_OUT_OF_SCOPE;
 if(mode==0){for(i=0;i<3;++i)if(w[0777]==counts[i]){accepted=1;relative=(uint16_t)(i+2u);break;}}
 if(accepted&&(w[01023+minus[relative-2u]]==0||w[01024+minus[relative-2u]]==0))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0115]=w[01012];w[01012]=1;io->wrote_channel11=0;
 if(w[01042]!=0){io->channel11=(uint16_t)(io->channel11|020u);io->wrote_channel11=1;w[0154]=io->channel11;}else w[0154]=0;
 w[0117]=key;w[1]=(uint16_t)((key==032 ? plus : negative)+1u);
 if(mode!=0){w[0]=(uint16_t)(mode-1u);w[2]=(uint16_t)(test+5u);}
 else if(!accepted){w[0]=agc_add_register(agc_extend(agc_negate(4)),agc_extend(w[0777]));w[2]=(uint16_t)(test+014u);}
 else{
  uint16_t index=(uint16_t)(relative-2u),off=(uint16_t)(minus[index]+(key==033 ? 1u : 0u)),on=(uint16_t)(minus[index]+(key==032 ? 1u : 0u));
  w[0137]=relative;w[1]=(uint16_t)((key==032 ? plus : negative)+2u);
  w[0122]=on;w[0123]=0;w[0124]=0;w[0]=off;w[2]=(uint16_t)(sgn+4u);(void)dsky_pinball_write(s,&io->control,DSKY_11DSPIN);
  w[0124]=02000;w[0]=w[0122];w[0122]=02000;w[2]=(uint16_t)(sgn+010u);(void)dsky_pinball_write(s,&io->control,DSKY_11DSPIN);
  w[01000]=agc_correct(agc_add_register(agc_extend(w[01000]),agc_extend((agc_word)(bits[index]+(key==032 ? 1u : 2u)))));
  w[0]=agc_extend(agc_diminished_abs(w[01015]));if((w[01015]&040000u)==0){w[01015]=0;w[0]=0;}
  w[2]=plus;
 }
 w[5]=finish;w[7]=0;return P66_BIAS_OK;
}
enum p66_bias_status dsky_charin_sign(apollo_overlay_state *s,dsky_charin_io *io){return sign_body(s,io,0);}
enum exec_status cm_charin_sign(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote){dsky_charin_io p;enum p66_bias_status status;
 if(wrote==0)return EXEC_OUT_OF_SCOPE;
 p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;
 status=sign_body(s,&p,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;
 io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;
}
