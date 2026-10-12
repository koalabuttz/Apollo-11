/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_tvc_init_cases.h"
#define CM_TVC_INIT_FROZEN_IMPLEMENTATION
#include "cm_tvc_init_vectors.h"
static agc_image image,expected;
/* Frozen oracle rows: status, channel 11, interrupt enable and every changed image word. */
unsigned cm_tvc_init_check(void){unsigned row,fail=0;
for(row=0;row<CM_TVC_INIT_FROZEN_ROWS;++row){frozen_words r;uint16_t low,high,status,ch11,irq,count,i,address,value,pad;cm_tvc_init_io io;uint8_t ok=1;
 if(!cm_tvc_init_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)||!frozen_words_next(&r,&status)||!frozen_words_next(&r,&ch11)||!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&count)||count>2048u){++fail;continue;}
 cm_tvc_init_input(&image,&io,(uint32_t)low|(uint32_t)high<<16);expected=image;
 for(i=0;i<count;++i){if(!frozen_words_next(&r,&address)||!frozen_words_next(&r,&value)||address>=2048u){ok=0;break;}expected.words[address]=value;}
 while(ok&&frozen_words_next(&r,&pad))if(pad)ok=0;
 if(!ok||!frozen_words_finished(&r)||(unsigned)cm_tvc_init(&image,&io)!=status||io.channel11!=ch11||io.interrupt_enabled!=irq||memcmp(&image,&expected,sizeof image))++fail;}
return fail;}
#ifndef CM_TVC_INIT_CHECK_ONLY
/* Every entry check refuses and leaves the image and io unchanged: bank
 * registers, an unknown Z, and S40.15 with Q outside 4000-7777. */
static unsigned refuse(uint32_t n,unsigned address,agc_word value){cm_tvc_init_io io,before;cm_tvc_init_input(&image,&io,n);image.words[address]=value;expected=image;before=io;
 return cm_tvc_init(&image,&io)!=CM_TVC_INIT_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&before,sizeof io);}
static unsigned rejected(void){unsigned f=0;
f+=refuse(77,3,03400u);f+=refuse(77,4,034000u);f+=refuse(77,6,036007u);f+=refuse(77,5,02150u);
f+=refuse(8,2,03777u);f+=refuse(8,2,010000u);
return f;}
int main(void){unsigned f=cm_tvc_init_check(),g=rejected();
printf("cm-tvc-init: %u frozen original TVCINITIALIZE states, entry refusals: %s\n",(unsigned)CM_TVC_INIT_FROZEN_ROWS,f||g?"FAIL":"PASS");return f||g?1:0;}
#endif
