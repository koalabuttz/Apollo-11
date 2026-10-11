/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lm_clock.h"
extern int CounterPINC(int16_t *);
int main(void){uint32_t n;uint16_t i,pending,source_pending;static const uint16_t ticks[16]={0,1,2,3,99,100,101,1023,16382,16383,16384,16385,32767,32768,65534,65535};apollo_overlay_state native,source,before;int16_t word;uint16_t got;unsigned count=0;
 for(n=0;n<=077777u;++n){word=(int16_t)n;got=(uint16_t)n;assert(lm_counter_pinc(&got)==CounterPINC(&word));assert(got==(uint16_t)word);}
 for(n=0;n<1024u;++n){for(i=0;i<2048u;++i)native.words[i]=(uint16_t)((n*1117u+i*337u)&077777u);native.words[024]=(uint16_t)((n*31u)&037777u);native.words[025]=(uint16_t)((n*187u)&037777u);native.words[026]=(uint16_t)((n*521u)&037777u);source=native;pending=source_pending=(uint16_t)(n&1u);assert(lm_clock_advance(&native,ticks[n%16u],&pending)==0);for(i=0;i<ticks[n%16u];++i){word=(int16_t)source.words[024];if(CounterPINC(&word)){int16_t high=(int16_t)source.words[025];(void)CounterPINC(&high);source.words[025]=(uint16_t)high;}source.words[024]=(uint16_t)word;word=(int16_t)source.words[026];if(CounterPINC(&word))source_pending=1;source.words[026]=(uint16_t)word;}assert(!memcmp(&native,&source,sizeof native)&&pending==source_pending);++count;}
 for(n=0;n<4u;++n){memset(&native,0,sizeof native);pending=0;if(n<3u)native.words[024u+n]=040000;else pending=2;before=native;source_pending=pending;assert(lm_clock_advance(&native,100,&pending)==-1);assert(!memcmp(&native,&before,sizeof native)&&pending==source_pending);}
 assert(lm_counter_pinc(0)==-1);got=0100000;assert(lm_counter_pinc(&got)==-1&&got==0100000);assert(lm_clock_advance(0,1,&pending)==-1);assert(lm_clock_advance(&native,1,0)==-1);
 printf("PASS:32768 hardware PINC raw signed-word transitions; %u complete2048-word TIME1/TIME2 carry/TIME3 latch sequences, 0..65535 pulses; rejected domains unchanged\n",count);return 0;}
