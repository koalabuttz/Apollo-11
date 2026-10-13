/* SPDX-License-Identifier: MIT */
/* Native AGC_BLOCK_TWO_SELF-CHECK cases in the oracle's format
 * (tools/self_check_reference.c). argv: the mission (0 Luminary099,
 * 1 Comanche055), its rope (build/fixed-data/<mission>/MAIN.agc.bin, for
 * the rope image) and "rope" for the whole-rope cases. */
#include <stdio.h>
#include <stdlib.h>
#include "self_check_cases.h"
#include "rope_image.h"
int main(int argc,char **argv){unsigned n,i,mission,rope,cases;
 if(argc<3)return 2;
 mission=(unsigned)atoi(argv[1])!=0u;rope=argc>3;cases=rope?SELF_CHECK_ROPE_CASES:SELF_CHECK_CASES;
 if(!apollo_rope_load(mission,argv[2])){fprintf(stderr,"cannot read %s\n",argv[2]);return 2;}
 for(n=0;n<cases;++n){apollo_overlay_state s;self_check_io io;uint16_t steps=0;enum exec_status st;
  st=rope?self_check_rope_run(mission,&s,&io,(uint16_t)n,0):self_check_run(mission,&s,&io,(uint16_t)n,&steps);
  if(st!=EXEC_OK){fprintf(stderr,"SELF-CHECK %u/%u stopped at %05o\n",mission,n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u",n,(unsigned)*self_check_interrupts(mission,&io),(unsigned)*self_check_superbank(mission,&io));
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
