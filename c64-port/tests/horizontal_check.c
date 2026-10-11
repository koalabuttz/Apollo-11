/* SPDX-License-Identifier: MIT */
#include "horizontal_cases.h"
#define HORIZONTAL_FROZEN_IMPLEMENTATION
#define HORIZONTAL_INPUT_FROZEN_IMPLEMENTATION
#include "horizontal_vectors.h"
unsigned hh_seed(horizontal_case *c,uint16_t group){
#ifdef HORIZONTAL_C64_FROZEN_ENTRY
    frozen_words r;uint16_t w[HH_WORDS];unsigned i;if(!horizontal_input_reference_begin(&r,group))return 1;for(i=0;i<HH_WORDS;++i)if(!frozen_words_next(&r,&w[i]))return 1;if(!frozen_words_finished(&r))return 1;hh_from_words(c,w);
#else
    if(group>=HH_REFERENCE_GROUPS)return 1;
    hh_input(c,horizontal_indices[group]);
#endif
    return 0;
}
unsigned hh_compare(horizontal_case *c,uint16_t row,unsigned status){frozen_words r;uint16_t w[HH_WORDS],word;unsigned i,failures=0;hh_words(c,w);if(!horizontal_reference_begin(&r,row))return 1;if(!frozen_words_next(&r,&word)||status!=word)++failures;for(i=0;i<HH_WORDS;++i)if(!frozen_words_next(&r,&word)||w[i]!=word)++failures;if(!frozen_words_finished(&r))++failures;return failures;}
unsigned hh_check(horizontal_case *c){uint16_t group;unsigned failures=0;for(group=0;group<HH_REFERENCE_GROUPS;++group){uint8_t phase;if(hh_seed(c,group))return 1;for(phase=0;phase<8u;++phase){unsigned status=hh_apply(c,phase);failures+=hh_compare(c,(uint16_t)(8u*group+phase),status);}}return failures;}
