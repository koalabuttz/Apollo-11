/* SPDX-License-Identifier: MIT */
#include "standard_lead_cm_cases.h"
#include "standard_lead_dump.h"
int main(void){uint16_t n;apollo_overlay_state s;cm_native_io io;enum exec_status status;
 for(n=0;n<STANDARD_LEAD_CM_CASES;++n){standard_lead_cm_input(&s,&io,n);status=standard_lead_cm_test(&s,&io,(enum standard_lead_cm_entry)(n/512u));if(status!=EXEC_OK&&status!=EXEC_NO_CORE)return 1;standard_lead_dump(n,(unsigned)status,&s);fprintf(stderr,"%u %u %u\n",n,io.superbank,io.interrupt_enabled);}return 0;}
