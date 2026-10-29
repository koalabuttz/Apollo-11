/* SPDX-License-Identifier: MIT */
#include "p66_native_check.h"
#include "p66_native_vectors.h"
static uint32_t image_hash(const apollo_overlay_state *m){
 uint32_t h=UINT32_C(2166136261);unsigned i;
 for(i=0;i<APOLLO_SHARED_WORDS;++i){h=(h^(uint8_t)m->words[i])*UINT32_C(16777619);h=(h^(uint8_t)(m->words[i]>>8))*UINT32_C(16777619);}return h;
}
typedef struct {unsigned failures;uint8_t row,stage;} check;
static void observe(void *user,const apollo_overlay_state*m,agc_word loc){
 check*c=user;uint16_t w[PN_WORDS];uint32_t h=UINT32_C(2166136261);unsigned i;
 static const agc_word ends[5]={03353,03356,03361,03364,03373};
 pn_words(m,m->words[0120],w);
 for(i=0;i<PN_WORDS;++i){h=(h^(uint8_t)w[i])*UINT32_C(16777619);h=(h^(uint8_t)(w[i]>>8))*UINT32_C(16777619);}
 if(c->stage>=5 || loc!=ends[c->stage] || h!=pn_fingerprints[c->row][c->stage])++c->failures;
 ++c->stage;
}
static uint16_t entry(apollo_overlay_state *m,uint16_t selector,
 uint16_t (*invoke)(void *,apollo_overlay_state *,uint16_t),void *modules) {
 if(invoke!=0)return invoke(modules,m,selector);
#ifndef P66_NATIVE_DISPATCH_ONLY
 return p66_native_entry(m,selector);
#else
 return P66_NATIVE_UNSUPPORTED;
#endif
}
unsigned pn_check(apollo_overlay_state*m,executive*s,exec_native*r,velocity_case*input,const exec_exclusion*guard,uint16_t (*invoke)(void *,apollo_overlay_state *,uint16_t),void *modules){
 check c={0,0,0};p66_native_binding binding={m,r,guard,observe,&c,P66_NATIVE_OK,invoke,modules};unsigned i,j;
 for(i=0;i<PN_TARGET_CASES;++i){
  c.row=(uint8_t)i;c.stage=0;pn_prepare(m,s,input,pn_target_numbers[i]);
  if(exec_native_context_init(r,s,p66_native_mappings,P66_NATIVE_MAPPING_COUNT,&binding)!=EXEC_OK)++c.failures;
  r->running=1;r->current.location=03275;r->current.bank=i&1u?064067:064007;
  for(j=0;j<5;++j)if(p66_native_step(&binding)!=EXEC_OK)++c.failures;
  if(p66_native_step(&binding)!=EXEC_OUT_OF_SCOPE || binding.status!=P66_NATIVE_BOUNDARY || c.stage!=5)++c.failures;
 }
 /* Reject invalid selector/ownership before any payload write. */
 pn_prepare(m,s,input,0);{
  uint32_t before_hash=image_hash(m);uint16_t before[PN_WORDS],after[PN_WORDS];agc_word base=m->words[0120];pn_words(m,base,before);
  if(entry(m,99,invoke,modules)!=P66_NATIVE_UNSUPPORTED)++c.failures;
  m->words[base-1]=1;if(entry(m,1,invoke,modules)!=P66_NATIVE_BAD_VAC)++c.failures;m->words[base-1]=0;
  m->words[0163]=123;if(entry(m,2,invoke,modules)!=P66_NATIVE_BAD_MODE)++c.failures;m->words[0163]=before[32];
  if(image_hash(m)!=before_hash)++c.failures;
  pn_words(m,base,after);for(j=0;j<PN_WORDS;++j)if(before[j]!=after[j])++c.failures;
 }
 return c.failures;
}
