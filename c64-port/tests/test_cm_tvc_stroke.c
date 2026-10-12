/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_tvc_stroke_cases.h"
#define CM_TVC_STROKE_FROZEN_IMPLEMENTATION
#include "cm_tvc_stroke_vectors.h"
static agc_image image,expected;
/* Frozen oracle rows: status, channel 14, interrupt enable and every changed image word. */
unsigned cm_tvc_stroke_check(void){unsigned row,fail=0;
for(row=0;row<CM_TVC_STROKE_FROZEN_ROWS;++row){frozen_words r;uint16_t low,high,status,ch14,irq,count,i,address,value,pad;cm_tvc_stroke_io io;uint8_t ok=1;
 if(!cm_tvc_stroke_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)||!frozen_words_next(&r,&status)||!frozen_words_next(&r,&ch14)||!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&count)||count>2048u){++fail;continue;}
 cm_tvc_stroke_input(&image,&io,(uint32_t)low|(uint32_t)high<<16);expected=image;
 for(i=0;i<count;++i){if(!frozen_words_next(&r,&address)||!frozen_words_next(&r,&value)||address>=2048u){ok=0;break;}expected.words[address]=value;}
 while(ok&&frozen_words_next(&r,&pad))if(pad)ok=0;
 if(!ok||!frozen_words_finished(&r)||(unsigned)cm_tvc_stroke(&image,&io)!=status||io.channel14!=ch14||io.interrupt_enabled!=irq||memcmp(&image,&expected,sizeof image))++fail;}
return fail;}
#ifndef CM_TVC_STROKE_CHECK_ONLY
/* Every entry check refuses and leaves the image and io unchanged: bank
 * registers, an unknown Z, and HACK/HACKWLST with CARD outside -4..-1. */
static unsigned refuse(uint32_t n,unsigned address,agc_word value){cm_tvc_stroke_io io,before;cm_tvc_stroke_input(&image,&io,n);image.words[address]=value;expected=image;before=io;
 return cm_tvc_stroke(&image,&io)!=CM_TVC_STROKE_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&before,sizeof io);}
static unsigned rejected(void){unsigned f=0;
f+=refuse(77,3,03400u);f+=refuse(77,4,036000u);f+=refuse(77,6,040007u);f+=refuse(77,5,03502u);
f+=refuse(1,03261,077777u);f+=refuse(2,03261,0u);f+=refuse(3,03261,077772u);
return f;}
int main(void){unsigned f=cm_tvc_stroke_check(),g=rejected();
printf("cm-tvc-stroke: %u frozen original TVCSTROKETEST states, entry refusals: %s\n",(unsigned)CM_TVC_STROKE_FROZEN_ROWS,f||g?"FAIL":"PASS");return f||g?1:0;}
#endif
