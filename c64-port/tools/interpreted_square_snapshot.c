/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "interpreted_square_cases.h"
int main(int argc,char **argv){uint32_t n,limit=INTERPRETED_SQUARE_CASES;enum interpreted_square_mission mission;if(argc<2)return 2;mission=(enum interpreted_square_mission)strtoul(argv[1],NULL,0);if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;uint16_t row[2051];unsigned i;interpreted_square_input(&m,&irq,mission,n);if(interpreted_square(&m,mission,(enum interpreted_square_kind)(n>>16)))return 2;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=irq;for(i=0;i<2048u;++i)row[i+3u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
