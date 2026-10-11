/* SPDX-License-Identifier: MIT */
#include "cm_restart_native_check.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
 cm_rn_run r,before;enum exec_status status;
 assert(cm_rn_check(&r)==0);
 cm_rn_init(&r,1);r.state.phases.address[0].location=02222;before=r;
 status=cm_restart_native_stage(&r.runtime,&r.state.record,&r.state.wait,&r.state.phases,0);
 assert(status==EXEC_UNMAPPED && memcmp(&r.state,&before.state,sizeof r.state)==0);
 cm_rn_init(&r,1);r.state.phases.phase[0]=AGC_NEG_ZERO;before=r;
 assert(cm_restart_native_stage(&r.runtime,&r.state.record,&r.state.wait,&r.state.phases,0)==EXEC_OUT_OF_SCOPE);
 assert(memcmp(&r.state,&before.state,sizeof r.state)==0);
 cm_rn_init(&r,1);r.runtime.in_callback=1;before=r;
 assert(cm_restart_native_stage(&r.runtime,&r.state.record,&r.state.wait,&r.state.phases,0)==EXEC_OUT_OF_SCOPE);
 assert(memcmp(&r.state,&before.state,sizeof r.state)==0);
 puts("PASS:64CMrestart sequences/320source checkpoints;192actualCcallbacks;unsupported maps/types/reentry retain state");return 0;
}
