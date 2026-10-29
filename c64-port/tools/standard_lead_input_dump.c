/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "standard_lead_target.h"
#include "standard_lead_dump.h"
int main(int argc,char **argv){unsigned n,count,kind;enum standard_lead_mission mission;apollo_overlay_state s;p66_bias_io io;
 if(argc!=4)return 2;
 mission=(enum standard_lead_mission)atoi(argv[1]);kind=(unsigned)atoi(argv[2]);count=(unsigned)atoi(argv[3]);
 for(n=0;n<count;++n){standard_lead_target_input(&s,&io,mission,(uint16_t)kind,(uint16_t)n);standard_lead_dump(n,0,&s);}return 0;}
