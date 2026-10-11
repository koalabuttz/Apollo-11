/* SPDX-License-Identifier: MIT */
#include "cm_interbank_extended_binding.h"
#include "cm_interbank_extended_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_ib_ext_fixture *f=CM_IB_EXT_FIXTURE;cm_native_io *io=CM_IB_EXT_IO;uint32_t h;unsigned i;
 if(f->run>=CM_IB_EXT_TARGET_CASES||selector!=f->run||f->step||f->source_case!=cm_ib_ext_indices[f->run])return 1;
 h=fold(2166136261u,(uint16_t)f->source_case);h=fold(h,(uint16_t)(f->source_case>>16));h=fold(h,(uint16_t)(f->source_case/65536u));h=fold(h,f->status);
 for(i=0;i<2048u;++i)h=fold(h,s->words[i]);h=fold(h,io->superbank);h=fold(h,io->interrupt_enabled);
 if(h!=cm_ib_ext_hash[f->run])++f->failures;
 if(f->run==0u||f->run==24u){const uint16_t *sample=cm_ib_ext_samples[f->run==24u];for(i=0;i<2048u;++i)if(s->words[i]!=sample[i+4u])++f->failures;if(io->superbank!=sample[2052]||io->interrupt_enabled!=sample[2053])++f->failures;}
 ++f->step;return 0;
}
