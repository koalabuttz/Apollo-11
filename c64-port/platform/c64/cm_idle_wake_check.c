/* SPDX-License-Identifier: MIT */
#include "cm_idle_wake_binding.h"
#include "cm_idle_wake_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_idle_wake_fixture *f=CM_IDLE_WAKE_FIXTURE;cm_native_io *io=CM_IDLE_WAKE_IO;unsigned i,row;uint32_t h,id;if(f->run>=8u||selector!=f->phase||selector>=9u)return 1;row=f->run*9u+selector;id=f->source_case*9u+selector;h=fold(2166136261u,(uint16_t)id);h=fold(h,(uint16_t)(id>>16));h=fold(h,0);h=fold(h,0);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);for(i=0;i<13u;++i)h=fold(h,((const uint16_t*)io)[i]);h=fold(h,f->irq.in_isr);h=fold(h,f->irq.pending_idle);if(h!=cm_idle_wake_hash[row])++f->failures;
 if(row==0u||row==71u){const uint16_t *sample=cm_idle_wake_samples[row==71u];for(i=0;i<2048u;++i)if(s->words[i]!=sample[i+4u])++f->failures;for(i=0;i<13u;++i)if(((const uint16_t*)io)[i]!=sample[i+2052u])++f->failures;if(f->irq.in_isr!=sample[2065]||f->irq.pending_idle!=sample[2066])++f->failures;}
 ++f->phase;return 0;}
