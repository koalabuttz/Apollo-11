/* SPDX-License-Identifier: MIT */
/* Branch-outcome check of the imu-compensation C64 rows
 * (tools/imu_compensation_cover.py). Built with --coverage.
 * replay MISSION FILE: runs the cases of FILE (or, with FILE "-", every
 *   case) whose mission is MISSION (0 Luminary, 1 Comanche) for gcov. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imu_compensation_cases.h"
static apollo_overlay_state image;
static void one(unsigned mission,unsigned n){icp_case c;uint16_t ie;
 icp_input(&image,&c,(uint16_t)n);
 if(c.cm==mission)(void)icp_run(&image,(uint16_t)n,&ie);
}
int main(int argc,char **argv){unsigned n,mission;FILE *f;
 if(argc!=4||strcmp(argv[1],"replay"))return 2;
 mission=(unsigned)atoi(argv[2]);
 if(!strcmp(argv[3],"-")){for(n=0;n<ICP_CASES;++n)one(mission,n);return 0;}
 f=fopen(argv[3],"r");if(!f)return 2;
 while(fscanf(f,"%u",&n)==1)one(mission,n);
 fclose(f);return 0;
}
