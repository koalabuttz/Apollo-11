/* SPDX-License-Identifier: MIT; input-only fixture helper. */
#include "service_routines_cases.h"
#include "service_routines_dump.h"
#include <stdlib.h>
int main(int argc,char **argv){int i;apollo_overlay_state s;p66_bias_io io;enum service_routines_mission mission;enum service_routines_entry entry;agc_word fixed[3];unsigned char b[6];for(i=1;i<argc;++i){uint32_t n=(uint32_t)strtoul(argv[i],0,0);unsigned j;service_routines_input(&s,&io,n,&mission,&entry,fixed);service_routines_dump(n,(uint16_t)service_routines_kind(n),0,&s,&io);for(j=0;j<3u;++j){b[2u*j]=(unsigned char)fixed[j];b[2u*j+1u]=(unsigned char)(fixed[j]>>8);}fwrite(b,1,6,stdout);}return ferror(stdout)?2:0;}
