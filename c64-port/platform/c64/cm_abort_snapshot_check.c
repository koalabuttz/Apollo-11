/* SPDX-License-Identifier: MIT */
#include "cm_abort_snapshot_binding.h"
#include "cm_abort_snapshot_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return(h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_abort_fixture *f=CM_ABORT_FIXTURE;cm_native_io *io=CM_ABORT_IO;uint32_t h;unsigned i;if(selector!=f->phase||f->run>=CM_ABORT_TARGET_CASES||f->source_case!=cm_abort_indices[f->run]||f->phase>=cm_abort_counts[f->run])return 1;h=fold(2166136261u,f->phase);h=fold(h,f->status);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);h=fold(h,io->superbank);h=fold(h,io->interrupt_enabled);if(h!=cm_abort_hash[f->run][f->phase])++f->failures;
 if((f->run==0u&&f->phase==0u)||(f->run==17u&&f->phase+1u==cm_abort_counts[f->run])){unsigned sample=f->run==0u?0u:1u;for(i=0;i<2048u;++i)if(s->words[i]!=cm_abort_sample[sample][i])++f->failures;if(io->superbank!=cm_abort_sample[sample][2048]||io->interrupt_enabled!=cm_abort_sample[sample][2049])++f->failures;}
 ++f->phase;return 0;}
