/* SPDX-License-Identifier: MIT */
#include "cm_abort_lifecycle_binding.h"
#include "cm_abort_lifecycle_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return(h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_lifecycle_fixture *f=CM_LIFECYCLE_FIXTURE;const uint16_t *fields=(const uint16_t *)CM_LIFECYCLE_IO;uint32_t h;unsigned i;if(selector!=f->phase||f->run>=CM_LIFECYCLE_TARGET_CASES||f->source_case!=cm_lifecycle_indices[f->run]||f->phase>=cm_lifecycle_counts[f->run])return 1;h=fold(2166136261u,f->phase);h=fold(h,f->status);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);for(i=0;i<13u;++i)h=fold(h,fields[i]);h=fold(h,f->in_isr);if(h!=cm_lifecycle_hash[f->run][f->phase])++f->failures;
 if((f->run==0u&&f->phase==0u)||(f->run+1u==CM_LIFECYCLE_TARGET_CASES&&f->phase+1u==cm_lifecycle_counts[f->run])){unsigned sample=f->run==0u?0u:1u;for(i=0;i<2048u;++i)if(s->words[i]!=cm_lifecycle_sample[sample][i])++f->failures;for(i=0;i<13u;++i)if(fields[i]!=cm_lifecycle_sample[sample][2048u+i])++f->failures;if(f->in_isr!=cm_lifecycle_sample[sample][2061])++f->failures;}
 ++f->phase;return 0;}
