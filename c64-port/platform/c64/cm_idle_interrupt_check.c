/* SPDX-License-Identifier: MIT */
#include "cm_idle_interrupt_binding.h"
#include "cm_idle_interrupt_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_idle_fixture *f=CM_IDLE_FIXTURE;cm_native_io *io=CM_IDLE_IO;uint32_t h;unsigned i;
 if(f->run>=CM_IDLE_TARGET_CASES||selector!=f->run||f->step||f->source_case!=cm_idle_indices[f->run])return 1;
 h=fold(2166136261u,(uint16_t)f->source_case);h=fold(h,(uint16_t)(f->source_case>>16));h=fold(h,0);h=fold(h,f->status);
 for(i=0;i<2048u;++i)h=fold(h,s->words[i]);for(i=0;i<13u;++i)h=fold(h,((const uint16_t *)io)[i]);h=fold(h,f->irq.in_isr);h=fold(h,f->irq.pending_idle);if(h!=cm_idle_hash[f->run])++f->failures;
 if(f->run==0u||f->run==63u){const uint16_t *sample=cm_idle_samples[f->run==63u];for(i=0;i<2048u;++i)if(s->words[i]!=sample[i+4u])++f->failures;for(i=0;i<13u;++i)if(((const uint16_t *)io)[i]!=sample[i+2052u])++f->failures;if(f->irq.in_isr!=sample[2065]||f->irq.pending_idle!=sample[2066])++f->failures;}
 ++f->step;return 0;
}
