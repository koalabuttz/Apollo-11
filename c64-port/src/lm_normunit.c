/* SPDX-License-Identifier: MIT */
#include "lm_normunit.h"
static int normunit(apollo_overlay_state *s,p66_bias_io *io,enum rtb_vector_entry front)
{
 enum rtb_unit_status st;uint8_t phase;
 if(rtb_vector(s,io,RTB_LM,front)!=RTB_OK)return RTB_OUT_OF_SCOPE;
 for(phase=0;phase<3u;++phase){
  st=rtb_unit_phase(s,io,RTB_LM,phase);
  if(st==RTB_UNIT_ABORT_1302)return 2;
  if(st==RTB_UNIT_OUT_OF_SCOPE)return RTB_OUT_OF_SCOPE;
  if(st==RTB_UNIT_OVERFLOW)break;
 }
 return RTB_OK;
}
int lm_normunit(apollo_overlay_state *s,p66_bias_io *io){return normunit(s,io,RTB_NORMUNIT_FRONT);}
int lm_normunx1(apollo_overlay_state *s,p66_bias_io *io){return normunit(s,io,RTB_NORMUNX1_FRONT);}
