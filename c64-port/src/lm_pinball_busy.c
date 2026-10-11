/* SPDX-License-Identifier: MIT */
#include "lm_pinball_busy.h"
enum lm_busy_status lm_pinball_busy(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11){uint16_t *w=s->words,cadr=agc_correct(w[0]);agc_word invalid;
 if(w[5]!=04442u||w[0]!=agc_extend(cadr)||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||*channel11>AGC_MASK||w[01043]==AGC_NEG_ZERO)return LM_BUSY_OUT_OF_SCOPE;
 /* POSTJUMP selects original bank4 NVSUBSY1 while preserving wakeCADR. */
 w[4]=010000;w[6]=(uint16_t)(010000u|(w[3]>>8));w[1]=agc_extend(cadr);
 invalid=(agc_word)(w[01042]!=0&&w[01042]!=AGC_NEG_ZERO?w[01042]:w[01043]);
 if(invalid!=0){w[0]=agc_diminished_abs(invalid);w[2]=04230;w[5]=05652;return LM_BUSY_ALARM;}
 *channel11=(agc_word)(*channel11|020u);w[01043]=cadr;w[0]=agc_extend(cadr);w[2]=02656;w[5]=05133;return LM_BUSY_SLEEP;
}
