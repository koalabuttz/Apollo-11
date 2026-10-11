/* SPDX-License-Identifier: MIT */
#include "cm_admission_floor_binding.h"
#include "cm_admission_floor_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_floor_fixture *f=CM_FLOOR_FIXTURE;cm_native_io *io=CM_FLOOR_IO;uint32_t h;unsigned i;
 if(f->run>=CM_FLOOR_TARGET_CASES||selector!=f->run||f->step||f->source_case!=cm_floor_indices[f->run])return 1;
 h=fold(2166136261u,(uint16_t)f->source_case);h=fold(h,(uint16_t)(f->source_case>>16));h=fold(h,0);h=fold(h,f->status);
 for(i=0;i<2048u;++i)h=fold(h,s->words[i]);for(i=0;i<13u;++i)h=fold(h,((const uint16_t *)io)[i]);if(h!=cm_floor_hash[f->run])++f->failures;
 if(f->run==0u||f->run==63u){const uint16_t *sample=cm_floor_samples[f->run==63u];for(i=0;i<2048u;++i)if(s->words[i]!=sample[i+4u])++f->failures;for(i=0;i<13u;++i)if(((const uint16_t *)io)[i]!=sample[i+2052u])++f->failures;}
 ++f->step;return 0;
}
