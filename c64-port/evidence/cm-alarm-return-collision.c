/* SPDX-License-Identifier: MIT; generated from retained audit tool. */
#include <stdlib.h>
#include "cm_alarm_complete_cases.h"
#include "cm_alarm_complete_dump.h"
int main(int argc,char **argv){uint32_t n;unsigned i;const uint32_t ids[2]={23683,23687};for(i=0;i<2u;++i){apollo_overlay_state s;cm_native_io io;enum exec_status status;n=ids[i];cm_alarm_complete_input(&s,&io,n);status=argc>1?cm_alarm_complete(&s,&io,CM_RECORD_BORTENT,0):cm_alarm_image(&s,&io,CM_RECORD_BORTENT,0);cm_alarm_complete_dump(n,status,&s,&io);}return 0;}
