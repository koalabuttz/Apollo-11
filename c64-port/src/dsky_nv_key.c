/* SPDX-License-Identifier: MIT
 * Literal original VERB/NOUN/NVCOM stores; verified2BLANK composition. */
#include "dsky_nv_key.h"
enum p66_bias_status dsky_nv_key(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_key key){uint16_t *w=s->words;
 if((unsigned)key>DSKY_KEY_NOUN || w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=060000 || w[6]!=(060000u|(w[3]>>8)) || io->superbank!=0100 || io->interrupt_enabled>1)return P66_BIAS_OUT_OF_SCOPE;
 w[key==DSKY_KEY_VERB ? 01001 : 01002]=0;w[0777]=key==DSKY_KEY_VERB ? 023 : 021;w[2]=02361;(void)dsky_pinball_write(s,io,DSKY_2BLANK);
 w[01000]=1;w[01013]=0;w[0136]=05155;w[0]=05155;w[2]=02370;w[5]=05155;w[7]=0;return P66_BIAS_OK;
}
