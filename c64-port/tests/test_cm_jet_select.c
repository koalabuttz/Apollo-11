/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_jet_select_cases.h"
#define CM_JET_SELECT_FROZEN_IMPLEMENTATION
#include "cm_jet_select_vectors.h"
static agc_image image,expected;
/* Frozen oracle rows: status, channels 5, 6 and 13 and every changed image word. */
unsigned cm_jet_select_check(void){unsigned row,fail=0;
for(row=0;row<CM_JET_SELECT_FROZEN_ROWS;++row){frozen_words r;uint16_t low,high,status,ch5,ch6,ch13,count,i,address,value,pad;cm_jet_select_io io;uint8_t ok=1;
 if(!cm_jet_select_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)||!frozen_words_next(&r,&status)||!frozen_words_next(&r,&ch5)||!frozen_words_next(&r,&ch6)||!frozen_words_next(&r,&ch13)||!frozen_words_next(&r,&count)||count>2048u){++fail;continue;}
 cm_jet_select_input(&image,&io,(uint32_t)low|(uint32_t)high<<16);expected=image;
 for(i=0;i<count;++i){if(!frozen_words_next(&r,&address)||!frozen_words_next(&r,&value)||address>=2048u){ok=0;break;}expected.words[address]=value;}
 while(ok&&frozen_words_next(&r,&pad))if(pad)ok=0;
 if(!ok||!frozen_words_finished(&r)||(unsigned)cm_jet_select(&image,&io)!=status||io.channel5!=ch5||io.channel6!=ch6||io.channel13!=ch13||memcmp(&image,&expected,sizeof image))++fail;}
return fail;}
#ifndef CM_JET_SELECT_CHECK_ONLY
/* Every entry check refuses and leaves the image and io unchanged: bank
 * registers, an unknown Z, XTRANS outside -4..+4 or -0, and a REPLACE
 * offset other than 0, 4 or 11. */
static unsigned refuse(uint32_t n,unsigned address,agc_word value){cm_jet_select_io io,before;cm_jet_select_input(&image,&io,n);image.words[address]=value;expected=image;before=io;
 return cm_jet_select(&image,&io)!=CM_JET_SELECT_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&before,sizeof io);}
static unsigned rejected(void){unsigned f=0;
f+=refuse(0,3,03400u);f+=refuse(0,4,042000u);f+=refuse(0,6,036007u);f+=refuse(0,5,02553u);f+=refuse(0,5,03616u);
f+=refuse(0,03231,5u);f+=refuse(1,03231,077772u);f+=refuse(2,03231,077777u);
f+=refuse(3,03060,1u);f+=refuse(3,03062,012u);f+=refuse(3,03064,077777u);
return f;}
int main(void){unsigned f=cm_jet_select_check(),g=rejected();
printf("cm-jet-select: %u frozen original JET_SELECTION_LOGIC states, entry refusals: %s\n",(unsigned)CM_JET_SELECT_FROZEN_ROWS,f||g?"FAIL":"PASS");return f||g?1:0;}
#endif
