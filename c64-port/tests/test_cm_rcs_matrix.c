/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_rcs_matrix_cases.h"
#define CM_RCS_MATRIX_FROZEN_IMPLEMENTATION
#include "cm_rcs_matrix_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned cm_rcs_matrix_check(void){unsigned row,fail=0;uint16_t value=0;
for(row=0;row<64u;++row){frozen_words r;uint16_t low,high,expected_irq,pad;uint32_t n;agc_word irq;if(!cm_rcs_matrix_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);cm_rcs_matrix_input(&image,&irq,n);expected=image;if(!frozen_words_next(&r,&expected_irq)||!patch(&r,&expected)){++fail;continue;}while(frozen_words_next(&r,&pad))if(pad)++fail;if(!frozen_words_finished(&r)||cm_rcs_matrix(&image)||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}
if(!cm_rcs_matrix_constant_read(19271,&value)||value!=01224){++fail;}return fail;}
#ifndef CM_RCS_MATRIX_CHECK_ONLY
static unsigned rejected(void){static const unsigned fields[]={3,4,5,6,7};unsigned k,f=0;agc_word irq;uint16_t value=03142;for(k=0;k<5u;++k){cm_rcs_matrix_input(&image,&irq,660);image.words[fields[k]]^=1;expected=image;if(cm_rcs_matrix(&image)!=CM_RCS_MATRIX_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;}if(cm_rcs_matrix_constant_read(0,&value)||value!=03142||cm_rcs_matrix_constant_read(19271,0))++f;return f;}
int main(void){unsigned f=cm_rcs_matrix_check()+rejected();printf("64 complete original CM RCS matrix states andQUADANGL: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
