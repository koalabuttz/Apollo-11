/* SPDX-License-Identifier: MIT */
#include "cm_fixed_compat_binding.h"
#include "cm_fixed_compat_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_fixed_fixture *f=CM_FIXED_FIXTURE;cm_native_io *io=CM_FIXED_IO;uint32_t h;unsigned i;
 if(f->run>=80u||selector!=f->phase||f->phase>1u||f->step!=f->phase||f->source_case!=cm_fixed_compat_indices[f->run])return 1;
 h=fold(2166136261u,(uint16_t)f->source_case);h=fold(h,(uint16_t)(f->source_case>>16));h=fold(h,f->phase);h=fold(h,f->status);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);h=fold(h,io->superbank);h=fold(h,io->interrupt_enabled);
 if(h!=cm_fixed_compat_hash[f->run][f->phase])++f->failures;
 if(f->run==0u){const uint16_t *sample=cm_fixed_compat_samples[f->phase];for(i=0;i<2048u;++i)if(sample[i+4u]!=s->words[i])++f->failures;if(sample[2052]!=io->superbank||sample[2053]!=io->interrupt_enabled)++f->failures;}
 ++f->step;return 0;
}
