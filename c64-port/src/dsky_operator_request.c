/* SPDX-License-Identifier: MIT */
#include "dsky_operator_request.h"
/* Pure branch-sensitive preflight for the immutable selected JOBWAKE body.
 * Prefix scratch writes cannot change this selected Executive domain. */
static int selected_wake(const uint16_t *w,uint8_t cm){uint16_t wake=w[01043];uint8_t i,cores=cm ? 7u : 8u,found=0;if(wake==0)return 1;
 if(w[0167]==0||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return 0;if(!found&&p>=040000u&&p<077777u&&(loc==wake||((loc==0||loc==077777u)&&(wake==0||wake==077777u)))){if(w[0165u+12u*i]&077610u)return 0;found=1;}}
 return 1;
}
enum p66_bias_status dsky_operator_request(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_request_control_route *route){enum dsky_request_control_route next;enum p66_bias_status status;if(route==0||(s->words[5]!=03466u&&s->words[5]!=03512u)||!selected_wake(s->words,0))return P66_BIAS_OUT_OF_SCOPE;
 status=dsky_request_control(s,io,&next);if(status!=P66_BIAS_OK)return status;
 status=dsky_display_release(s,io);if(status!=P66_BIAS_OK)return status;
 status=dsky_request_control(s,io,&next);if(status!=P66_BIAS_OK)return status;io->wrote_channel11=1;*route=next;return P66_BIAS_OK;
}
enum exec_status cm_operator_request(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,enum dsky_request_control_route *route){enum dsky_request_control_route next;enum exec_status status;uint16_t event=0;if(wrote==0||route==0||(s->words[5]!=03466u&&s->words[5]!=03512u)||!selected_wake(s->words,1)||(s->words[01043]!=0&&io->executive_alarm!=0))return EXEC_OUT_OF_SCOPE;
 status=cm_request_control(s,io,&event,&next);if(status!=EXEC_OK)return status;
 status=cm_display_release(s,io,&event);if(status!=EXEC_OK)return status;
 status=cm_request_control(s,io,&event,&next);if(status!=EXEC_OK)return status;*wrote=1;*route=next;return EXEC_OK;
}
