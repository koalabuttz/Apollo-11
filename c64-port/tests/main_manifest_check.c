/* SPDX-License-Identifier: MIT */
/* main_manifest_check (include/main_manifest.h), on the host and the C64. */
#include "main_manifest.h"
#include "rope_image.h"
static uint8_t emitted[APOLLO_ROPE_WORDS/8u];
unsigned main_manifest_check(unsigned mission,uint16_t *checks){
 const main_manifest *m=&main_manifests[mission];unsigned failures=0;uint16_t f,r,i,n=0;agc_word w;
 for(i=0;i<sizeof emitted;++i)emitted[i]=0;
 for(f=0;f<m->file_count;++f){const main_manifest_file *e=&m->files[f];uint32_t sum=0;uint16_t words=0;
  for(r=e->first_run;r<e->first_run+e->runs;++r){const main_manifest_run *run=&m->runs[r];
   for(i=0;i<run->count;++i){uint16_t p=(uint16_t)(run->start+i);
    if(!apollo_rope_read(mission,p,&w)){++failures;continue;}
    if(emitted[p>>3]&(uint8_t)(1u<<(p&7u)))++failures;           /* emitted twice */
    emitted[p>>3]|=(uint8_t)(1u<<(p&7u));sum+=w;++words;}}
  ++*checks;if(words!=e->words||sum!=e->sum)++failures;
  n=(uint16_t)(n+words);}
 ++*checks;if(n!=m->emitted)++failures;
 /* Every word no file emits: unused (+0 or TC to itself) or a bank checksum. */
 ++*checks;
 for(i=0;i<APOLLO_ROPE_WORDS;++i){uint16_t bank=(uint16_t)(i/1024u),k;agc_word self;
  if(emitted[i>>3]&(uint8_t)(1u<<(i&7u)))continue;
  if(!apollo_rope_read(mission,i,&w)){++failures;continue;}
  self=(agc_word)(bank==2u||bank==3u?i:02000u+i%1024u);
  if(w==0u||w==self)continue;
  for(k=0;k<36u&&m->checksum_at[k]!=i;++k){}
  if(k==36u)++failures;}
 return failures;
}
