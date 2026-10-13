/* SPDX-License-Identifier: MIT */
/* C64 row selection for imu-mode-switching (tools/imu_mode_switching_cover.py).
 *
 * select: built with -fsanitize-coverage=trace-pc on every linked native source.
 *   Runs every case; a row's signature is its mission, the basic-block edges
 *   it takes in that mission, the routine (entry case) or call site (round
 *   trip) it starts at, and its outcome kind in that mission. Greedily picks
 *   cases until the rows take every item the host stream takes, and prints
 *   them, one case number per line; the items no row takes go to stderr.
 * replay MISSION FILE: built with --coverage. Runs the cases of FILE (or,
 *   with FILE "-", every case) whose mission is MISSION (0 Luminary,
 *   1 Comanche) for gcov. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imu_mode_switching_cases.h"

#define EDGES 262144u
#define ITEMS (2u*EDGES+IMS_ROUTINES+IMS_SITES+8u)
static uintptr_t edge_a[EDGES],edge_b[EDGES];static uint8_t edge_used[EDGES];
static uintptr_t prev_pc;static int tracing;
static uint32_t hits[32768];static unsigned nhits;static uint8_t seen[EDGES];
static unsigned edge_id(uintptr_t a,uintptr_t b){unsigned h=(unsigned)((a*0x9e3779b97f4a7c15ull^b*0xc2b2ae3d27d4eb4full)>>40)%EDGES;
 while(edge_used[h]&&(edge_a[h]!=a||edge_b[h]!=b))h=(h+1u)%EDGES;
 if(!edge_used[h]){edge_used[h]=1;edge_a[h]=a;edge_b[h]=b;}
 return h;
}
void __sanitizer_cov_trace_pc(void);
void __sanitizer_cov_trace_pc(void){uintptr_t pc=(uintptr_t)__builtin_return_address(0);unsigned id;
 if(!tracing)return;
 id=edge_id(prev_pc,pc);prev_pc=pc;
 if(!seen[id]&&nhits<32000u){seen[id]=1;hits[nhits++]=id;}
}
/* Runs case n; returns its mission, or 2 for a case with no site. */
static unsigned run(uint16_t n,unsigned *kind){apollo_overlay_state s;uint16_t ie;agc_word ch[4];unsigned i;uint8_t cm;
 for(i=0;i<nhits;++i)if(hits[i]<EDGES)seen[hits[i]]=0;
 nhits=0;prev_pc=0;
 if(ims_target(n,&cm)==0xffffu||cm>1u)return 2u;
 tracing=1;*kind=ims_run(&s,n,&ie,ch);tracing=0;
 return cm;
}
static int cmp32(const void *a,const void *b){uint32_t x=*(const uint32_t *)a,y=*(const uint32_t *)b;return x<y?-1:x>y;}
/* Item ids: edge e of mission cm is cm*EDGES+e; routine r is 2*EDGES+r;
 * site k is 2*EDGES+IMS_ROUTINES+k; kind k of mission cm is
 * 2*EDGES+IMS_ROUTINES+IMS_SITES+4*cm+k. */
static void items(uint16_t n,unsigned cm,unsigned kind){unsigned i;
 for(i=0;i<nhits;++i){seen[hits[i]]=0;hits[i]+=cm*EDGES;}
 hits[nhits++]=n<IMS_ENTRY_CASES?2u*EDGES+n%IMS_ROUTINES:2u*EDGES+IMS_ROUTINES+(n-IMS_TRIP_FIRST)%IMS_SITES;
 hits[nhits++]=2u*EDGES+IMS_ROUTINES+IMS_SITES+4u*cm+(kind&3u);
}

typedef struct {uint64_t hash;uint32_t *ids;unsigned count;uint16_t n;} sig;
static sig *sigs;static unsigned nsigs,capsigs;
#define HT 1048576u
static int32_t ht[HT];
static void add_row(uint16_t n){uint64_t h=1469598103934665603ull;unsigned i,slot;
 qsort(hits,nhits,sizeof *hits,cmp32);
 for(i=0;i<nhits;++i)h=(h^hits[i])*1099511628211ull;
 slot=(unsigned)(h%HT);
 while(ht[slot]>=0){sig *g=&sigs[ht[slot]];if(g->hash==h&&g->count==nhits&&!memcmp(g->ids,hits,nhits*sizeof *hits))return;slot=(slot+1u)%HT;}
 if(nsigs==capsigs){capsigs=capsigs?capsigs*2u:4096u;sigs=realloc(sigs,capsigs*sizeof *sigs);if(!sigs)exit(2);}
 sigs[nsigs].hash=h;sigs[nsigs].count=nhits;sigs[nsigs].n=n;
 sigs[nsigs].ids=malloc(nhits*sizeof *hits);if(!sigs[nsigs].ids)exit(2);
 memcpy(sigs[nsigs].ids,hits,nhits*sizeof *hits);ht[slot]=(int32_t)nsigs++;
}
static uint8_t need[ITEMS];
static int do_select(void){unsigned n,i,left=0,picked=0,cm,kind;
 memset(ht,-1,sizeof ht);
 for(n=0;n<IMS_CASES;++n){cm=run((uint16_t)n,&kind);if(cm>1u)continue;items((uint16_t)n,cm,kind);
  for(i=0;i<nhits;++i)need[hits[i]]=1;
  add_row((uint16_t)n);}
 for(i=0;i<ITEMS;++i)left+=need[i];
 for(;;){unsigned best=0,gain=0,g;
  for(i=0;i<nsigs;++i){unsigned j;g=0;for(j=0;j<sigs[i].count;++j)g+=need[sigs[i].ids[j]];
   if(g>gain||(g==gain&&g&&sigs[i].n<sigs[best].n)){gain=g;best=i;}}
  if(!gain)break;
  for(i=0;i<sigs[best].count;++i){left-=need[sigs[best].ids[i]];need[sigs[best].ids[i]]=0;}
  printf("%u\n",sigs[best].n);++picked;
 }
 fprintf(stderr,"%u rows, %u signatures, %u items left\n",picked,nsigs,left);
 return 0;
}
static int do_replay(unsigned mission,const char *file){unsigned n,kind;FILE *f;
 if(!strcmp(file,"-")){for(n=0;n<IMS_CASES;++n){uint8_t cm;if(ims_target((uint16_t)n,&cm)!=0xffffu&&cm==mission)run((uint16_t)n,&kind);}return 0;}
 f=fopen(file,"r");if(!f)return 2;
 while(fscanf(f,"%u",&n)==1){uint8_t cm;if(ims_target((uint16_t)n,&cm)!=0xffffu&&cm==mission)run((uint16_t)n,&kind);}
 fclose(f);return 0;
}
int main(int argc,char **argv){
 if(argc==2&&!strcmp(argv[1],"select"))return do_select();
 if(argc==4&&!strcmp(argv[1],"replay"))return do_replay((unsigned)atoi(argv[2]),argv[3]);
 return 2;
}
