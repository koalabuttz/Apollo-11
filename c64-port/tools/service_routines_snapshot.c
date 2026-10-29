/* SPDX-License-Identifier: MIT */
#include "service_routines_cases.h"
#include "service_routines_dump.h"
#include <stdlib.h>
int main(int argc,char **argv){uint32_t n,start=argc>1?(uint32_t)strtoul(argv[1],0,0):0,end=argc>2?(uint32_t)strtoul(argv[2],0,0):SERVICE_ROUTINES_CASES;apollo_overlay_state s;p66_bias_io io;enum service_routines_mission mission;enum service_routines_entry entry;agc_word fixed[3];
 for(n=start;n<end;++n){service_routines_input(&s,&io,n,&mission,&entry,fixed);if(service_routines_kind(n)>=24u){uint16_t iterations=0;service_routines_dump(n,(uint16_t)service_routines_kind(n),(uint16_t)(service_routines_block(&s,&io,mission,fixed[0],fixed[1],0,0,1024,&iterations)!=SERVICE_BLOCK_DONE),&s,&io);}else service_routines_dump(n,(uint16_t)service_routines_kind(n),(uint16_t)!service_routines_step(&s,&io,mission,entry,fixed[0],fixed[1],fixed[2]),&s,&io);}return ferror(stdout)?2:0;}
