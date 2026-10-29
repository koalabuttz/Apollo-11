/* SPDX-License-Identifier: MIT */
#include "p66_guidance_fixture.h"
#include "p66_guidance_live_cases.h"
#include "p66_guidance_live_vectors.h"
static uint32_t hash_word(uint32_t h,uint16_t w){h=(h^(uint8_t)w)*UINT32_C(16777619);return(h^(uint8_t)(w>>8))*UINT32_C(16777619);}
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 p66_guidance_fixture *f=P66_GUIDANCE_FIXTURE;uint16_t map[PGL_WORDS];unsigned i,phase=f->mixed&&f->stage>=2?f->stage-2:f->stage;uint32_t h=UINT32_C(2166136261);
 static const uint16_t control[7]={0116,023,0115,7,3,4,6};
 if(f->number>=2||f->stage>=(f->mixed?PGL_MIXED_STAGES:PGL_STAGES)||selector!=(f->mixed?pgl_mixed_boundaries[f->stage]:pgl_boundaries[f->stage]))return 1;
 /* Stream the full493-word source state: no large second snapshot on stack. */
 pgl_addresses(map,image->words[0120]);for(i=0;i<PGL_WORDS;++i)h=hash_word(h,image->words[map[i]]);
 for(i=0;i<96;++i)h=hash_word(h,image->words[0154+i]);for(i=0;i<220;++i)h=hash_word(h,image->words[0400+i]);
 h=hash_word(h,image->words[0120]);h=hash_word(h,image->words[0121]);for(i=0;i<7;++i)h=hash_word(h,image->words[control[i]]);
 h=hash_word(h,phase<5?0:image->words[5]);h=hash_word(h,phase<5?0:image->words[2]);
 if(h!=(f->mixed?pgl_mixed_fingerprints[f->number][f->stage]:pgl_fingerprints[f->number][f->stage]))++f->failures;
 ++f->stage;return 0;
}
