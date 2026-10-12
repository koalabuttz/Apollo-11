/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_tvc_restarts_cases.h"
#define CM_TVC_RESTARTS_FROZEN_IMPLEMENTATION
#include "cm_tvc_restarts_vectors.h"
static agc_image image,expected;
/* Frozen oracle rows: status, channels 12/14 and every changed image word. */
unsigned cm_tvc_restarts_check(void){unsigned row,fail=0;
for(row=0;row<CM_TVC_RESTARTS_FROZEN_ROWS;++row){frozen_words r;uint16_t low,high,status,ch12,ch14,count,i,address,value,pad;cm_tvc_restarts_io io;uint8_t ok=1;
 if(!cm_tvc_restarts_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)||!frozen_words_next(&r,&status)||!frozen_words_next(&r,&ch12)||!frozen_words_next(&r,&ch14)||!frozen_words_next(&r,&count)||count>2048u){++fail;continue;}
 cm_tvc_restarts_input(&image,&io,(uint32_t)low|(uint32_t)high<<16);expected=image;
 for(i=0;i<count;++i){if(!frozen_words_next(&r,&address)||!frozen_words_next(&r,&value)||address>=2048u){ok=0;break;}expected.words[address]=value;}
 while(ok&&frozen_words_next(&r,&pad))if(pad)ok=0;
 if(!ok||!frozen_words_finished(&r)||(unsigned)cm_tvc_restarts(&image,&io)!=status||io.channel12!=ch12||io.channel14!=ch14||memcmp(&image,&expected,sizeof image))++fail;}
return fail;}
#ifndef CM_TVC_RESTARTS_CHECK_ONLY
/* Every entry check refuses and leaves the image and channels unchanged:
 * bank registers, an unknown Z, EXRSTRT with TVCEXPHS outside 1..6, and
 * REDOTVC reaching FINCOPY with TVCPHASE past the TVCCADR table. */
static unsigned refuse(uint32_t n,unsigned address,agc_word value){cm_tvc_restarts_io io,before;cm_tvc_restarts_input(&image,&io,n);image.words[address]=value;if(address==03244u)image.words[03251]=0;expected=image;before=io;
 return cm_tvc_restarts(&image,&io)!=CM_TVC_RESTARTS_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&before,sizeof io);}
static unsigned rejected(void){unsigned f=0;
f+=refuse(77,3,03400u);f+=refuse(77,4,036000u);f+=refuse(77,6,034007u);f+=refuse(77,5,03137u);
f+=refuse(5,03251,0u);f+=refuse(5,03251,7u);f+=refuse(5,03251,077776u);
f+=refuse(0,03244,013u);f+=refuse(0,03244,037777u);
return f;}
int main(void){unsigned f=cm_tvc_restarts_check(),g=rejected();
printf("cm-tvc-restarts: %u frozen original TVCRESTARTS states, entry refusals: %s\n",(unsigned)CM_TVC_RESTARTS_FROZEN_ROWS,f||g?"FAIL":"PASS");return f||g?1:0;}
#endif
