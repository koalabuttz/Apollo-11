/* SPDX-License-Identifier: MIT */
#include "cm_abort_recovery_binding.h"
#include "cm_abort_recovery_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return(h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_recovery_fixture *f=CM_RECOVERY_FIXTURE;cm_native_io *io=CM_RECOVERY_IO;uint32_t h;unsigned i;if(selector!=f->phase||f->run>=CM_RECOVERY_TARGET_CASES||f->source_case!=cm_recovery_indices[f->run]||f->phase>=cm_recovery_counts[f->run])return 1;h=fold(2166136261u,f->phase);h=fold(h,f->status);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);const uint16_t *fields=(const uint16_t *)io;for(i=0;i<13u;++i)h=fold(h,fields[i]);if(h!=cm_recovery_hash[f->run][f->phase])++f->failures;
 if((f->run==0u&&f->phase==0u)||(f->run==15u&&f->phase+1u==cm_recovery_counts[f->run])){unsigned sample=f->run==0u?0u:1u;for(i=0;i<2048u;++i)if(s->words[i]!=cm_recovery_sample[sample][i])++f->failures;for(i=0;i<13u;++i)if(fields[i]!=cm_recovery_sample[sample][2048u+i])++f->failures;}
 ++f->phase;return 0;}
