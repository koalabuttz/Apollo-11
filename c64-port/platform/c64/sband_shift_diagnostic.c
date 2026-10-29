/* SPDX-License-Identifier: MIT */
#include "sband_shift_target.h"
int main(void){volatile unsigned char *box=(volatile unsigned char *)0xc000;unsigned completed;int status;
 __asm__("sei");*(volatile unsigned char *)1u&=0xfeu;box[0]=0xff;
 status=sband_shift_target_check(&completed);box[1]=(unsigned char)completed;box[2]=(unsigned char)(completed>>8);box[3]=(unsigned char)status;box[0]=(unsigned char)(status?0xee:0xa5);for(;;){}
}
