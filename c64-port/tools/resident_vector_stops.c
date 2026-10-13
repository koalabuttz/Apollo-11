/* SPDX-License-Identifier: MIT */
/* For tools/resident_rupt_vectors.py: each other-vector case's native run
 * (tests/resident_vector_run.h) as "n kind point", which the yaAGC
 * reference runs the original to.
 *   resident-vector-stops ROPE.bin   (build with -DRK_LM=1 or -DRK_CM=1) */
#include <stdio.h>
#include "resident_vector_run.h"
#if RK_LM
#define MISSION 0u
#else
#define MISSION 1u
#endif
static agc_word rope_words[36u*1024u];
static bool fixed(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;
 if(physical>=36u*1024u)return false;
 *word=rope_words[physical];return true;}
int main(int argc,char **argv){FILE *f;unsigned i,bank;unsigned char b[2];uint16_t n;
 if(argc!=2||!(f=fopen(argv[1],"rb")))return 2;
 for(i=0;i<36u*1024u;++i){if(fread(b,1,2,f)!=2u)return 2;
  bank=i/1024u;bank=bank<2u?bank+2u:bank<4u?bank-2u:bank;    /* yaYUL order 2, 3, 0, 1, 4, ... */
  rope_words[bank*1024u+i%1024u]=(agc_word)((((unsigned)b[0]<<8)|b[1])>>1);}
 fclose(f);
 for(n=0;n<RR_VECTOR_CASES;++n){static rr_vcase c;static apollo_overlay_state image;uint16_t point,routines;uint8_t kind;
  c.s=&image;rr_vector_input(MISSION,n,&c);
  kind=rr_vector_run(MISSION,&c,fixed,&point,&routines);
  printf("%u %u %u\n",(unsigned)n,kind,point);}
 return 0;}
