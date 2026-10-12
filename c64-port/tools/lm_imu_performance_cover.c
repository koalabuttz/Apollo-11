/* SPDX-License-Identifier: MIT */
/* C64 row selection for lm-imu-performance (tools/lm_imu_performance_cover.py).
 * Linked with src/lm_imu_performance.c built with -DIPT_C64_PART=0 (every
 * part's chained steps replayed through ipt_part).
 *
 * select: built with -fsanitize-coverage=trace-pc on src/lm_imu_performance.c.
 *   Runs every case as the host stream (part 0) and as each C64 part; a
 *   row's signature is the set of basic-block edges it takes and its
 *   routine key. Greedily picks rows of parts 1..IPT_PARTS (none of kind 3)
 *   until they take every edge the host stream takes and start at every
 *   key, and prints them as "part case" lines; the edges no part row takes
 *   go to stderr.
 * replay FILE: built with --coverage. Runs the "part case" rows of FILE
 *   (or, with FILE "-", every case as the host stream) for gcov. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lm_imu_performance_cases.h"

#define EDGES 65536u
static uintptr_t edge_a[EDGES],edge_b[EDGES];static uint8_t edge_used[EDGES];
static uintptr_t prev_pc;static int tracing;
static uint32_t hits[8192];static unsigned nhits;static uint8_t seen[EDGES];
static unsigned edge_id(uintptr_t a,uintptr_t b){unsigned h=(unsigned)((a*0x9e3779b97f4a7c15ull^b*0xc2b2ae3d27d4eb4full)>>48)%EDGES;
 while(edge_used[h]&&(edge_a[h]!=a||edge_b[h]!=b))h=(h+1u)%EDGES;
 if(!edge_used[h]){edge_used[h]=1;edge_a[h]=a;edge_b[h]=b;}
 return h;
}
void __sanitizer_cov_trace_pc(void);
void __sanitizer_cov_trace_pc(void){uintptr_t pc=(uintptr_t)__builtin_return_address(0);unsigned id;
 if(!tracing)return;
 id=edge_id(prev_pc,pc);prev_pc=pc;
 if(!seen[id]&&nhits<8192u){seen[id]=1;hits[nhits++]=id;}
}
static unsigned run(unsigned part,uint16_t n){apollo_overlay_state s;uint16_t ie;unsigned k,i;
 for(i=0;i<nhits;++i)seen[hits[i]]=0;
 nhits=0;prev_pc=0;ipt_part=part;tracing=1;k=ipt_run(&s,n,&ie);tracing=0;return k;
}
static int cmp32(const void *a,const void *b){uint32_t x=*(const uint32_t *)a,y=*(const uint32_t *)b;return x<y?-1:x>y;}

/* Unique row signatures: sorted edge ids plus the routine key (EDGES+r). */
typedef struct {uint64_t hash;uint32_t *ids;unsigned count;unsigned part,n;} sig;
static sig *sigs;static unsigned nsigs,capsigs;
#define HT 1048576u
static int32_t ht[HT];
static void add_row(unsigned part,unsigned n){uint64_t h=1469598103934665603ull;unsigned i,slot;
 hits[nhits++]=EDGES+n%IPT_ROUTINES;
 qsort(hits,nhits,sizeof *hits,cmp32);
 for(i=0;i<nhits;++i)h=(h^hits[i])*1099511628211ull;
 slot=(unsigned)(h%HT);
 while(ht[slot]>=0){sig *g=&sigs[ht[slot]];if(g->hash==h&&g->count==nhits&&!memcmp(g->ids,hits,nhits*sizeof *hits)){--nhits;return;}slot=(slot+1u)%HT;}
 if(nsigs==capsigs){capsigs=capsigs?capsigs*2u:4096u;sigs=realloc(sigs,capsigs*sizeof *sigs);if(!sigs)exit(2);}
 sigs[nsigs].hash=h;sigs[nsigs].count=nhits;sigs[nsigs].part=part;sigs[nsigs].n=n;
 sigs[nsigs].ids=malloc(nhits*sizeof *hits);if(!sigs[nsigs].ids)exit(2);
 memcpy(sigs[nsigs].ids,hits,nhits*sizeof *hits);ht[slot]=(int32_t)nsigs++;
 --nhits;
}
static uint8_t need[EDGES+IPT_ROUTINES];
static int do_select(void){unsigned n,p,i,left=0,picked=0;
 memset(ht,-1,sizeof ht);
 for(n=0;n<IPT_CASES;++n){run(0,(uint16_t)n);for(i=0;i<nhits;++i)need[hits[i]]=1;}
 for(i=0;i<IPT_ROUTINES;++i)need[EDGES+i]=1;
 for(i=0;i<EDGES+IPT_ROUTINES;++i)left+=need[i];
 for(p=1;p<=IPT_PARTS;++p)for(n=0;n<IPT_CASES;++n){if(run(p,(uint16_t)n)==3u)continue;add_row(p,n);}
 for(;;){unsigned best=0,gain=0,g;
  for(i=0;i<nsigs;++i){unsigned j;g=0;for(j=0;j<sigs[i].count;++j)g+=need[sigs[i].ids[j]];
   if(g>gain||(g==gain&&g&&(sigs[i].part<sigs[best].part||(sigs[i].part==sigs[best].part&&sigs[i].n<sigs[best].n)))){gain=g;best=i;}}
  if(!gain)break;
  for(i=0;i<sigs[best].count;++i){left-=need[sigs[best].ids[i]];need[sigs[best].ids[i]]=0;}
  printf("%u %u\n",sigs[best].part,sigs[best].n);++picked;
 }
 for(i=0;i<EDGES+IPT_ROUTINES;++i)if(need[i]){if(i>=EDGES)fprintf(stderr,"key %u not run by any part\n",i-EDGES);else fprintf(stderr,"edge %#lx %#lx\n",(unsigned long)edge_a[i],(unsigned long)edge_b[i]);}
 fprintf(stderr,"%u rows, %u signatures, %u items left\n",picked,nsigs,left);
 return 0;
}
static int do_replay(const char *file){unsigned p,n;FILE *f;
 if(!strcmp(file,"-")){for(n=0;n<IPT_CASES;++n)run(0,(uint16_t)n);return 0;}
 f=fopen(file,"r");if(!f)return 2;
 while(fscanf(f,"%u %u",&p,&n)==2)if(run(p,(uint16_t)n)==3u){fprintf(stderr,"part %u case %u is kind 3\n",p,n);return 1;}
 fclose(f);return 0;
}
int main(int argc,char **argv){
 if(argc==2&&!strcmp(argv[1],"select"))return do_select();
 if(argc==3&&!strcmp(argv[1],"replay"))return do_replay(argv[2]);
 return 2;
}
