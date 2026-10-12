/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_tvc_massprop_cases.h"
#define CM_TVC_MASSPROP_FROZEN_IMPLEMENTATION
#include "cm_tvc_massprop_vectors.h"
static agc_image image,expected;
/* Frozen oracle rows: status, IRQ and every changed image word. */
unsigned cm_tvc_massprop_check(void){unsigned row,fail=0;
for(row=0;row<CM_TVC_MASSPROP_FROZEN_ROWS;++row){frozen_words r;uint16_t low,high,status,irq_after,count,i,address,value,pad;agc_word irq;uint8_t ok=1;
 if(!cm_tvc_massprop_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)||!frozen_words_next(&r,&status)||!frozen_words_next(&r,&irq_after)||!frozen_words_next(&r,&count)||count>2048u){++fail;continue;}
 cm_tvc_massprop_input(&image,&irq,(uint32_t)low|(uint32_t)high<<16);expected=image;
 for(i=0;i<count;++i){if(!frozen_words_next(&r,&address)||!frozen_words_next(&r,&value)||address>=2048u){ok=0;break;}expected.words[address]=value;}
 while(ok&&frozen_words_next(&r,&pad))if(pad)ok=0;
 if(!ok||!frozen_words_finished(&r)||irq!=irq_after||(unsigned)cm_tvc_massprop(&image)!=status||(status==0u&&memcmp(&image,&expected,sizeof image)))++fail;}
return fail;}
#ifndef CM_TVC_MASSPROP_CHECK_ONLY
/* Every entry check refuses and leaves the image unchanged. */
static unsigned rejected(void){static const unsigned field[]={3,4,5,6};unsigned k,f=0;agc_word irq,w=0;
for(k=0;k<4u;++k){cm_tvc_massprop_input(&image,&irq,77);image.words[field[k]]^=02000u;expected=image;if(cm_tvc_massprop(&image)!=CM_TVC_MASSPROP_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;}
if(cm_tvc_massprop_fixed(03461u,&w)||cm_tvc_massprop_fixed(03521u,&w)||cm_tvc_massprop_fixed(03463u,0)||!cm_tvc_massprop_fixed(03435u,&w)||w!=051506u||!cm_tvc_massprop_fixed(03520u,&w)||w!=075420u)++f;
return f;}
int main(void){unsigned f=cm_tvc_massprop_check(),g=rejected();
printf("cm-tvc-massprop: %u frozen original MASSPROP/FIXCW states, entry refusals: %s\n",(unsigned)CM_TVC_MASSPROP_FROZEN_ROWS,f||g?"FAIL":"PASS");return f||g?1:0;}
#endif
