/* SPDX-License-Identifier: MIT */
/* Native side of the cm-alarm-sites differential. Links no rope reader. */
#include <stdlib.h>
#include "cm_alarm_sites_cases.h"
#include "cm_alarm_complete_dump.h"
int main(void){uint32_t n,cases=cm_alarm_sites_cases();for(n=0;n<cases;++n){apollo_overlay_state s;cm_native_io io;enum exec_status status;enum cm_alarm_sites_kind kind=cm_alarm_sites_input(&s,&io,n);
 if(kind==CM_SITES_ALARM2&&s.words[5]==03261u&&cm_alarm2_producer(&s,&io)!=EXEC_OK)return 2;
 status=cm_alarm_sites_run(&s,&io,kind);if(status!=EXEC_OK){fprintf(stderr,"cm-alarm-sites scope %lu\n",(unsigned long)n);return 2;}cm_alarm_complete_dump(n,(uint16_t)status,&s,&io);}
 return 0;}
