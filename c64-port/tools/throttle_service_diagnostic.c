/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "throttle_service_vectors.h"
unsigned throttle_service_check(void);
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned failures;mail[0]=0;failures=throttle_service_check();mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)THROTTLE_SERVICE_FROZEN_ROWS;mail[4]=(uint8_t)(THROTTLE_SERVICE_FROZEN_ROWS>>8);mail[0]=0xa5;return failures?1:0;}
