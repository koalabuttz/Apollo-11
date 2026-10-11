/* SPDX-License-Identifier: MIT */
#include "p66_runtime_fetch_bridge.h"
#include "agc_word.h"
enum p66_bridge_status p66_runtime_throttle_fetch(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words;
 uint8_t after_poll=(uint8_t)(w[5]==06067u&&w[0164]==03507u&&w[020]==020006u);
 uint8_t newops=(uint8_t)(w[5]==06070u&&w[0164]==03510u&&w[020]<=AGC_MASK);
 if((!after_poll&&!newops)||io->superbank>060u||(io->superbank&017u)||io->interrupt_enabled>1u||w[3]!=03400u||w[4]!=064000u||w[6]!=064007u||w[7]!=0||w[0165]!=064007u||w[0163]!=0||w[023]!=0||w[067]>AGC_MASK)return P66_BRIDGE_OUT_OF_SCOPE;
 /* Source INCR LOC is already beyond CCS NEWJOB. The compiled fixed
  * NEWOPS EXIT fetch edits CYR to zero before the original EXIT body. */
 w[0164]=03510;w[5]=06070;w[020]=0;
 return p66_runtime_throttle_enter(s,io);
}
