/* SPDX-License-Identifier: MIT */
/* imu-compensation on the C64, against digests frozen from the original
 * programs (tools/imu_compensation_freeze.py). Built three times:
 *   -DICP_ENTRY_ONLY               every 37th entry case (both missions);
 *   -DICP_NO_EXEC -DICP_TRIP_CM    every 16th Comanche round trip,
 *   -DICP_NO_EXEC -DICP_TRIP_LM    every 16th plain Luminary round trip:
 *     BANKCALL through the native INTER-BANK routines and this file to
 *     SWRETURN and the caller, or to NOVAC.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "imu_compensation_cases.h"
#include "imu_compensation_c64_vectors.h"
static apollo_overlay_state image;
static uint16_t failures,checks;
static void one(uint16_t n,uint32_t expected){++checks;if(icp_digest(&image,n)!=expected)++failures;}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t n;
#ifdef ICP_ENTRY_ONLY
 for(n=0;n<ICP_C64_COUNT;++n)one((uint16_t)(n*ICP_C64_STRIDE),icp_c64[n]);
#elif defined ICP_TRIP_CM
 /* Comanche sites are 4 and 5 of each group of ICP_SITES. */
 {uint16_t i=0,k=0;for(n=ICP_TRIP_FIRST;n<ICP_PLAIN_FIRST;++n)if((n-ICP_TRIP_FIRST)%ICP_SITES>=4u){if(k%ICP_C64_TRIP_STRIDE==0u)one(n,icp_c64_cm_trips[i++]);++k;}}
#else
 for(n=0;n<ICP_C64_PLAIN_TRIPS;++n)one((uint16_t)(ICP_PLAIN_FIRST+n*ICP_C64_TRIP_STRIDE),icp_c64_plain[n]);
#endif
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
