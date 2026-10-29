/* SPDX-License-Identifier: MIT
 * Original CHARIN/CHARIN2/RELDSPON and literal VERB/NOUN table branches. */
#include "dsky_charin_nv.h"
enum p66_bias_status dsky_charin_nv(apollo_overlay_state *s,dsky_charin_io *io){uint16_t *w=s->words,code=w[0154];
 if((code!=021 && code!=037) || w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=060000 || w[6]!=(060000u|(w[3]>>8)) || io->control.superbank!=0100 || io->control.interrupt_enabled>1 || io->channel11>077777)return P66_BIAS_OUT_OF_SCOPE;
 w[0115]=w[01012];w[01012]=1;io->wrote_channel11=0;
 if(w[01042]!=0){io->channel11=(uint16_t)(io->channel11|020u);io->wrote_channel11=1;w[0154]=io->channel11;}else w[0154]=0;
 w[0117]=code;return dsky_nv_key(s,&io->control,code==021 ? DSKY_KEY_VERB : DSKY_KEY_NOUN);
}
