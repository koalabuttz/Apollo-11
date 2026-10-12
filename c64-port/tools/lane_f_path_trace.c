/* SPDX-License-Identifier: MIT */
/* Host-only: run every case of one Lane F module with its branch counting
 * on and print, per case, the case number, the outcome kind and the
 * branches the case took. tools/lane_f_c64_rows.py chooses the C64 rows
 * from this. Built with TRACE_HDR (the module's cases header), TRACE_PATHS
 * (its counter array), TRACE_NPATHS, TRACE_CASES and TRACE_RUN(s,io,n,steps). */
#include <stdio.h>
#include <string.h>
#include TRACE_HDR
#include "lm_flight_io.h"
unsigned TRACE_PATHS[TRACE_NPATHS];
int main(void){static unsigned before[TRACE_NPATHS];apollo_overlay_state s;lm_flight_io io;uint16_t steps;unsigned n,i,kind;
 (void)io;
 for(n=0;n<TRACE_CASES;++n){memcpy(before,TRACE_PATHS,sizeof before);
  kind=TRACE_RUN(&s,&io,(uint16_t)n,&steps);
  printf("%u %u",n,kind);
  for(i=0;i<TRACE_NPATHS;++i)if(TRACE_PATHS[i]!=before[i])printf(" %u",i);
  putchar('\n');}
 return 0;
}
