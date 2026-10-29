/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_rcs_monitor_cases.h"
#define LM_RCS_MONITOR_FROZEN_IMPLEMENTATION
#include "lm_rcs_monitor_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned lm_rcs_monitor_check(void){unsigned row,fail=0;uint16_t value=0;static const uint16_t constants[18]={02454,040106,040,020,0100,0200,010,1,4,2,010,020,4,0200,1,2,040,0100};
for(row=0;row<192u;++row){frozen_words r;uint16_t low,high,irq,sup,ch,pad;uint32_t n;agc_word input;p66_bias_io io;enum exec_status s;if(!lm_rcs_monitor_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);lm_rcs_monitor_input(&image,&io,&input,n);expected=image;if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!frozen_words_next(&r,&ch)||!patch(&r,&expected)){++fail;continue;}while(frozen_words_next(&r,&pad))if(pad)++fail;s=lm_rcs_monitor(&image,&io,input);if(!frozen_words_finished(&r)||((unsigned)s!=(unsigned)EXEC_OK&&(unsigned)s!=(unsigned)EXEC_NO_CORE)||io.interrupt_enabled!=irq||io.superbank!=sup||input!=ch||memcmp(&image,&expected,sizeof image))++fail;}
for(row=0;row<18u;++row){uint16_t address=(uint16_t)(row<2u?6816u+row:6817u+row);if(!lm_rcs_monitor_constant_read(address,&value)||value!=constants[row])++fail;}return fail;}
#ifndef LM_RCS_MONITOR_CHECK_ONLY
static unsigned rejected(void){static const unsigned fields[]={3,4,5,6,7,01276};unsigned k,f=0;agc_word ch;p66_bias_io io,old;uint16_t value=03142;for(k=0;k<6u;++k){lm_rcs_monitor_input(&image,&io,&ch,660);if(k==5u)image.words[fields[k]]=0400;else image.words[fields[k]]^=1;expected=image;old=io;if((unsigned)lm_rcs_monitor(&image,&io,ch)!=(unsigned)EXEC_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++f;}
lm_rcs_monitor_input(&image,&io,&ch,660);io.interrupt_enabled=1;expected=image;old=io;if((unsigned)lm_rcs_monitor(&image,&io,ch)!=(unsigned)EXEC_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++f;
lm_rcs_monitor_input(&image,&io,&ch,660);io.superbank=1;expected=image;old=io;if((unsigned)lm_rcs_monitor(&image,&io,ch)!=(unsigned)EXEC_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++f;
lm_rcs_monitor_input(&image,&io,&ch,660);image.words[0154+EXEC_PRIORITY]=0;expected=image;if((unsigned)lm_rcs_monitor(&image,&io,ch)!=(unsigned)EXEC_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;
if(lm_rcs_monitor_constant_read(0,&value)||value!=03142||lm_rcs_monitor_constant_read(6816,0)){++f;}return f;}
int main(void){unsigned f=lm_rcs_monitor_check()+rejected();uint32_t a;static const uint16_t constants[18]={02454,040106,040,020,0100,0200,010,1,4,2,010,020,4,0200,1,2,040,0100};for(a=0;a<65536u;++a){uint16_t v=03142,expect=0;uint8_t supported=(uint8_t)(a==6816u||a==6817u||(a>=6819u&&a<6835u));if(supported)expect=constants[a<6819u?a-6816u:a-6817u];if(lm_rcs_monitor_constant_read((uint16_t)a,&v)!=supported||v!=(supported?expect:03142u))++f;}printf("192 complete original LM RCS monitor states and constants: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
