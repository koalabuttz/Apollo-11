/* SPDX-License-Identifier: MIT */
/* NORMUNIT and QUICTRIG for FINDCDUW's RTBs; see include/lm_findcduw_rtb.h. */
#include "lm_findcduw_rtb.h"
#include "rtb_vector.h"
#include "lm_interp_orders.h"
#include "lm_powered_quick.h"
enum rtb_status lm_findcduw_normunit(apollo_overlay_state *s,p66_bias_io *io){
 if(rtb_vector(s,io,RTB_LM,RTB_NORMUNIT_FRONT)!=RTB_OK)return RTB_OUT_OF_SCOPE;
 return lmi_unit1(s)?RTB_OUT_OF_SCOPE:RTB_OK;}
enum rtb_status lm_findcduw_quictrig(apollo_overlay_state *s,p66_bias_io *io){
 if(lm_powered_quick(s,&io->interrupt_enabled)!=LM_POWERED_QUICK_OK)return RTB_OUT_OF_SCOPE;
 if(lm_powered_quick_return(s)!=LM_POWERED_QUICK_OK)return RTB_OUT_OF_SCOPE;
 lmi_swreturn(s);
 return RTB_OK;
}
