/* SPDX-License-Identifier: MIT */
#include "radial_cases.h"
#define RADIAL_FROZEN_IMPLEMENTATION
#define RADIAL_INPUT_FROZEN_IMPLEMENTATION
#include "radial_vectors.h"
unsigned ra_check(radial_case *c){uint16_t k,row=0;unsigned failures=0;for(k=0;k<RA_REFERENCE_CASES;++k){uint16_t n=radial_indices[k];uint8_t step;
#ifndef RADIAL_C64_FROZEN_ENTRY
ra_prepare(c,n);
#endif
for(step=0;step<ra_steps(n);++step){frozen_words reader;uint16_t w[RA_WORDS],expected;unsigned i,status;
#ifdef RADIAL_C64_FROZEN_ENTRY
if(!radial_input_reference_begin(&reader,row))return 1;for(i=0;i<RA_WORDS;++i)if(!frozen_words_next(&reader,&w[i]))return 1;if(!frozen_words_finished(&reader))return 1;ra_from_words(c,w);
#else
ra_next(c,n,step);if(ra_integrated(n))ra_prefix(c,n);
#endif
status=(unsigned)ra_apply(c);ra_words(c,w);if(!radial_reference_begin(&reader,row))++failures;else{if(!frozen_words_next(&reader,&expected)||status!=expected)++failures;for(i=0;i<RA_WORDS;++i){if(!frozen_words_next(&reader,&expected)){++failures;break;}if(w[i]!=expected)++failures;}if(!frozen_words_finished(&reader))++failures;}++row;}}if(row!=RADIAL_FROZEN_ROWS)++failures;return failures;}
