/* SPDX-License-Identifier: MIT */
/* Host-only: run every case of one Lane F module with its branch counting
 * on and print, per case, the case number, the outcome kind and the
 * edges the case took. tools/lane_f_c64_rows.py chooses the C64 rows
 * from this. Built with TRACE_HDR (the module's cases header), TRACE_PATHS
 * (its counter array), TRACE_NPATHS, TRACE_CASES and TRACE_RUN(s,io,n,steps). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include TRACE_HDR
#include "lm_flight_io.h"
unsigned TRACE_PATHS[TRACE_NPATHS];
/* Every linked source of the module is built with clang
 * -fsanitize-coverage=edge,trace-pc-guard: one guard per control-flow edge
 * (critical edges split, so each branch outcome and each switch arm has its
 * own), printed as TRACE_NPATHS + guard - 1, so the C64 rows must take every
 * edge the host cases take in any linked source, counter or not. */
static unsigned char *edge_hit;static unsigned edges;
void __sanitizer_cov_trace_pc_guard_init(uint32_t *start,uint32_t *stop);
void __sanitizer_cov_trace_pc_guard(uint32_t *guard);
void __sanitizer_cov_trace_pc_guard_init(uint32_t *start,uint32_t *stop){uint32_t *g;
 if(start==stop||*start)return;
 for(g=start;g<stop;++g)*g=++edges;
 edge_hit=realloc(edge_hit,edges+1u);if(!edge_hit)abort();memset(edge_hit,0,edges+1u);}
void __sanitizer_cov_trace_pc_guard(uint32_t *guard){if(*guard)edge_hit[*guard]=1;}
/* An argument runs that one case only (tools/c64_row_cover.py). */
int main(int argc,char **argv){static unsigned before[TRACE_NPATHS];apollo_overlay_state s;lm_flight_io io;uint16_t steps;unsigned n,i,kind,last=TRACE_CASES;
 (void)io;n=0;if(argc>1){n=(unsigned)atoi(argv[1]);last=n+1u;}
 for(;n<last;++n){memcpy(before,TRACE_PATHS,sizeof before);
  kind=TRACE_RUN(&s,&io,(uint16_t)n,&steps);
  printf("%u %u",n,kind);
  for(i=0;i<TRACE_NPATHS;++i)if(TRACE_PATHS[i]!=before[i])printf(" %u",i);
  for(i=1;i<=edges;++i)if(edge_hit[i]){printf(" %u",TRACE_NPATHS+i-1u);edge_hit[i]=0;}
  putchar('\n');}
 return 0;
}
