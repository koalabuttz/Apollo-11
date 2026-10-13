/* SPDX-License-Identifier: MIT */
/* Native CSM_GEOMETRY RTB routine and Comanche SHORTMP
 * (include/geometry_rtb.h). Each instruction below is the original's, on
 * the canonical image. */
#include "geometry_rtb.h"
#include "agc_image_ops.h"
#include "rtb_angles.h"
#define W s->words
uint8_t geometry_shortmp(apollo_overlay_state *s){
 /* SHORTMP (07256): TS MPTEMP; EXTEND; MP MPAC +2; TS MPAC +2; CAF ZERO;
  * XCH MPAC +1; TCF DMPSUB2. (A TS that skips on overflow is refused.) */
 if(ai_ts(s,0135u))return RTB_OUT_OF_SCOPE;
 ai_mp(s,0156u);
 if(ai_ts(s,0156u))return RTB_OUT_OF_SCOPE;
 ai_caf(s,0u);ai_xch(s,0155u);
 /* DMPSUB2 (07075): EXTEND; MP MPTEMP; DAS MPAC +1; XCH MPAC; EXTEND;
  * MP MPTEMP; DAS MPAC; TC Q (07104, leaving Q 07105). */
 ai_mp(s,0135u);ai_das(s,0155u);ai_xch(s,0154u);ai_mp(s,0135u);ai_das(s,0154u);
 W[2]=07105u;
 return RTB_OK;
}
uint8_t geometry_rtb(apollo_overlay_state *s,p66_bias_io *io){
 unsigned i;
 if(!s||!io||W[5]!=02027u||W[4]!=046000u||io->interrupt_enabled>1u||W[7]||W[3]>03400u||(W[3]&0377u))return RTB_OUT_OF_SCOPE;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return RTB_OUT_OF_SCOPE;
 /* SXTLOGIC: CAF 10DEGS-; ADS MPAC; CAF QUARTER; TC SHORTMP. */
 ai_caf(s,07020u);ai_ads(s,0154u);ai_caf(s,010000u);ai_tc(s,02033u);
 if(geometry_shortmp(s)!=RTB_OK)return RTB_OUT_OF_SCOPE;
 /* TC DANZIG. */
 ai_tc(s,02034u);W[5]=06030u;
 return RTB_OK;
}
