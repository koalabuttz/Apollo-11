/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_auto_man_cases.h"
#define CM_AUTO_MAN_FROZEN_IMPLEMENTATION
#include "cm_auto_man_vectors.h"
static agc_image image,expected;
/* Frozen oracle rows: status, two zero words and every changed image word. */
unsigned cm_auto_man_check(void){unsigned row,fail=0;
for(row=0;row<CM_AUTO_MAN_FROZEN_ROWS;++row){frozen_words r;uint16_t low,high,status,ch14,zero,count,i,address,value,pad;cm_auto_man_io io;uint8_t ok=1;
 if(!cm_auto_man_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)||!frozen_words_next(&r,&status)||!frozen_words_next(&r,&ch14)||!frozen_words_next(&r,&zero)||!frozen_words_next(&r,&count)||count>2048u){++fail;continue;}
 cm_auto_man_input(&image,&io,(uint32_t)low|(uint32_t)high<<16);expected=image;
 for(i=0;i<count;++i){if(!frozen_words_next(&r,&address)||!frozen_words_next(&r,&value)||address>=2048u){ok=0;break;}expected.words[address]=value;}
 while(ok&&frozen_words_next(&r,&pad))if(pad)ok=0;
 if(!ok||!frozen_words_finished(&r)||(unsigned)cm_auto_man(&image,&io)!=status||ch14||zero||memcmp(&image,&expected,sizeof image))++fail;}
return fail;}
#ifndef CM_AUTO_MAN_CHECK_ONLY
/* Every entry check refuses and leaves the image and io unchanged: bank
 * registers, an unknown Z, and the STICKCHK and SMALLMP returns with an
 * index out of range. */
static unsigned refuse(uint32_t n,unsigned address,agc_word value){cm_auto_man_io io,before;cm_auto_man_input(&image,&io,n);image.words[address]=value;expected=image;before=io;
 return cm_auto_man(&image,&io)!=CM_AUTO_MAN_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&before,sizeof io);}
static unsigned rejected(void){unsigned f=0;
f+=refuse(1,3,03400u);f+=refuse(1,4,040000u);f+=refuse(1,6,042007u);f+=refuse(1,5,03251u);f+=refuse(1,5,03753u);
f+=refuse(5,03256,4u);f+=refuse(5,03257,077776u);f+=refuse(5,03260,4u);f+=refuse(6,03106,3u);f+=refuse(7,03106,077776u);
return f;}
int main(void){unsigned f=cm_auto_man_check(),g=rejected();
printf("cm-auto-man: %u frozen original AUTOMATIC_MANEUVERS states, entry refusals: %s\n",(unsigned)CM_AUTO_MAN_FROZEN_ROWS,f||g?"FAIL":"PASS");return f||g?1:0;}
#endif
