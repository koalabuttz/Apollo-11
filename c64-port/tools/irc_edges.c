/* SPDX-License-Identifier: MIT */
/* Host-only edge recorder for tools/interp_routine_differential.py: the
 * sources under test are built with -fsanitize-coverage=trace-pc-guard, so
 * every branch outcome is an edge with a guard. The runner resets the hit
 * set before each case and prints the guards the case reached; the guard's
 * first call site maps it to a source line (llvm-symbolizer). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint32_t nguards;static uint8_t *hit;static uintptr_t *pcs;
void __sanitizer_cov_trace_pc_guard_init(uint32_t *start,uint32_t *stop);
void __sanitizer_cov_trace_pc_guard(uint32_t *guard);
void __sanitizer_cov_trace_pc_guard_init(uint32_t *start,uint32_t *stop){uint32_t *x,old=nguards;
 if(start==stop||*start)return;
 for(x=start;x<stop;++x)*x=++nguards;
 hit=realloc(hit,nguards+1u);pcs=realloc(pcs,(nguards+1u)*sizeof *pcs);
 if(!hit||!pcs)exit(3);
 memset(hit+old+1,0,nguards-old);memset(pcs+old+1,0,(nguards-old)*sizeof *pcs);
}
void __sanitizer_cov_trace_pc_guard(uint32_t *guard){
 if(!*guard)return;
 hit[*guard]=1;if(!pcs[*guard])pcs[*guard]=(uintptr_t)__builtin_return_address(0);
}
void irc_edges_reset(void){if(hit)memset(hit,0,nguards+1u);}
void irc_edges_print(void){uint32_t i;for(i=1;i<=nguards;++i)if(hit[i])printf(" e%u",(unsigned)i);}
/* "pc <guard> <address>" for every guard some case reached. */
void irc_edges_map(void){uint32_t i;for(i=1;i<=nguards;++i)if(pcs[i])fprintf(stderr,"pc %u 0x%lx\n",(unsigned)i,(unsigned long)(pcs[i]-1u));}
