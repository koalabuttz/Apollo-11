/* SPDX-License-Identifier: MIT */
#include "projection_cases.h"
#define PROJECTION_FROZEN_IMPLEMENTATION
#define PROJECTION_INPUT_FROZEN_IMPLEMENTATION
#include "projection_vectors.h"
unsigned pj_seed(projection_case *c,uint16_t group){
#ifdef PROJECTION_C64_FROZEN_ENTRY
    frozen_words r;uint16_t w[PJ_WORDS];unsigned i;if(!projection_input_reference_begin(&r,group))return 1;for(i=0;i<PJ_WORDS;++i)if(!frozen_words_next(&r,&w[i]))return 1;if(!frozen_words_finished(&r))return 1;pj_from_words(c,w);
#else
    if(group>=PJ_REFERENCE_GROUPS)return 1;
    pj_input(c,projection_indices[group]);
#endif
    return 0;
}
unsigned pj_compare(projection_case *c,uint16_t row,unsigned status){frozen_words r;uint16_t w[PJ_WORDS],word;unsigned i,failures=0;pj_words(c,w);if(!projection_reference_begin(&r,row))return 1;if(!frozen_words_next(&r,&word)||status!=word)++failures;for(i=0;i<PJ_WORDS;++i)if(!frozen_words_next(&r,&word)||w[i]!=word)++failures;if(!frozen_words_finished(&r))++failures;return failures;}
unsigned pj_check(projection_case *c){uint16_t group;unsigned failures=0;for(group=0;group<PJ_REFERENCE_GROUPS;++group){uint8_t phase;if(pj_seed(c,group))return 1;for(phase=0;phase<3u;++phase){unsigned status=pj_apply(c,phase);failures+=pj_compare(c,(uint16_t)(3u*group+phase),status);}}return failures;}
