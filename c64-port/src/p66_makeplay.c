/* SPDX-License-Identifier: MIT */
#include "p66_makeplay.h"
static uint8_t ready(const uint16_t *w,const p66_bias_io *io,uint16_t pc){return(uint8_t)(w[5]==pc&&w[3]==03400u&&w[4]==020000u&&w[6]==020007u&&w[7]==0&&w[0155]==01474u&&w[0157]==063476u&&w[0160]==023400u&&io->superbank<=0160u&&(io->superbank&017u)==0&&io->interrupt_enabled<=1u);}
enum p66_makeplay_boundary p66_makeplay_priority(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words;if(!ready(w,io,02546)||w[0167]==0||w[0167]>=040000u||(w[0167]&037000u)==0)return P66_MAKEPLAY_OUT_OF_SCOPE;
 w[0163]=(agc_word)(w[0167]&037000u);w[0]=033000;w[1]=agc_extend(agc_correct(w[1]));w[2]=02553;w[5]=05146;return P66_MAKEPLAY_PRIORITY;
}
enum p66_makeplay_boundary p66_makeplay_select(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11){uint16_t *w=s->words,pm,which,address;
 if(!ready(w,io,02553)||w[0163]<01000u||(w[0163]&0140777u)||w[01012]>7u||*channel11>AGC_MASK)return P66_MAKEPLAY_OUT_OF_SCOPE;
 w[0164]=2;w[1]=agc_extend(agc_correct(w[1]));
 /* Fixed P66 flags are LINUS/nonflashing. Matching an existing normal
  * display while the astronaut owns DSPLOCK ends the duplicate job. */
 if(w[0374]==063476u&&w[01012]!=0){w[0]=(agc_word)(w[01012]-1u);w[2]=03410;w[5]=05155;return P66_MAKEPLAY_END;}
 w[01072]=023400;w[0371]=01474;w[0366]=(agc_word)(w[0163]|io->superbank);io->interrupt_enabled=1;
 pm=(agc_word)(w[0100]&066521u);
 if(pm!=0){w[0]=agc_diminished_abs(pm);w[2]=3;w[5]=02754;return P66_MAKEPLAY_SLEEP;}
 *channel11=(agc_word)(*channel11&077757u);which=(uint16_t)((w[0100]&0700u)!=0);address=(uint16_t)(01042u+which);
 w[0]=agc_extend(w[address]);w[address]=0;w[1]=which;w[2]=02614;w[5]=05137;io->interrupt_enabled=0;return P66_MAKEPLAY_WAKE;
}
enum p66_makeplay_boundary p66_makeplay_wake_return(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words,offset=w[064];
 if(!ready(w,io,02614)||w[0164]!=2u||w[01072]!=023400u||w[0371]!=01474u||(offset!=077776u&&(offset>84u||offset%12u)))return P66_MAKEPLAY_OUT_OF_SCOPE;
 /* XCHTOEND leaves ENDINST in LOCCTR if CCS found no sleeping job. */
 if(offset==0||offset==077776u)w[064]=05155;else w[0164u+offset]=05155;
 w[0]=2;w[1]=agc_extend(agc_correct(w[1]));w[2]=3;w[0164]=2;w[5]=03070;io->interrupt_enabled=1;return P66_MAKEPLAY_PLAY;
}
