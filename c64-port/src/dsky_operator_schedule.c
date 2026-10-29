/* SPDX-License-Identifier: MIT */
#include "dsky_operator_schedule.h"
static int scheduler_domain(const uint16_t *w,exec_address request,uint8_t cm){uint16_t priority=(uint16_t)(w[01045]&077776u);uint8_t i,cores=cm ? 7u : 8u;if(request.location!=w[01046]||request.bank!=w[01047]||request.location>077777u||request.bank>077777u)return 0;
 if(w[5]==03466u){if(priority==0||(priority&0140777u)||w[0167]==0||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;for(i=1;i<cores;++i)if(w[0167u+12u*i]==0)return 0;for(i=0;i<5u;++i)if(w[0400u+i*44u]!=0&&w[0400u+i*44u]!=(uint16_t)(0400u+i*44u))return 0;return 1;}
 if(w[5]!=03512u||request.location==0||request.location>07777u||(request.bank&077777u)!=(request.bank&076167u)||w[026]>037777u)return 0;
 if(agc_classify(w[01045])==AGC_POSITIVE&&w[01045]>WL_MAX_DELAY)return 0;
 return 1;
}
enum p66_bias_status dsky_operator_schedule(apollo_overlay_state *s,dsky_charin_io *io,exec_address request,enum dsky_operator_schedule_route *route){enum dsky_request_control_route next;enum exec_status admitted;enum wl_status waiting;enum p66_bias_status status;if(route==0||!scheduler_domain(s->words,request,0))return P66_BIAS_OUT_OF_SCOPE;
 status=dsky_operator_request(s,io,&next);if(status!=P66_BIAS_OK)return status;
 if(next==DSKY_REQUEST_WAITLIST){waiting=lm_waitlist_image(s,&io->control,LM_WAITLIST_ENTRY,request);if(waiting==WL_BAD_DELAY){*route=DSKY_SCHEDULE_WAIT_BAD;return P66_BIAS_OK;}if(waiting==WL_FULL){*route=DSKY_SCHEDULE_WAIT_FULL;return P66_BIAS_OK;}if(waiting!=WL_OK)return P66_BIAS_OUT_OF_SCOPE;}
 else{admitted=lm_admission_image(s,&io->control,next==DSKY_REQUEST_FINDVAC ? LM_ADMIT_FINDVAC : LM_ADMIT_NOVAC,request);if(admitted==EXEC_NO_VAC||admitted==EXEC_NO_CORE){*route=DSKY_SCHEDULE_ADMISSION_ALARM;return P66_BIAS_OK;}if(admitted!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;}
 s->words[2]=0160;s->words[5]=05155;*route=DSKY_SCHEDULE_END;return P66_BIAS_OK;
}
enum exec_status cm_operator_schedule(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,exec_address request,enum dsky_operator_schedule_route *route){enum dsky_request_control_route next;enum exec_status status;enum cm_wl_status waiting;uint16_t event=0;if(route==0||wrote==0||!scheduler_domain(s->words,request,1)||(s->words[5]==03466u&&io->executive_alarm!=0)||(s->words[5]==03512u&&agc_classify(s->words[01045])==AGC_POSITIVE&&io->waitlist_alarm!=0))return EXEC_OUT_OF_SCOPE;
 status=cm_operator_request(s,io,&event,&next);if(status!=EXEC_OK)return status;
 if(next==DSKY_REQUEST_WAITLIST){waiting=cm_waitlist_extended(s,io,request,0);if(waiting==CM_WL_BAD_DELAY){*wrote=event;*route=DSKY_SCHEDULE_WAIT_BAD;return EXEC_OK;}if(waiting==CM_WL_FULL){*wrote=event;*route=DSKY_SCHEDULE_WAIT_FULL;return EXEC_OK;}if(waiting!=CM_WL_OK)return EXEC_OUT_OF_SCOPE;}
 else{status=cm_admission_complete(s,io,next==DSKY_REQUEST_FINDVAC ? CM_ADMIT_FINDVAC : CM_ADMIT_NOVAC,request);if(status==EXEC_NO_VAC||status==EXEC_NO_CORE){*wrote=event;*route=DSKY_SCHEDULE_ADMISSION_ALARM;return EXEC_OK;}if(status!=EXEC_OK)return EXEC_OUT_OF_SCOPE;}
 s->words[2]=0160;s->words[5]=05217;*wrote=event;*route=DSKY_SCHEDULE_END;return EXEC_OK;
}
