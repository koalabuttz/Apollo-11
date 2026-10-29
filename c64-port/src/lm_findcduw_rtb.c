/* SPDX-License-Identifier: MIT */
/* NORMUNIT and QUICTRIG for FINDCDUW's RTBs; see include/lm_findcduw_rtb.h. */
#include "lm_findcduw_rtb.h"
#include "rtb_vector.h"
#include "rtb_unit.h"
#include "lm_interp_orders.h"
#include "lm_powered_quick.h"
enum rtb_status lm_findcduw_normunit(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase;
 if(rtb_vector(s,io,RTB_LM,RTB_NORMUNIT_FRONT)!=RTB_OK)return RTB_OUT_OF_SCOPE;
 for(phase=0;phase<3u;++phase){
  enum rtb_unit_status st=rtb_unit_phase(s,io,RTB_LM,phase);
  if(st==RTB_UNIT_OVERFLOW)break;           /* DVOVF's TC DANZIG */
  if(st!=RTB_UNIT_OK)return RTB_OUT_OF_SCOPE; /* (1302 or out of scope) */
 }
 return RTB_OK;
}
enum rtb_status lm_findcduw_quictrig(apollo_overlay_state *s,p66_bias_io *io){
 if(lm_powered_quick(s,&io->interrupt_enabled)!=LM_POWERED_QUICK_OK)return RTB_OUT_OF_SCOPE;
 if(lm_powered_quick_return(s)!=LM_POWERED_QUICK_OK)return RTB_OUT_OF_SCOPE;
 lmi_swreturn(s);
 return RTB_OK;
}
