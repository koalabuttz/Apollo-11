/* SPDX-License-Identifier: MIT */
#include "p66_status_check.h"
typedef char status_retains_entry_result[(sizeof(((p66_native_binding *)0)->status)==sizeof(uint16_t))?1:-1];
typedef struct {uint16_t code,calls,observations;} result_fixture;
static uint16_t report(void *user,apollo_overlay_state *image,uint16_t selector){
 result_fixture *f=user;(void)selector;++f->calls;
 /* Already qualified arithmetic alarm boundary. The fixture isolates the
  * resident transport, rather than recomputing a numerical operation. */
 image->words[5]=03402;image->words[2]=03043;
 image->words[0154]=012345;image->words[0155]=065432;
 return f->code;
}
static void observe(void *user,const apollo_overlay_state *image,agc_word loc){
 result_fixture *f=user;(void)image;(void)loc;++f->observations;
}
uint16_t p66_status_check(apollo_overlay_state *image,executive *state,
 exec_native *runtime,const exec_exclusion *guard){
 static const uint16_t codes[]={01302,01202,0x8003,0xffff,4};
 result_fixture f={0,0,0};p66_native_binding b={image,runtime,guard,observe,&f,0,report,&f};
 uint16_t errors=0;unsigned i,j;
 for(i=0;i<sizeof codes/sizeof codes[0];++i){
  for(j=0;j<APOLLO_SHARED_WORDS;++j)image->words[j]=0;
  exec_init(state);
  if(exec_admit(state,022000,03275,064007,1)!=EXEC_OK)++errors;
  p66_native_export_executive(image,state);
  if(exec_native_context_init(runtime,state,p66_native_mappings,P66_NATIVE_MAPPING_COUNT,&b)!=EXEC_OK)++errors;
  runtime->running=1;runtime->current.location=03275;runtime->current.bank=064007;
  f.code=codes[i];f.calls=0;f.observations=0;
  if(p66_native_step(&b)!=EXEC_OUT_OF_SCOPE||runtime->fault!=EXEC_OUT_OF_SCOPE||b.status!=f.code)++errors;
  if(f.calls!=1||f.observations!=0||image->words[5]!=03402||image->words[2]!=03043||
     state->core[0][0]!=012345||state->core[0][1]!=065432||image->words[0154]!=012345||image->words[0155]!=065432)++errors;
  if(runtime->current.location!=03275||runtime->current.bank!=064007||runtime->running)++errors;
  if(p66_native_step(&b)!=EXEC_OUT_OF_SCOPE||f.calls!=1||b.status!=f.code)++errors;
 }
 return errors;
}
