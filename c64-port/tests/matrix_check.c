/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "matrix_check.h"
#include "matrix_vectors.h"
uint16_t matrix_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t number,f,patch,end,failures=0;uint8_t phase,column;agc_word irq,super;enum apollo_fixed_mission mission=n<64u?APOLLO_FIXED_LM:APOLLO_FIXED_CM;if(n>=128u)return 1;number=matrix_numbers[n%64u];column=(uint8_t)((number/4u)%2u);matrix_input(s,&irq,&super,(uint16_t)(number/8u),mission,column);memcpy(before,s,sizeof *s);for(phase=0;phase<=number%4u;++phase)if(!matrix_native_phase(s,mission,column,s->words+01000,phase))++failures;patch=matrix_offsets[n];end=matrix_offsets[n+1u];for(f=0;f<2050u;++f){uint16_t expected=f<2048u?before->words[f]:f==2048u?super:irq;uint16_t actual=f<2048u?s->words[f]:f==2048u?super:irq;if(patch<end&&matrix_patch[patch][0]==f)expected=matrix_patch[patch++][1];if(actual!=expected)++failures;}if(patch!=end)++failures;return failures;}
