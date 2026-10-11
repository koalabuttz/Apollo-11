/* SPDX-License-Identifier: MIT */
#include "cm_init_display_mixed_binding.h"
#include "cm_init_display_mixed_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return(h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_init_mixed_fixture *f=CM_INIT_MIXED_FIXTURE;cm_native_io *io=CM_INIT_MIXED_IO;uint32_t h;unsigned i;if(selector!=f->phase||f->run>=CM_INIT_MIXED_TARGET_CASES||f->source_case!=cm_init_mixed_indices[f->run]||f->phase>=cm_init_mixed_counts[f->run])return 1;h=fold(2166136261u,f->phase);h=fold(h,f->status);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);h=fold(h,io->superbank);h=fold(h,io->interrupt_enabled);if(h!=cm_init_mixed_hash[f->run][f->phase])++f->failures;
 if(f->next.address.bank!=(uint16_t)(s->words[6]|io->superbank)||f->next.address.location!=(f->next.interpretive?s->words[0164]:s->words[5])||f->next.interpretive!=(uint8_t)(f->phase==1u&&(f->source_case&1u)&&(f->source_case/2u)%7u))++f->failures;
 if((f->run==0u&&f->phase==0u)||(f->run==55u&&f->phase+1u==cm_init_mixed_counts[f->run])){unsigned sample=f->run==0u?0u:1u;for(i=0;i<2048u;++i)if(s->words[i]!=cm_init_mixed_sample[sample][i])++f->failures;if(io->superbank!=cm_init_mixed_sample[sample][2048]||io->interrupt_enabled!=cm_init_mixed_sample[sample][2049])++f->failures;}
 ++f->phase;return 0;}
