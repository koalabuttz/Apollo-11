/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "rtb_pulse_return_cases.h"
#ifndef RTBP_MISSION
#define RTBP_MISSION 0
#endif
#define RTB_PULSE_RETURN_FROZEN_IMPLEMENTATION
#include "rtb_pulse_return_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned rtb_pulse_return_check(void){unsigned row,fail=0;for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,status,pad;uint32_t n;p66_bias_io io;enum rtb_status result;if(!rtb_pulse_return_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);rtb_pulse_return_input(&image,&io,(enum rtb_mission)RTBP_MISSION,n);expected=image;if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!frozen_words_next(&r,&status)||!patch(&r,&expected)){++fail;continue;}while(frozen_words_next(&r,&pad))if(pad)++fail;result=rtb_pulse_return(&image,&io,(enum rtb_mission)RTBP_MISSION);if(!frozen_words_finished(&r)||(unsigned)result!=status||io.interrupt_enabled!=irq||io.superbank!=sup||memcmp(&image,&expected,sizeof image))++fail;}return fail;}
#ifndef RTB_PULSE_RETURN_CHECK_ONLY
int main(void){unsigned f=rtb_pulse_return_check();printf("32 complete original PULSEIMU return states mission%d: %s\n",RTBP_MISSION,f?"FAIL":"PASS");return f?1:0;}
#endif
