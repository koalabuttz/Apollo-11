/* SPDX-License-Identifier: MIT */
/* cm-t4rupt-imu on the C64: every sixteenth Comanche055 IMU monitor case
 * (one routine) and the cases that complete the branch outcomes, against
 * digests frozen from the original program
 * (tools/cm_t4rupt_imu_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_t4rupt_imu_cases.h"
#include "cm_t4rupt_imu_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<CTI_C64_COUNT;++n){++checks;if(cti_digest(&image,cti_c64_case[n])!=cti_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
