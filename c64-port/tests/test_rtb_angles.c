/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "rtb_angles_cases.h"
#ifndef RTB_MISSION
#define RTB_MISSION 0
#endif
#ifndef RTB_ENTRY
#define RTB_ENTRY 0
#endif
#define RTB_ANGLES_FROZEN_IMPLEMENTATION
#include "rtb_angles_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned rtb_angles_check(void){unsigned row,fail=0;
for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,pad;uint32_t n;p66_bias_io io;if(!rtb_angles_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);rtb_angles_input(&image,&io,(enum rtb_mission)RTB_MISSION,(enum rtb_angle_entry)RTB_ENTRY,n);expected=image;if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!patch(&r,&expected)){++fail;continue;}while(frozen_words_next(&r,&pad))if(pad)++fail;if(!frozen_words_finished(&r)||(unsigned)rtb_angles(&image,&io,(enum rtb_mission)RTB_MISSION,(enum rtb_angle_entry)RTB_ENTRY)!=(unsigned)RTB_OK||io.interrupt_enabled!=irq||io.superbank!=sup||memcmp(&image,&expected,sizeof image))++fail;}{uint16_t value=03142;uint8_t supported=(uint8_t)(RTB_MISSION==1);if(rtb_angles_constant_read((enum rtb_mission)RTB_MISSION,19349,&value)!=supported||value!=(supported?01155u:03142u))++fail;}return fail;}
#ifndef RTB_ANGLES_CHECK_ONLY
static unsigned rejected(void){static const unsigned fields[]={3,4,5,6,7};unsigned k,f=0;p66_bias_io io,old;for(k=0;k<5u;++k){rtb_angles_input(&image,&io,(enum rtb_mission)RTB_MISSION,(enum rtb_angle_entry)RTB_ENTRY,660);image.words[fields[k]]^=1;expected=image;old=io;if((unsigned)rtb_angles(&image,&io,(enum rtb_mission)RTB_MISSION,(enum rtb_angle_entry)RTB_ENTRY)!=(unsigned)RTB_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++f;}return f;}
int main(void){unsigned f=rtb_angles_check()+rejected();uint32_t a;for(a=0;a<65536u;++a){uint16_t value=03142;uint8_t supported=(uint8_t)(RTB_MISSION==1&&a==19349u);if(rtb_angles_constant_read((enum rtb_mission)RTB_MISSION,(uint16_t)a,&value)!=supported||value!=(supported?01155u:03142u))++f;}if(rtb_angles_constant_read((enum rtb_mission)RTB_MISSION,19349,0))++f;printf("32 complete original RTB angle states mission%d entry%d: %s\n",RTB_MISSION,RTB_ENTRY,f?"FAIL":"PASS");return f?1:0;}
#endif
