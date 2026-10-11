/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_charin_nv_check.h"
static apollo_overlay_state image,before;
int main(void){uint8_t n;uint16_t failures=0,wrote,old_wrote;cm_native_io io,old;
 for(n=0;n<CM_CHARIN_NV_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_charin_nv_check(n,&image,&before));
 cm_charin_nv_input(&image,&io,&wrote,73);image.words[0154]=034;before=image;old=io;old_wrote=wrote;
 if(cm_charin_nv(&image,&io,&wrote)!=EXEC_OUT_OF_SCOPE || memcmp(&image,&before,sizeof image) || memcmp(&io,&old,sizeof io) || wrote!=old_wrote)++failures;
 printf("CM VERB/NOUN entry:64 complete2052-field source states,%u failures\n",failures);return failures!=0;}
