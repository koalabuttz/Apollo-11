/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_rcs_dap_cases.h"
#define CM_RCS_DAP_FROZEN_IMPLEMENTATION
#include "cm_rcs_dap_vectors.h"
static agc_image image,expected;
/* Frozen oracle rows: status, channels 12, 13 and 14 and every changed image word. */
unsigned cm_rcs_dap_check(void){unsigned row,fail=0;
for(row=0;row<CM_RCS_DAP_FROZEN_ROWS;++row){frozen_words r;uint16_t low,high,status,ch12,ch13,ch14,count,i,address,value,pad;cm_rcs_dap_io io;uint8_t ok=1;
 if(!cm_rcs_dap_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)||!frozen_words_next(&r,&status)||!frozen_words_next(&r,&ch12)||!frozen_words_next(&r,&ch13)||!frozen_words_next(&r,&ch14)||!frozen_words_next(&r,&count)||count>2048u){++fail;continue;}
 cm_rcs_dap_input(&image,&io,(uint32_t)low|(uint32_t)high<<16);expected=image;
 for(i=0;i<count;++i){if(!frozen_words_next(&r,&address)||!frozen_words_next(&r,&value)||address>=2048u){ok=0;break;}expected.words[address]=value;}
 while(ok&&frozen_words_next(&r,&pad))if(pad)ok=0;
 if(!ok||!frozen_words_finished(&r)||(unsigned)cm_rcs_dap(&image,&io)!=status||io.channel12!=ch12||io.channel13!=ch13||io.channel14!=ch14||memcmp(&image,&expected,sizeof image))++fail;}
return fail;}
#ifndef CM_RCS_DAP_CHECK_ONLY
/* Every entry check refuses and leaves the image and io unchanged: bank
 * registers, an unknown Z, the table indices out of range, and the TC Q
 * entries without a caller's return. */
static unsigned refuse(uint32_t n,unsigned address,agc_word value){cm_rcs_dap_io io,before;cm_rcs_dap_input(&image,&io,n);image.words[address]=value;expected=image;before=io;
 return cm_rcs_dap(&image,&io)!=CM_RCS_DAP_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&before,sizeof io);}
static unsigned rejected(void){unsigned f=0;
f+=refuse(1,3,03400u);f+=refuse(1,4,040000u);f+=refuse(1,6,042007u);f+=refuse(1,5,02106u);f+=refuse(1,5,03250u);
f+=refuse(0,03217,12u);f+=refuse(0,03217,077775u);f+=refuse(0,03217,077777u);f+=refuse(4,01130,7u);f+=refuse(4,01130,077776u);
f+=refuse(5,03106,3u);f+=refuse(5,03107,7u);f+=refuse(5,03217,13u);f+=refuse(6,03106,077776u);
f+=refuse(13,2,03247u);f+=refuse(13,2,03765u);f+=refuse(16,2,010000u);f+=refuse(18,2,02154u);f+=refuse(19,2,02732u);
return f;}
int main(void){unsigned f=cm_rcs_dap_check(),g=rejected();
printf("cm-rcs-dap: %u frozen original RCS-CSM_DIGITAL_AUTOPILOT states, entry refusals: %s\n",(unsigned)CM_RCS_DAP_FROZEN_ROWS,f||g?"FAIL":"PASS");return f||g?1:0;}
#endif
