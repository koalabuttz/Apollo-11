/* SPDX-License-Identifier: MIT */
#include "p66_recurrence_fixture.h"
#include "p66_recurrence_live_vectors.h"
static uint32_t hash(uint32_t h,uint16_t w){h=(h^(uint8_t)w)*UINT32_C(16777619);return(h^(uint8_t)(w>>8))*UINT32_C(16777619);}
uint16_t apollo_overlay_main(apollo_overlay_state*m,uint16_t loc){p66_recurrence_fixture*f=P66_RECURRENCE_FIXTURE;uint32_t h=UINT32_C(2166136261);unsigned i;if(f->number>=2u||f->stage>=prl_counts[f->number]||loc!=prl_locations[f->number][f->stage])return 1;for(i=0;i<2048u;++i)h=hash(h,m->words[i]);h=hash(h,f->io->superbank);h=hash(h,f->io->interrupt_enabled);h=hash(h,*f->channel14);h=hash(h,*f->channel11);h=hash(h,*f->in_interrupt);if(h!=prl_fingerprints[f->number][f->stage])++f->failures;++f->stage;return 0;}
