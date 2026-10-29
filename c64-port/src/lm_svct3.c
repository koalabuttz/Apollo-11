/* SPDX-License-Identifier: MIT; original Luminary099 SVCT3 end task. */
#include "lm_svct3.h"
#include "agc_word.h"
enum lm_svct3_exit lm_svct3_phase(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){enum agc_ccs_class flag,imu;
 if(s==0||io==0||phase>2u||s->words[6]!=004003u||s->words[3]!=01400u||s->words[4]!=004000u||io->interrupt_enabled>1u||(io->superbank&~0160u)!=0)return LM_SVCT3_INVALID;
 if(phase==1u){s->words[5]=05261;return LM_SVCT3_TASKOVER;}   /* TCF TASKOVER */
 if(phase==2u)s->words[2]=05261;                               /* TC SVCT3 */
 flag=agc_classify(s->words[0076]);s->words[0]=agc_diminished_abs(s->words[0076]);   /* CCS FLAGWRD2 */
 if(flag==AGC_POSITIVE||flag==AGC_PLUS_ZERO){s->words[5]=05261;return LM_SVCT3_TASKOVER;}
 imu=agc_classify(s->words[01304]);s->words[0]=agc_diminished_abs(s->words[01304]);  /* CKIMUSE: CCS IMUCADR */
 if(imu==AGC_PLUS_ZERO){s->words[0]=035000;s->words[2]=05253;s->words[5]=05072;return LM_SVCT3_NOVAC;}  /* CAF PRIO35; TC NOVAC */
 s->words[2]=05257;s->words[5]=05221;return LM_SVCT3_FIXDELAY;   /* SVCT3X: TC FIXDELAY */
}
