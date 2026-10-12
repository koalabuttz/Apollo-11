/* SPDX-License-Identifier: MIT */
/* Host-only: run every case of one Lane F module with its branch counting
 * on and print, per case, the case number, the outcome kind and the
 * branches the case took. tools/lane_f_c64_rows.py chooses the C64 rows
 * from this. Built with TRACE_HDR (the module's cases header), TRACE_PATHS
 * (its counter array), TRACE_NPATHS, TRACE_CASES and TRACE_RUN(s,io,n,steps). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include TRACE_HDR
#include "lm_flight_io.h"
unsigned TRACE_PATHS[TRACE_NPATHS];
/* The module's own source is built with -fsanitize-coverage=trace-pc: each
 * basic block it enters calls this with its return address, numbered in
 * order of first entry and printed as TRACE_NPATHS + its number, so the C64
 * rows must reach every block the host cases reach, counter or not. */
#define BLOCKS 16384u
static uintptr_t block_pc[BLOCKS];
static unsigned char block_hit[BLOCKS];
static unsigned blocks;
void __sanitizer_cov_trace_pc(void);
void __sanitizer_cov_trace_pc(void){uintptr_t pc=(uintptr_t)__builtin_return_address(0);unsigned h=(unsigned)(pc*2654435761u)%BLOCKS;
 while(block_pc[h]&&block_pc[h]!=pc)h=(h+1u)%BLOCKS;
 if(!block_pc[h]){block_pc[h]=pc;++blocks;}
 block_hit[h]=1;}
static unsigned block_id[BLOCKS],next_id;
/* An argument runs that one case only (tools/c64_row_cover.py). */
int main(int argc,char **argv){static unsigned before[TRACE_NPATHS];apollo_overlay_state s;lm_flight_io io;uint16_t steps;unsigned n,i,kind,last=TRACE_CASES;
 (void)io;n=0;if(argc>1){n=(unsigned)atoi(argv[1]);last=n+1u;}
 for(;n<last;++n){memcpy(before,TRACE_PATHS,sizeof before);
  kind=TRACE_RUN(&s,&io,(uint16_t)n,&steps);
  printf("%u %u",n,kind);
  for(i=0;i<TRACE_NPATHS;++i)if(TRACE_PATHS[i]!=before[i])printf(" %u",i);
  for(i=0;i<BLOCKS;++i)if(block_hit[i]){if(!block_id[i])block_id[i]=++next_id;printf(" %u",TRACE_NPATHS+block_id[i]-1u);block_hit[i]=0;}
  putchar('\n');}
 if(blocks>BLOCKS/2u){fprintf(stderr,"too many blocks\n");return 1;}
 return 0;
}
