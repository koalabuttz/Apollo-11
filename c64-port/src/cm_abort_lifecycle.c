/* SPDX-License-Identifier: MIT */
#include "cm_abort_lifecycle.h"
bool cm_abort_lifecycle_step(apollo_overlay_state *s,cm_native_io *io,uint8_t group,agc_word literal,uint8_t *in_isr,uint16_t *result){enum exec_status status;enum cm_enema_result recovery;
 if(!s||!io||!in_isr||!result||group>13u||*in_isr>1u)return false;
 if(group<=4u)status=cm_abort_snapshot_stage(s,io,group,literal);
 else if(group<=11u)status=cm_abort_recovery_stage(s,io,(uint8_t)(group-5u));
 else if(group==12u)status=cm_abort_whimper(s,io,in_isr);
 else{recovery=cm_enema_image(s,io);*result=(uint16_t)recovery;return recovery!=CM_ENEMA_OUT_OF_SCOPE;}
 *result=(uint16_t)status;return status==EXEC_OK;
}
