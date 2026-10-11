/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "interpreted_square_cases.h"
int main(int argc,char **argv){uint32_t n;enum interpreted_square_mission mission;if(argc<2)return 2;mission=(enum interpreted_square_mission)strtoul(argv[1],NULL,0);while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;agc_word irq;interpreted_square_input(&m,&irq,mission,n);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
