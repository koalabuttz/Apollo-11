/* SPDX-License-Identifier: MIT */
#include "p66_native_fixture.h"
#include "p66_native_cases.h"
static uint32_t hash_word(uint32_t hash,uint16_t word){
 hash=(hash^(uint8_t)word)*UINT32_C(16777619);
 return (hash^(uint8_t)(word>>8))*UINT32_C(16777619);
}
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 p66_native_fixture *f=P66_NATIVE_FIXTURE;uint16_t words[PN_WORDS];unsigned i;
 uint32_t hash=UINT32_C(2166136261);
 static const agc_word ends[5]={03353,03356,03361,03364,03373};
 static const agc_word mixed_ends[7]={03353,03356,03353,03356,03361,03364,03373};
 if(f->stage>=(f->mixed?7u:5u) || selector!=(f->mixed?mixed_ends[f->stage]:ends[f->stage]))return 1;
 pn_words(image,image->words[0120],words);
 for(i=0;i<PN_WORDS;++i)hash=hash_word(hash,words[i]);
 if(f->mixed){
  for(i=0;i<96;++i)hash=hash_word(hash,image->words[0154+i]);
  for(i=0;i<220;++i)hash=hash_word(hash,image->words[0400+i]);
  hash=hash_word(hash,image->words[0120]);hash=hash_word(hash,image->words[0121]);
  hash=hash_word(hash,image->words[0116]);hash=hash_word(hash,image->words[023]);
  hash=hash_word(hash,image->words[0115]);hash=hash_word(hash,image->words[7]);
  hash=hash_word(hash,image->words[3]);hash=hash_word(hash,image->words[4]);hash=hash_word(hash,image->words[6]);
 }
 if(hash!=f->fingerprints[f->stage])++f->failures;
 ++f->stage;return 0;
}
