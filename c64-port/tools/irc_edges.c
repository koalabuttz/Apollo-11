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
#ifdef IRC_SITES
/* Spec 'site_edges': a guard outside the milestone's own code (an
 * interpreter body, an image op, a shared helper) also counts under the last
 * milestone guard reached before it, the block that made the call: so each
 * call site's outcomes of the shared code are edges of their own
 * ("s<site>.<guard>"), as if every site had its own copy of the helpers. The
 * trampoline runs between routines, at no call site: it ends the site. The
 * code ranges ("start end kind" in hex, kind 1 the milestone's routines,
 * 2 the trampoline, 3 the milestone's own helpers, which count per call
 * site like the shared code; sorted) come from the file IRC_SITE_RANGES names, which
 * tools/interp_routine_differential.py writes from the link map. */
enum { NPAIR = 1u << 16, NRANGE = 4096 };
static uint64_t pair[NPAIR];static uint32_t used[NPAIR],nused,site;
static uintptr_t lo[NRANGE],hi[NRANGE];static uint8_t rkind[NRANGE];static unsigned nrange;static int ranges_read;
static void read_ranges(void){const char *f=getenv("IRC_SITE_RANGES");FILE *in;unsigned long a,b;unsigned k;
 ranges_read=1;if(!f||!(in=fopen(f,"r")))return;
 while(nrange<NRANGE&&fscanf(in,"%lx %lx %u",&a,&b,&k)==3){lo[nrange]=a;hi[nrange]=b;rkind[nrange++]=(uint8_t)k;}
 fclose(in);
}
static unsigned kind_of(uintptr_t pc){unsigned l=0,h=nrange;
 while(l<h){unsigned m=(l+h)/2u;if(pc<lo[m])h=m;else if(pc>=hi[m])l=m+1u;else return rkind[m];}
 return 0;
}
static void pair_add(uint32_t g){uint64_t k=(uint64_t)site<<32|g;uint32_t h=(uint32_t)((k*0x9e3779b97f4a7c15ull)>>48);
 while(pair[h]){if(pair[h]==k)return;h=(h+1u)&(NPAIR-1u);}
 if(nused>=NPAIR/2u)exit(4);
 pair[h]=k;used[nused++]=h;
}
#endif
void __sanitizer_cov_trace_pc_guard(uint32_t *guard){
 if(!*guard)return;
 hit[*guard]=1;if(!pcs[*guard])pcs[*guard]=(uintptr_t)__builtin_return_address(0);
#ifdef IRC_SITES
 {uintptr_t pc=(uintptr_t)__builtin_return_address(0);
  unsigned k;if(!ranges_read)read_ranges();k=kind_of(pc);
  if(k==1u)site=*guard;else if(k==2u)site=0;else if(site)pair_add(*guard);}
#endif
}
void irc_edges_reset(void){if(hit)memset(hit,0,nguards+1u);
#ifdef IRC_SITES
 {uint32_t i;for(i=0;i<nused;++i)pair[used[i]]=0;nused=0;site=0;}
#endif
}
void irc_edges_print(void){uint32_t i;for(i=1;i<=nguards;++i)if(hit[i])printf(" e%u",(unsigned)i);
#ifdef IRC_SITES
 for(i=0;i<nused;++i)printf(" s%u.%u",(unsigned)(pair[used[i]]>>32),(unsigned)(pair[used[i]]&0xffffffffu));
#endif
}
/* "pc <guard> <address>" for every guard some case reached. */
void irc_edges_map(void){uint32_t i;for(i=1;i<=nguards;++i)if(pcs[i])fprintf(stderr,"pc %u 0x%lx\n",(unsigned)i,(unsigned long)(pcs[i]-1u));}
