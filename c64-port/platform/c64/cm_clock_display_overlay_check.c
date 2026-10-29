/* SPDX-License-Identifier: MIT */
#include "cm_clock_display_overlay_binding.h"
#include "cm_clock_display_overlay_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_mission_fixture *f=CM_CLOCK_DISPLAY_FIXTURE;cm_native_io *io=CM_CLOCK_DISPLAY_IO;cm_mission_loop *r=CM_CLOCK_DISPLAY_RUNTIME;uint32_t h;unsigned i;
 if(selector!=f->step||f->run>=3u||f->source_case!=cm_clock_display_overlay_indices[f->run]||f->step>=cm_clock_display_overlay_count[f->run])return 1;
 h=fold(2166136261u,f->group);h=fold(h,f->status);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);h=fold(h,io->superbank);h=fold(h,io->interrupt_enabled);h=fold(h,io->channel11);h=fold(h,r->irq.in_isr);h=fold(h,r->irq.pending_danzig);h=fold(h,r->transfer.interpretive);h=fold(h,r->transfer.address.location);h=fold(h,r->transfer.address.bank);
 if(h!=cm_clock_display_overlay_hash[f->run][f->step])++f->failures;
 if(f->step+1u==cm_clock_display_overlay_count[f->run]&&1){unsigned sample=f->run;const uint16_t *v=cm_clock_display_overlay_sample[sample];if(v[0]!=f->group||v[1]!=f->status)++f->failures;for(i=0;i<2048u;++i)if(v[i+2u]!=s->words[i])++f->failures;if(v[2050]!=io->superbank||v[2051]!=io->interrupt_enabled||v[2052]!=io->channel11||v[2053]!=r->irq.in_isr||v[2054]!=r->irq.pending_danzig||v[2055]!=r->transfer.interpretive||v[2056]!=r->transfer.address.location||v[2057]!=r->transfer.address.bank)++f->failures;}
 ++f->step;return 0;}
