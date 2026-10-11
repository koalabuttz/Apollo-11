/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_restart_check.h"
static unsigned errors;
#define CHECK(c) do { if(!(c)) { fprintf(stderr,"CM restart failure line%d: %s\n",__LINE__,#c);++errors; } } while(0)
int main(void) {
 cm_restart_case c;cm_executive before;uint8_t i;
 errors+=cm_rs_check(&c);
 cm_rs_prepare(&c,768);before=c.exec;CHECK(cm_rs_job(&c.exec,&c.phases,6)==EXEC_OUT_OF_SCOPE && memcmp(&before,&c.exec,sizeof before)==0);
 c.phases.phase[c.group]=1;CHECK(cm_rs_job(&c.exec,&c.phases,c.group)==EXEC_OUT_OF_SCOPE && memcmp(&before,&c.exec,sizeof before)==0);
 cm_rs_prepare(&c,768);c.exec.alarm=01202;before=c.exec;CHECK(cm_rs_job(&c.exec,&c.phases,c.group)==EXEC_BAILOUT_PENDING && memcmp(&before,&c.exec,sizeof before)==0);
 cm_rs_prepare(&c,768);c.exec.newjob=12;before=c.exec;CHECK(cm_rs_job(&c.exec,&c.phases,c.group)==EXEC_OUT_OF_SCOPE && memcmp(&before,&c.exec,sizeof before)==0);
 for(i=0;i<6;++i) { cm_rs_prepare(&c,512);c.phases.complement[i]^=1;CHECK(cm_rs_validate(&c.exec,&c.phases)==CM_RS_PHASE_BAD && c.exec.core[0][5]==i); }
 if(errors!=0) return 1;
 puts("PASS:1280 CM original cleanup/startsub/phase/variablejob states,18 complete vectors and fail-closed inputs");return 0;
}
