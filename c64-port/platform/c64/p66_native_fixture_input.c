/* SPDX-License-Identifier: MIT */
#include "p66_native_fixture.h"
#include "p66_native_cases.h"
#include "p66_native_vectors.h"
#include "p66_native_mixed_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 executive state;unsigned i;p66_native_fixture *f=P66_NATIVE_FIXTURE;
 uint8_t mixed=(uint8_t)(selector>>8);selector&=255;
 if(selector>=8 || mixed>1)return 1;
 pn_prepare(image,&state,0,selector);
 if(mixed){unsigned base=image->words[0120]==0401u?0455u:0401u;image->words[base-1u]=0;
  for(i=0;i<43;++i)image->words[base+i]=(agc_word)((i*17u+selector*31u)&077777u);
  image->words[0154+12+EXEC_LOC]=03275;image->words[0154+12+EXEC_BANK]=064007;
  image->words[0154+12+EXEC_PUSH]=(agc_word)base;image->words[0154+12+EXEC_PRIORITY]=(agc_word)(04000u+base);
 }
 f->mixed=mixed;f->number=(uint8_t)selector;f->stage=0;f->failures=0;
 for(i=0;i<(mixed?7u:5u);++i)f->fingerprints[i]=mixed?pn_mixed_fingerprints[selector][i]:pn_fingerprints[selector][i];
 return 0;
}
