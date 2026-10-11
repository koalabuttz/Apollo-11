/* SPDX-License-Identifier: MIT */
#include "cm_basic_finish_cases.h"
#include "cm_basic_finish_vectors.h"
#include "cm_basic_finish_check.h"
static apollo_overlay_state state;static cm_native_io io;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_basic_finish_check(void){unsigned run,n,i,j,fail=0;
 for(run=0;run<
#ifdef __mos__
 128u
#else
 16384u
#endif
 ;++run){uint32_t h;enum exec_status status;exec_transfer next;
#ifdef __mos__
 n=cm_basic_finish_indices[run];
#else
 n=run;
#endif
 cm_basic_finish_input(&state,&io,(uint16_t)n);status=cm_basic_finish(&state,&io,&next);
 h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);h=fold(h,io.channel11);h=fold(h,next.interpretive);h=fold(h,next.address.location);h=fold(h,next.address.bank);
 if(h!=cm_basic_finish_hash[run])++fail;
 for(j=0;j<2u;++j)if(n==cm_basic_finish_sample_indices[j]){if(cm_basic_finish_samples[j][0]!=(uint16_t)status)++fail;for(i=0;i<2048u;++i)if(state.words[i]!=cm_basic_finish_samples[j][i+1u])++fail;if(io.superbank!=cm_basic_finish_samples[j][2049]||io.interrupt_enabled!=cm_basic_finish_samples[j][2050]||io.channel11!=cm_basic_finish_samples[j][2051]||next.interpretive!=cm_basic_finish_samples[j][2052]||next.address.location!=cm_basic_finish_samples[j][2053]||next.address.bank!=cm_basic_finish_samples[j][2054])++fail;}
 }return fail;}
