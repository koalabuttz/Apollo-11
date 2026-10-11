/* SPDX-License-Identifier: MIT */
#include "p66_makeplay_sleep.h"
static uint8_t ready(const uint16_t *w,const p66_bias_io *io,uint16_t pc){return(uint8_t)(w[5]==pc&&w[3]==03400u&&w[4]==020000u&&w[6]==020007u&&w[7]==0&&w[0155]==01474u&&w[0157]==063476u&&w[0160]==023400u&&w[01072]==023400u&&w[0371]==01474u&&io->superbank<=0160u&&(io->superbank&017u)==0&&io->interrupt_enabled<=1u);}
enum p66_sleep_boundary p66_makeplay_sleep_enter(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words;if(!ready(w,io,02754)||w[0164]!=2u)return P66_SLEEP_OUT_OF_SCOPE;
 /* Normal PRIOOCT/WAITMASK passes01004 to UPENT2, whose low-three mask
  * sets only normal-wait bit01000. CS ONE plus COPINDEX gives FACEREG1. */
 w[0100]|=01000;w[0154]=1;w[0]=020615;w[1]=01000;w[2]=02770;w[5]=05137;io->interrupt_enabled=0;return P66_SLEEP_WAKE;
}
enum p66_sleep_boundary p66_makeplay_sleep_return(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words,offset=w[064];if(!ready(w,io,02770)||w[0154]!=1u||w[0164]!=2u||(offset!=077776u&&(offset>84u||offset%12u)))return P66_SLEEP_OUT_OF_SCOPE;
 if(offset==0||offset==077776u)w[064]=05155;else w[0164u+offset]=05155;
 w[0]=020615;w[1]=agc_extend(agc_correct(w[1]));w[2]=3;w[5]=05133;io->interrupt_enabled=1;return P66_SLEEP_EXECUTIVE;
}
enum p66_sleep_boundary p66_makeplay_sleep_resume(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words;if(!ready(w,io,02615))return P66_SLEEP_OUT_OF_SCOPE;
 w[0]=2;w[1]=agc_extend(agc_correct(w[1]));w[0164]=2;w[5]=03070;return P66_SLEEP_PLAY;
}
enum p66_sleep_boundary p66_makeplay_busy(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words,cadr=w[01042];if(!ready(w,io,03121)||w[0164]!=2u||w[0162]!=011254u)return P66_SLEEP_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 if(cadr!=0&&cadr!=AGC_NEG_ZERO){w[0]=agc_diminished_abs(cadr);w[5]=05155;return P66_SLEEP_END;}
 /* ASTROMSK normal value00204 loses its lowthreebits in UPENT2. */
 w[0100]|=0200;w[0]=020615;w[1]=0200;w[2]=03260;w[5]=04442;io->interrupt_enabled=1;return P66_SLEEP_PINBALL;
}
