/* SPDX-License-Identifier: MIT */
#include "p66_goplay.h"
static uint8_t ready(const uint16_t *w,const p66_bias_io *io,uint16_t pc){return(uint8_t)(w[5]==pc&&w[3]==03400u&&w[4]==020000u&&w[6]==020007u&&w[7]==0&&w[0164]==2u&&w[01072]==023400u&&w[0371]==01474u&&io->superbank<=060u&&(io->superbank&017u)==0&&io->interrupt_enabled<=1u);}
enum p66_bias_status p66_goplay_enter(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words;if(!ready(w,io,03070))return P66_BIAS_OUT_OF_SCOPE;
 w[0162]=011254;w[0160]=023400;w[0141]=0;w[0165]=w[0156];w[01073]=w[0156];w[0]=01474;w[1]=0;w[2]=03121;w[5]=04155;return P66_BIAS_OK;
}
enum p66_bias_status p66_goplay_finish(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11){uint16_t *w=s->words,oldflags;if(!ready(w,io,03122)||w[01012]!=0||(w[01021]&020000u)!=0||*channel11>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 *channel11=(agc_word)(*channel11&077737u);w[0156]=w[0165];w[0162]=011254;w[0160]=023400;
 oldflags=(uint16_t)(w[0100]&077177u);w[0100]=(uint16_t)(oldflags&077677u);w[061]=4;w[062]=8;w[063]=oldflags;w[0123]=0;
 w[0]=60;w[1]=03132;w[2]=3;w[5]=05155;io->interrupt_enabled=1;return P66_BIAS_OK;
}
