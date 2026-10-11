/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_keyboard_irq_check.h"
static apollo_overlay_state image,before;
int main(void){uint8_t n;uint16_t failures=0,isr,old_isr,i;cm_native_io io,old;
 for(n=0;n<CM_KEYBOARD_IRQ_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_keyboard_irq_check(n,&image,&before));
 cm_keyboard_irq_input(&image,&io,&isr,73);for(i=0;i<7;++i)image.words[0154+i*12u+11u]=010110;before=image;old=io;old_isr=isr;
 if(cm_keyboard_irq_run(&image,&io,&isr,73)!=EXEC_OUT_OF_SCOPE || memcmp(&image,&before,sizeof image) || memcmp(&io,&old,sizeof io) || isr!=old_isr)++failures;
 printf("CM keyboard IRQ:64 complete2051-field original states,%u failures\n",failures);return failures!=0;}
