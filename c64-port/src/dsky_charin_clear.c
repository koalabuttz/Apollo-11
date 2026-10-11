/* SPDX-License-Identifier: MIT */
#include "dsky_charin_clear.h"
#include "dsky_two_digit.h"
#include "cm_formatter.h"
static enum p66_bias_status clear_body(apollo_overlay_state *s,dsky_charin_io *io,uint8_t cm){
 static const uint16_t singular[]={14,5,4},paired[]={13,9,3},first[]={14,9,4},bit[]={020,010,4};
 uint16_t *w=s->words,count=(w[0777]&040000u) ? agc_negate(w[0777]) : w[0777],relative;
 if(w[0154]!=036||w[5]!=(cm ? 02000u : 02077u)||w[3]>03400||(w[3]&0377u)||w[4]!=060000||w[6]!=(060000u|(w[3]>>8))||io->control.superbank!=0100||io->control.interrupt_enabled>1||io->channel11>077777||count>19||count==15)return P66_BIAS_OUT_OF_SCOPE;
 relative=count<5 ? 4 : count<10 ? 3 : count<15 ? 2 : count<18 ? 1 : 0;
 if(agc_classify(w[01015])==AGC_POSITIVE){if(relative>0)--relative;if(relative>=2&&(w[01001]>99||w[01034]==0))return P66_BIAS_OUT_OF_SCOPE;}
 if(relative>=2&&w[01023+(singular[relative-2u]>>1)]==0)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0115]=w[01012];w[01012]=1;io->wrote_channel11=0;
 if(w[01042]!=0){io->channel11=(uint16_t)(io->channel11|020u);io->wrote_channel11=1;w[0154]=io->channel11;}else w[0154]=0;
 w[0117]=036;w[0137]=relative;
 if(relative<2){w[0]=(uint16_t)(1u-relative);w[2]=cm ? 02435u : 02534u;}
 else{uint16_t i=(uint16_t)(relative-2u);
  if(agc_classify(w[01015])==AGC_POSITIVE){
   w[01013]=agc_correct(agc_add_register(agc_extend(w[01013]),3));w[0125]=relative;
   if(w[01001]!=0)w[01001]=w[01001]==1u ? 077777u : (uint16_t)(w[01001]-1u);
   w[0133]=cm ? 02421u : 02520u;w[0134]=062000;w[0117]=cm ? 04675u : 04631u;w[0777]=023;w[0]=w[01001]==077777u ? 0 : w[01001];w[2]=cm ? 03225u : 03320u;w[5]=cm ? 03213u : 03306u;
   if(cm){cm_native_io q={0};q.superbank=io->control.superbank;q.interrupt_enabled=io->control.interrupt_enabled;(void)cm_two_digit(s,&q);io->control.superbank=q.superbank;io->control.interrupt_enabled=q.interrupt_enabled;}
   else (void)dsky_two_digit(s,&io->control);
   /* DIM of +1 produces -0. DSPDECVN draws the same zero digits,
    * while its final SHORTMP leaves MPAC major as -0. The shared
    * formatter accepts ordinary 0..99, so retain this source zero
    * debris explicitly after the equivalent zero digit exchange. */
   if(w[01001]==077777u)w[0154]=077777u;
   w[0137]=w[0125];
  }
  w[1]=cm ? 02424u : 02523u;w[01001+relative]=0;w[01004+relative]=0;w[0124]=0;w[01000]=(uint16_t)(w[01000]&agc_negate(bit[i])&077774u);w[0143]=singular[i];w[2]=cm ? 02460u : 02557u;(void)dsky_pinball_write(s,&io->control,DSKY_DSPIN);
  w[0777]=paired[i];w[2]=cm ? 02464u : 02563u;(void)dsky_pinball_write(s,&io->control,DSKY_2BLANK);
  w[0777]=(uint16_t)(paired[i]-2u);w[2]=cm ? 02467u : 02566u;(void)dsky_pinball_write(s,&io->control,DSKY_2BLANK);
  w[0777]=first[i];w[01015]=agc_correct(agc_add_register(agc_extend(w[01015]),1));w[0]=first[i];w[2]=cm ? 02426u : 02525u;
 }
 w[5]=cm ? 05217u : 05155u;w[7]=0;return P66_BIAS_OK;
}
enum p66_bias_status dsky_charin_clear(apollo_overlay_state *s,dsky_charin_io *io){return clear_body(s,io,0);}
enum exec_status cm_charin_clear(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote){dsky_charin_io p;enum p66_bias_status status;
 if(wrote==0)return EXEC_OUT_OF_SCOPE;
 p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;status=clear_body(s,&p,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;
}
