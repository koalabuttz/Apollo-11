/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_entry_dap_cases.h"
#define CM_ENTRY_DAP_FROZEN_IMPLEMENTATION
#include "cm_entry_dap_vectors.h"
static agc_image image,expected;
/* Frozen oracle rows: status, channels 5 and 6 and every changed image word. */
unsigned cm_entry_dap_check(void){unsigned row,fail=0;
for(row=0;row<CM_ENTRY_DAP_FROZEN_ROWS;++row){frozen_words r;uint16_t low,high,status,ch5,ch6,count,i,address,value,pad;cm_entry_dap_io io;uint8_t ok=1;
 if(!cm_entry_dap_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)||!frozen_words_next(&r,&status)||!frozen_words_next(&r,&ch5)||!frozen_words_next(&r,&ch6)||!frozen_words_next(&r,&count)||count>2048u){++fail;continue;}
 cm_entry_dap_input(&image,&io,(uint32_t)low|(uint32_t)high<<16);expected=image;
 for(i=0;i<count;++i){if(!frozen_words_next(&r,&address)||!frozen_words_next(&r,&value)||address>=2048u){ok=0;break;}expected.words[address]=value;}
 while(ok&&frozen_words_next(&r,&pad))if(pad)ok=0;
 if(!ok||!frozen_words_finished(&r)||(unsigned)cm_entry_dap(&image,&io)!=status||io.channel5!=ch5||io.channel6!=ch6||memcmp(&image,&expected,sizeof image))++fail;}
return fail;}
#ifndef CM_ENTRY_DAP_CHECK_ONLY
/* Every entry check refuses and leaves the image and io unchanged: bank
 * registers (including the other bank's FB), an unknown Z, SW/NDX outside
 * -13..+13, JETAG outside -1..+1 or -0, and CMDAPMOD outside -1..+1. */
static unsigned refuse(uint32_t n,unsigned address,agc_word value){cm_entry_dap_io io,before;cm_entry_dap_input(&image,&io,n);image.words[address]=value;expected=image;before=io;
 return cm_entry_dap(&image,&io)!=CM_ENTRY_DAP_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&before,sizeof io);}
static unsigned rejected(void){unsigned f=0;
f+=refuse(12,3,03400u);f+=refuse(12,4,040000u);f+=refuse(12,6,032007u);f+=refuse(26,4,032000u);f+=refuse(26,6,032006u);
f+=refuse(12,5,02435u);f+=refuse(12,5,03675u);f+=refuse(26,5,03613u);
f+=refuse(12,0305,14u);f+=refuse(12,0305,077761u);f+=refuse(12,03311,2u);f+=refuse(12,03311,077777u);f+=refuse(19,03311,077775u);
f+=refuse(19,03300,2u);f+=refuse(19,03300,077775u);
return f;}
int main(void){unsigned f=cm_entry_dap_check(),g=rejected();
printf("cm-entry-dap: %u frozen original CM_ENTRY_DIGITAL_AUTOPILOT states, entry refusals: %s\n",(unsigned)CM_ENTRY_DAP_FROZEN_ROWS,f||g?"FAIL":"PASS");return f||g?1:0;}
#endif
