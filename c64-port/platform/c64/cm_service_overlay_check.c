/* SPDX-License-Identifier: MIT */
#include "cm_service_overlay_binding.h"
#include "cm_service_loop_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_service_fixture *f=CM_SERVICE_FIXTURE;cm_native_io *io=CM_SERVICE_IO;uint32_t hash;unsigned i;
 if(selector!=f->step||f->run>=6u||f->source_case!=f->run*64u||f->step>=cm_service_loop_count[f->source_case])return 1;
 hash=fold(2166136261u,f->group);hash=fold(hash,f->status);for(i=0;i<2048u;++i)hash=fold(hash,s->words[i]);hash=fold(hash,io->superbank);hash=fold(hash,io->interrupt_enabled);
 if(hash!=cm_service_loop_hash[f->run][f->step])++f->failures;
 if(f->run==0u&&f->step+1u==cm_service_loop_count[0]){if(cm_service_loop_sample[0]!=f->group||cm_service_loop_sample[1]!=f->status)++f->failures;for(i=0;i<2048u;++i)if(cm_service_loop_sample[i+2u]!=s->words[i])++f->failures;if(cm_service_loop_sample[2050]!=io->superbank||cm_service_loop_sample[2051]!=io->interrupt_enabled)++f->failures;}
 ++f->step;return 0;
}
