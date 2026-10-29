/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "radial_cases.h"
#include "radial_vectors.h"
static radial_case c;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(!(p&4u))__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned failures=0;uint8_t i,token;
mail[0]=0;puts("APOLLO NATIVE P66 RADIAL ALTITUDE");failures+=ra_check(&c);
for(i=0;i<5u;++i){uint16_t k=i<4u?i:RA_REFERENCE_CASES-1u,row=0,w[RA_WORDS],expected;unsigned j,status;uint8_t p;frozen_words reader;
for(j=0;j<k;++j)row=(uint16_t)(row+ra_steps(radial_indices[j]));
if(!radial_input_reference_begin(&reader,row))return 1;for(j=0;j<RA_WORDS;++j)if(!frozen_words_next(&reader,&w[j]))return 1;if(!frozen_words_finished(&reader))return 1;ra_from_words(&c,w);
token=enter();mail[8]=(uint8_t)(0x10u+i);status=(unsigned)ra_apply(&c);mail[8]=(uint8_t)(0x20u+i);leave(token);
__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u))++failures;
ra_words(&c,w);if(!radial_reference_begin(&reader,row))++failures;else{if(!frozen_words_next(&reader,&expected)||status!=expected)++failures;for(j=0;j<RA_WORDS;++j)if(!frozen_words_next(&reader,&expected)||w[j]!=expected)++failures;if(!frozen_words_finished(&reader))++failures;}}
token=enter();{uint8_t nested=enter(),p;(void)ra_apply(&c);leave(nested);__asm__ volatile("php\npla":"=a"(p)::"memory");if(!(p&4u))++failures;}leave(token);
mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)RADIAL_FROZEN_ROWS;mail[4]=(uint8_t)(RADIAL_FROZEN_ROWS>>8);mail[5]=3;mail[6]=(uint8_t)sizeof(radial_case);mail[7]=RA_WORDS;puts(failures?"RADIAL TESTS FAILED":"COMPLETE ASSEMBLY RADIAL STATE PASS");mail[0]=0xa5;return failures;}
