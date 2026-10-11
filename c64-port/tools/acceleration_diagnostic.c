/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "acceleration_vectors.h"
unsigned acceleration_check(void);
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned failures;mail[0]=0;failures=acceleration_check();mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)ACCELERATION_FROZEN_ROWS;mail[4]=(uint8_t)(ACCELERATION_FROZEN_ROWS>>8);mail[0]=0xa5;return failures?1:0;}
