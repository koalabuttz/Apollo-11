/* SPDX-License-Identifier: MIT */
#include "p66_throttle_bridge.h"
#include "agc_word.h"
static uint8_t valid_io(const p66_bias_io *io){return (uint8_t)(io->superbank<=060u&&!(io->superbank&017u)&&io->interrupt_enabled<=1u);}
static uint8_t banks(const uint16_t *w,agc_word bank){return (uint8_t)(w[3]==03400u&&w[4]==bank&&w[6]==(bank|7u)&&w[7]==0);}
enum p66_bridge_status p66_throttle_enter(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words;agc_register a,l;
 if(!valid_io(io)||!banks(w,064000)||w[5]!=06061u||w[0164]!=03507u||w[0165]!=064007u||w[0163]!=0||w[023]!=0||w[067]!=0)return P66_BRIDGE_OUT_OF_SCOPE;
 /* DANZIG fetches the original EXIT, clearing its CYR/EDOP prefix. EXIT then
  * places caller BBANK in A before the explicit DXCH MPAC. */
 a=agc_extend(w[0154]);l=agc_extend(w[0155]);w[0154]=064007;w[0155]=agc_correct(w[1]);
 w[020]=0;w[023]=0;w[0164]=03510;
 /* BANKCALL preserves measured A/L, leaves a basic return pair in BUF2,
  * and TC10000 sets Q to SWRETURN. */
 w[0133]=03514;w[0134]=064000;w[0]=a;w[1]=l;w[2]=04631;
 w[4]=062000;w[6]=062007;w[5]=02221;
 return P66_BRIDGE_OK;
}
enum p66_bridge_status p66_throttle_return(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words;
 if(!valid_io(io)||!banks(w,062000)||w[5]!=02367u||w[0133]!=03514u||w[0134]!=064000u||w[03607]!=04631u||w[0164]!=03510u||w[0165]!=064007u||w[067]!=0)return P66_BRIDGE_OUT_OF_SCOPE;
 /* RTNHOLD dispatches to SWRETURN, whose FBANK exchange updates BUF2+1.
  * Original TC INTPRET makes Q03515; QXCH LOC retains the prior LOC in Q. */
 w[0134]=062000;w[4]=064000;w[6]=064007;w[1]=agc_extend(agc_correct(w[1]));w[2]=03510;
 w[0164]=03515;w[0165]=064007;w[0115]=040000;w[023]=0;
 w[0]=agc_extend(040000);w[5]=06070;io->interrupt_enabled=1;
 return P66_BRIDGE_OK;
}
