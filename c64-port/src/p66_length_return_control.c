/* SPDX-License-Identifier: MIT */
#include "p66_length_return_control.h"
uint16_t p66_length_return_control(apollo_overlay_state *image,uint16_t selector){uint16_t *w;if(!image)return P66_NATIVE_BAD_MODE;w=image->words;if((selector!=212u&&selector!=236u)||w[5]!=06061u||w[0164]!=(selector==212u?03437u:03526u)||w[023]!=(selector==212u?0052u:0u)||w[0163]!=0u)return P66_NATIVE_BAD_MODE;w[2]=(w[0116]==0u&&w[0135]!=AGC_MASK)?02314u:3u;return P66_NATIVE_OK;}
uint16_t p66_feedback_store_return_control(apollo_overlay_state *image){uint16_t *w;if(!image)return P66_NATIVE_BAD_MODE;w=image->words;if(w[5]!=06061u||w[0164]!=03444u||w[023]!=0u||w[0163]!=0u)return P66_NATIVE_BAD_MODE;w[2]=3;return P66_NATIVE_OK;}
