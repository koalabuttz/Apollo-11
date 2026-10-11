/* SPDX-License-Identifier: MIT */
#include "trig_cases.h"
#define TRIG_FROZEN_IMPLEMENTATION
#include "trig_vectors.h"
unsigned tg_check(trig_case *c){unsigned k,op,failures=0;for(k=0;k<32u;++k)for(op=0;op<4u;++op){frozen_words reader;uint16_t w[TG_WORDS],word;unsigned i;tg_input(c,(uint32_t)(4u*trig_reference_indices[k]+op));if(!tg_apply(c,(uint8_t)op))++failures;tg_words(c,w);if(!trig_reference_begin(&reader,(uint16_t)(4u*k+op)))++failures;else{for(i=0;i<TG_WORDS;++i){if(!frozen_words_next(&reader,&word)){++failures;break;}if(w[i]!=word)++failures;}if(!frozen_words_finished(&reader))++failures;}}return failures;}
