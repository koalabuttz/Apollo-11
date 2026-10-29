/* SPDX-License-Identifier: MIT */
#include "cm_priority_display_binding.h"
#include "cm_priority_display_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_priority_display_fixture *f=CM_PRIORITY_DISPLAY_FIXTURE;const uint16_t *fields=(const uint16_t*)CM_PRIORITY_DISPLAY_IO;uint32_t h;unsigned i;
 if(f->run>=32u||selector!=f->run||f->step>=cm_priority_display_counts[f->run]||f->source_case!=cm_priority_display_indices[f->run])return 1;
 h=fold(2166136261u,(uint16_t)f->source_case);h=fold(h,(uint16_t)(f->source_case>>16));h=fold(h,f->step);h=fold(h,f->status);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);for(i=0;i<13u;++i)h=fold(h,fields[i]);if(h!=cm_priority_display_hash[f->run][f->step])++f->failures;
 if((f->run==0u&&f->step==0u)||(f->run==31u&&f->step+1u==cm_priority_display_counts[31])){const uint16_t *sample=cm_priority_display_samples[f->run==31u];for(i=0;i<2048u;++i)if(s->words[i]!=sample[i+4u])++f->failures;for(i=0;i<13u;++i)if(fields[i]!=sample[2052u+i])++f->failures;}++f->step;return 0;}
