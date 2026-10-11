/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "trig_cases.h"
#include "trig_vectors.h"
unsigned tg_check(trig_case *);
static trig_case c;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(!(p&4u))__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;static const uint16_t samples[5]={0,5,6,3,32767},rows[5]={0,5,6,3,127};unsigned failures;uint8_t i,token;mail[0]=0;puts("APOLLO NATIVE CDU TRIGONOMETRY");failures=tg_check(&c);
for(i=0;i<5u;++i){uint8_t p;uint16_t w[TG_WORDS],word;unsigned j;frozen_words reader;tg_input(&c,samples[i]);token=enter();mail[8]=(uint8_t)(0x10u+i);if(!tg_apply(&c,tg_operation(samples[i])))++failures;mail[8]=(uint8_t)(0x20u+i);leave(token);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||(i==0&&(p&4u)))++failures;tg_words(&c,w);if(!trig_reference_begin(&reader,rows[i]))++failures;else{for(j=0;j<TG_WORDS;++j)if(!frozen_words_next(&reader,&word)||w[j]!=word)++failures;if(!frozen_words_finished(&reader))++failures;}}
tg_input(&c,3);token=enter();{uint8_t nested=enter(),p;(void)tg_apply(&c,3);leave(nested);__asm__ volatile("php\npla":"=a"(p)::"memory");if(!(p&4u))++failures;}leave(token);mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=128;mail[4]=0;mail[5]=3;mail[6]=(uint8_t)sizeof(trig_case);mail[7]=TG_WORDS;puts(failures?"TRIG TESTS FAILED":"128 COMPLETE ORIGINAL STATES PASS");mail[0]=0xa5;return failures;}
