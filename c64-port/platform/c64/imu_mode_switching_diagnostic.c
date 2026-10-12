/* SPDX-License-Identifier: MIT */
/* imu-mode-switching on the C64, against digests frozen from the original
 * programs (tools/imu_mode_switching_freeze.py). Built for each third of the
 * routines (-DIMS_C64_PART=1, 2 or 3, include/imu_mode_switching.h) three
 * times, each checking the cases that enter a routine of that third:
 *   -DIMS_ENTRY_ONLY   every 73rd entry case (both missions);
 *   -DIMS_TRIP_LM      every 11th Luminary round trip,
 *   -DIMS_TRIP_CM      every 11th Comanche round trip: BANKCALL or IBNKCALL
 *     through the native INTER-BANK routines and this file back to the
 *     caller, or to where native code ends.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "imu_mode_switching_cases.h"
#include "imu_mode_switching_c64_vectors.h"
static apollo_overlay_state image;
static uint16_t failures,checks;
static int linked(uint16_t n){uint8_t cm;uint16_t i,key=ims_target(n,&cm);const agc_routine_table *t;
 if(cm>1u)return 0;
 t=cm?&cm_imu_mode_switching_routines:&lm_imu_mode_switching_routines;
 for(i=0;i<t->count;++i)if(t->entries[i].physical==key)return 1;
 return 0;
}
static void one(uint16_t n,uint32_t expected){if(!linked(n))return;++checks;if(ims_digest(&image,n)!=expected)++failures;}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t n;
#ifdef IMS_ENTRY_ONLY
 for(n=0;n<IMS_C64_COUNT;++n)one((uint16_t)(n*IMS_C64_STRIDE),ims_c64[n]);
#else
 /* Luminary sites are 0..IMS_LM_SITES-1 of each group of IMS_SITES. */
 {uint16_t i=0,k=0;for(n=IMS_TRIP_FIRST;n<IMS_CASES;++n){
#ifdef IMS_TRIP_LM
  if((n-IMS_TRIP_FIRST)%IMS_SITES<IMS_LM_SITES){
#else
  if((n-IMS_TRIP_FIRST)%IMS_SITES>=IMS_LM_SITES){
#endif
   if(k%IMS_C64_TRIP_STRIDE==0u)one(n,ims_c64_trips[i++]);++k;}}}
#endif
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
