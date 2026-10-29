/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "cm_waitlist_image_check.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned failures;
 mail[0]=0;failures=cm_waitlist_image_check();mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=128;mail[4]=0;mail[0]=0xa5;for(;;){}return 0;
}
