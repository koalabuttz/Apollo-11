/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include <string.h>
#include "edge_cover.h"
static uint8_t *hit;static uint32_t *list;static unsigned edges,listed;
void __sanitizer_cov_trace_pc_guard_init(uint32_t *start,uint32_t *stop);
void __sanitizer_cov_trace_pc_guard(uint32_t *guard);
void __sanitizer_cov_trace_pc_guard_init(uint32_t *start,uint32_t *stop){
 uint32_t *g;
 if(start==stop||*start)return;
 for(g=start;g<stop;++g)*g=++edges;
 hit=realloc(hit,edges+1u);list=realloc(list,(edges+1u)*sizeof *list);
 if(!hit||!list)abort();
 memset(hit,0,edges+1u);
}
void __sanitizer_cov_trace_pc_guard(uint32_t *guard){
 if(*guard&&!hit[*guard]){hit[*guard]=1;list[listed++]=*guard;}
}
unsigned edge_cover_edges(void){return edges;}
/* Open-addressing set of 64-bit signatures of the edge sets already printed. */
static uint64_t *seen;static size_t cap,used;
static int insert(uint64_t h){
 size_t i;
 if(!h)h=1;
 if(2u*(used+1u)>cap){
  uint64_t *old=seen;size_t oc=cap,j;
  cap=cap?2u*cap:1u<<16;seen=calloc(cap,sizeof *seen);if(!seen)abort();used=0;
  for(j=0;j<oc;++j)if(old[j])(void)insert(old[j]);
  free(old);
 }
 for(i=(size_t)(h%cap);seen[i];i=(i+1u)%cap)if(seen[i]==h)return 0;
 seen[i]=h;++used;return 1;
}
static int cmp(const void *a,const void *b){uint32_t x=*(const uint32_t *)a,y=*(const uint32_t *)b;return x<y?-1:x>y;}
void edge_cover_mark(FILE *out,const char *tag){
 unsigned i;uint64_t h=1469598103934665603ull;
 qsort(list,listed,sizeof *list,cmp);
 for(i=0;i<listed;++i){h^=list[i];h*=1099511628211ull;hit[list[i]]=0;}
 if(listed&&insert(h)){
  fprintf(out,"%s |",tag);
  for(i=0;i<listed;++i)fprintf(out," %u",(unsigned)list[i]);
  fputc('\n',out);
 }
 listed=0;
}
