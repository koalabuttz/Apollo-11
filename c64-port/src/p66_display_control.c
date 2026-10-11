/* SPDX-License-Identifier: MIT */
#include "p66_display_control.h"
#include "agc_word.h"
enum p66_display_boundary p66_display_exit(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words;uint8_t suppressed=(uint8_t)((w[0104]&01000u)!=0);
 if(io->superbank>060u||(io->superbank&017u)||io->interrupt_enabled>1u||w[5]!=03531u||w[3]!=03400u||w[4]!=064000u||w[6]!=064007u||w[7]!=0||w[0164]!=03530u||w[0165]!=064007u||w[0163]!=0||w[020]!=0||w[023]!=0||w[067]!=0||(!suppressed&&w[03621]!=2u))return P66_DISPLAY_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[2]=04631;
 if(suppressed){w[0133]=03533;w[0134]=064000;w[0]=0;w[4]=062000;w[6]=062007;w[5]=05155;return P66_DISPLAY_ENDOFJOB;}
 /* The first call saves the P66 return, then VERTDISP makes V06N60 and the
  * second BANKCALL replaces BUF2 with ENDLLJOB's return in bank31. */
 w[0133]=03476;w[0134]=062000;w[0]=01474;w[4]=020000;w[6]=020007;w[5]=02470;
 return P66_DISPLAY_REGODSPR;
}
