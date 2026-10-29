/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_sps_rcs_cases.h"
#define LM_SPS_RCS_FROZEN_IMPLEMENTATION
#include "lm_sps_rcs_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned lm_sps_rcs_check(void){unsigned row,fail=0;uint16_t value=0;
for(row=0;row<96u;++row){frozen_words r;uint16_t low,high,expected_irq,pad;uint32_t n;agc_word irq;if(!lm_sps_rcs_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);lm_sps_rcs_input(&image,&irq,n);expected=image;if(!frozen_words_next(&r,&expected_irq)||!patch(&r,&expected)){++fail;continue;}while(frozen_words_next(&r,&pad))if(pad)++fail;if(!frozen_words_finished(&r)||lm_sps_rcs(&image)||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}
if(!lm_sps_rcs_constant_read(18342,&value)||value!=00632){++fail;}
if(!lm_sps_rcs_constant_read(18372,&value)||value!=4){++fail;}
if(!lm_sps_rcs_constant_read(18373,&value)||value!=012){++fail;}
if(!lm_sps_rcs_constant_read(18374,&value)||value!=012){++fail;}return fail;}
#ifndef LM_SPS_RCS_CHECK_ONLY
static unsigned rejected(void){static const unsigned fields[]={3,4,5,6,7};unsigned k,f=0;agc_word irq;uint16_t value=03142;for(k=0;k<5u;++k){lm_sps_rcs_input(&image,&irq,660);image.words[fields[k]]^=1;expected=image;if(lm_sps_rcs(&image)!=LM_SPS_RCS_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;}lm_sps_rcs_input(&image,&irq,660);image.words[03105]=2;expected=image;if(lm_sps_rcs(&image)!=LM_SPS_RCS_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;
lm_sps_rcs_input(&image,&irq,660);image.words[2]=010000;expected=image;if(lm_sps_rcs(&image)!=LM_SPS_RCS_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;
if(lm_sps_rcs_constant_read(0,&value)||value!=03142||lm_sps_rcs_constant_read(18342,0)){++f;}return f;}
int main(void){unsigned f=lm_sps_rcs_check()+rejected();uint32_t a;for(a=0;a<65536u;++a){uint16_t v=03142,expect=0;uint8_t supported=(uint8_t)(a==18342u||a==18372u||a==18373u||a==18374u);if(supported)expect=(uint16_t)(a==18342u?00632u:(a==18372u?4u:012u));if(lm_sps_rcs_constant_read((uint16_t)a,&v)!=supported||v!=(supported?expect:03142u))++f;}printf("96 complete original LM SPS controller states and constants: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
