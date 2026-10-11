/* SPDX-License-Identifier: MIT */
#include "cm_abort_lifecycle_cases.h"
#include "cm_enema_image_cases.h"
agc_word cm_abort_lifecycle_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n,uint8_t *in_isr){static const agc_word entries[3]={07755,05721,05747};uint32_t c=n/3u;cm_enema_image_input(s,io,c);s->words[4]=02000;s->words[6]=(agc_word)(02000u|(s->words[3]>>8));s->words[5]=entries[n%3u];s->words[2]=c&1u?02724:02642;s->words[0103]=(agc_word)((s->words[0103]&077737u)|((c&1u)?040u:0));s->words[0375]=s->words[0376]=s->words[0377]=0;s->words[015]=03231;*in_isr=(uint8_t)(c&1u);return c&1u?01202:01201;}
uint8_t cm_abort_lifecycle_count(uint32_t n){if(n%3u==0u)return 7u;return (uint8_t)(n/3u&1u?8u:14u);}
uint8_t cm_abort_lifecycle_group(uint32_t n,uint8_t checkpoint){uint8_t count=cm_abort_lifecycle_count(n);if(checkpoint<5u)return checkpoint;if(checkpoint+2u>=count)return (uint8_t)(12u+checkpoint+2u-count);return checkpoint;}
