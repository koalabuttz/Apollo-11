/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "rtb_vector_cases.h"
#ifndef VECTOR_MISSION
#define VECTOR_MISSION 0
#endif
#ifndef VECTOR_ENTRY
#define VECTOR_ENTRY 0
#endif
#define RTB_VECTOR_FROZEN_IMPLEMENTATION
#include "rtb_vector_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned rtb_vector_check(void){unsigned row,fail=0;
for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,pad;uint32_t n;p66_bias_io io;if(!rtb_vector_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);rtb_vector_input(&image,&io,(enum rtb_mission)VECTOR_MISSION,(enum rtb_vector_entry)VECTOR_ENTRY,n);expected=image;if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!patch(&r,&expected)){++fail;continue;}while(frozen_words_next(&r,&pad))if(pad)++fail;if(!frozen_words_finished(&r)||(unsigned)rtb_vector(&image,&io,(enum rtb_mission)VECTOR_MISSION,(enum rtb_vector_entry)VECTOR_ENTRY)!=(unsigned)RTB_OK||io.interrupt_enabled!=irq||io.superbank!=sup||memcmp(&image,&expected,sizeof image))++fail;}return fail;}
#ifndef RTB_VECTOR_CHECK_ONLY
static unsigned rejected(void){static const unsigned fields[]={3,4,5,6,7};unsigned k,f=0;p66_bias_io io,old;for(k=0;k<5u;++k){rtb_vector_input(&image,&io,(enum rtb_mission)VECTOR_MISSION,(enum rtb_vector_entry)VECTOR_ENTRY,660);image.words[fields[k]]^=1;expected=image;old=io;if((unsigned)rtb_vector(&image,&io,(enum rtb_mission)VECTOR_MISSION,(enum rtb_vector_entry)VECTOR_ENTRY)!=(unsigned)RTB_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++f;}return f;}
int main(void){unsigned f=rtb_vector_check()+rejected();printf("32 complete RTB vector/frontier states mission%d entry%d: %s\n",VECTOR_MISSION,VECTOR_ENTRY,f?"FAIL":"PASS");return f?1:0;}
#endif
