/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_quality.h"
#include "cm_quality_cases.h"
#ifndef CMQ_PHASE
#define CMQ_PHASE 0
#endif
#define STR2(x) #x
#define STR(x) STR2(x)
#define VEC2(x) cm_quality_vectors_##x.h
#define VEC(x) VEC2(x)
#define CM_QUALITY_FROZEN_IMPLEMENTATION
#include STR(VEC(CMQ_PHASE))
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static unsigned guards(p66_bias_io *io){uint16_t addresses[5]={0163,0166,0,7,2047};unsigned i,fail=0;addresses[2]=(uint16_t)(image.words[0120]+052u);for(i=0;i<5u;++i){agc_word old=image.words[addresses[i]];image.words[addresses[i]]=i==4u?0100000u:(agc_word)(old^1u);expected=image;if(cm_quality_step(&image,io,CMQ_PHASE)!=2u||memcmp(&image,&expected,sizeof image))++fail;image.words[addresses[i]]=old;}expected=image;if(cm_quality_step(&image,io,20)!=2u||memcmp(&image,&expected,sizeof image))++fail;if(cm_quality_step(NULL,io,CMQ_PHASE)!=2u||cm_quality_step(&image,NULL,CMQ_PHASE)!=2u||memcmp(&image,&expected,sizeof image))++fail;{agc_word old=io->superbank;io->superbank=(agc_word)(old|1u);if(cm_quality_step(&image,io,CMQ_PHASE)!=2u||memcmp(&image,&expected,sizeof image)||io->superbank!=(agc_word)(old|1u))++fail;io->superbank=old;}return fail;}
unsigned cm_quality_check(void){unsigned row,fail=0;for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,status,pad;uint32_t n;p66_bias_io io;uint8_t result;if(!cm_quality_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);cm_quality_input(&image,&io,n);if(!frozen_words_next(&r,&io.interrupt_enabled)||!frozen_words_next(&r,&io.superbank)||!patch(&r,&image)){++fail;continue;}if(row==0u)fail+=guards(&io);expected=image;if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!frozen_words_next(&r,&status)||!patch(&r,&expected)){++fail;continue;}while(frozen_words_next(&r,&pad))if(pad)++fail;result=cm_quality_step(&image,&io,CMQ_PHASE);if(!frozen_words_finished(&r)||result!=status||io.interrupt_enabled!=irq||io.superbank!=sup||memcmp(&image,&expected,sizeof image))++fail;}return fail;}
#ifndef CM_QUALITY_CHECK_ONLY
int main(void){unsigned f=cm_quality_check();printf("32 complete QUALITY phase%d original states: %s\n",CMQ_PHASE,f?"FAIL":"PASS");return f?1:0;}
#endif
