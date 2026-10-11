/* SPDX-License-Identifier: MIT */
#include "cm_clock_mission_overlay_binding.h"
#include "cm_clock_mission_overlay_vectors.h"
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_mission_fixture *f=CM_CLOCK_MISSION_FIXTURE;cm_native_io *io=CM_CLOCK_MISSION_IO;cm_mission_loop *r=CM_CLOCK_MISSION_RUNTIME;uint32_t h;unsigned i;
 if(selector!=f->step||f->run>=7u||f->source_case!=f->run*64u||f->step>=cm_clock_mission_overlay_count[f->run])return 1;
 h=fold(2166136261u,f->group);h=fold(h,f->status);for(i=0;i<2048u;++i)h=fold(h,s->words[i]);h=fold(h,io->superbank);h=fold(h,io->interrupt_enabled);h=fold(h,r->irq.in_isr);h=fold(h,r->irq.pending_danzig);h=fold(h,r->transfer.interpretive);h=fold(h,r->transfer.address.location);h=fold(h,r->transfer.address.bank);
 if(h!=cm_clock_mission_overlay_hash[f->run][f->step])++f->failures;
 if(f->run==0u&&f->step+1u==cm_clock_mission_overlay_count[0]){if(cm_clock_mission_overlay_sample[0]!=f->group||cm_clock_mission_overlay_sample[1]!=f->status)++f->failures;for(i=0;i<2048u;++i)if(cm_clock_mission_overlay_sample[i+2u]!=s->words[i])++f->failures;if(cm_clock_mission_overlay_sample[2050]!=io->superbank||cm_clock_mission_overlay_sample[2051]!=io->interrupt_enabled||cm_clock_mission_overlay_sample[2052]!=r->irq.in_isr||cm_clock_mission_overlay_sample[2053]!=r->irq.pending_danzig||cm_clock_mission_overlay_sample[2054]!=r->transfer.interpretive||cm_clock_mission_overlay_sample[2055]!=r->transfer.address.location||cm_clock_mission_overlay_sample[2056]!=r->transfer.address.bank)++f->failures;}
 ++f->step;return 0;}
