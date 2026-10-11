/* SPDX-License-Identifier: MIT */
#include "p66_runtime_bridge.h"
#include "agc_word.h"
static uint8_t valid_io(const p66_bias_io *io){return (uint8_t)(io->superbank<=060u&&!(io->superbank&017u)&&io->interrupt_enabled<=1u);}
static uint8_t banks(const uint16_t *w,agc_word bank){return (uint8_t)(w[3]==03400u&&w[4]==bank&&w[6]==(bank|7u)&&w[7]==0);}
enum p66_bridge_status p66_runtime_throttle_enter(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words;agc_register a,l;
 if(!valid_io(io)||!banks(w,064000)||(w[5]!=06070u&&w[5]!=06743u)||w[0164]!=03510u||w[0165]!=064007u||w[0163]!=0||w[020]!=0||w[023]!=0||w[067]>AGC_MASK)return P66_BRIDGE_OUT_OF_SCOPE;
 /* NEWOPS decodes the fixed EXIT pair without a DANZIG job test. EXIT
  * restores BBANK; the following DXCH exchanges measured acceleration. */
 a=agc_extend(w[0154]);l=agc_extend(w[0155]);w[0154]=064007;w[0155]=agc_correct(w[1]);
 w[0133]=03514;w[0134]=064000;w[0]=a;w[1]=l;w[2]=04631;
 w[4]=062000;w[6]=062007;w[5]=02221;
 return P66_BRIDGE_OK;
}
enum p66_bridge_status p66_runtime_throttle_return(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words;
 if(!valid_io(io)||!banks(w,062000)||w[5]!=02367u||w[0133]!=03514u||w[0134]!=064000u||w[03607]!=04631u||w[0164]!=03510u||w[0165]!=064007u||w[067]>AGC_MASK)return P66_BRIDGE_OUT_OF_SCOPE;
 /* Basic SWRETURN and INTPRET do not examine NEWJOB. The next true
  * interpretive boundary will own that pending request. */
 w[0134]=062000;w[4]=064000;w[6]=064007;w[1]=agc_extend(agc_correct(w[1]));w[2]=03510;
 w[0164]=03515;w[0165]=064007;w[0115]=040000;w[023]=0;
 w[0]=agc_extend(040000);w[5]=06070;io->interrupt_enabled=1;
 return P66_BRIDGE_OK;
}
enum p66_display_boundary p66_runtime_display_exit(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words;uint8_t suppressed=(uint8_t)((w[0104]&01000u)!=0);
 if(!valid_io(io)||!banks(w,064000)||w[5]!=03531u||w[0164]!=03530u||w[0165]!=064007u||w[0163]!=0||w[020]!=0||w[023]!=0||w[067]>AGC_MASK||(!suppressed&&w[03621]!=2u))return P66_DISPLAY_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[2]=04631;
 if(suppressed){w[0133]=03533;w[0134]=064000;w[0]=0;w[4]=062000;w[6]=062007;w[5]=05155;return P66_DISPLAY_ENDOFJOB;}
 w[0133]=03476;w[0134]=062000;w[0]=01474;w[4]=020000;w[6]=020007;w[5]=02470;
 return P66_DISPLAY_REGODSPR;
}
