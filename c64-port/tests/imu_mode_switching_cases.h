/* SPDX-License-Identifier: MIT */
#ifndef IMU_MODE_SWITCHING_CASES_H
#define IMU_MODE_SWITCHING_CASES_H
#include "imu_mode_switching.h"
#include "cm_native_image.h"
/* imu-mode-switching cases. tools/imu_mode_switching_reference.c runs the
 * same image, channels and interrupt state from the same Z in the
 * unmodified rope.
 *   entry cases: every routine of lm_ and cm_imu_mode_switching_routines
 *     (72 + 72), IMS_ENTRY_STATES states each, run until control leaves the
 *     file (an external address of ims_externals, the subroutine caller's
 *     return at 04000, or an erasable address);
 *   round trips: every genuine TC BANKCALL or TC IBNKCALL site whose CADR
 *     is one of these routines (Luminary 74, Comanche 54), IMS_SITE_STATES
 *     states each, from BANKCALL/IBNKCALL through the native
 *     INTER-BANK_COMMUNICATION routines and this file back to the caller's
 *     Q +1, or to where native code ends (an external of this file other
 *     than the INTER-BANK entries or SETISSW).
 * Every case ends with the 2048 erasable words, the interrupt-enable state
 * and channels 11-14 compared with the original program. States are drawn
 * inside each routine's domain (include/imu_mode_switching.h); the
 * refusals are exercised by tests/test_imu_mode_switching.c. */
#define IMS_ROUTINES (IMS_LM_ROUTINES+IMS_CM_ROUTINES)
#define IMS_ENTRY_STATES 384u
#define IMS_ENTRY_CASES (IMS_ROUTINES*IMS_ENTRY_STATES)
#define IMS_LM_SITES 74u
#define IMS_CM_SITES 54u
#define IMS_SITES (IMS_LM_SITES+IMS_CM_SITES)
#define IMS_SITE_STATES 64u
#define IMS_TRIP_FIRST IMS_ENTRY_CASES
#define IMS_CASES (IMS_TRIP_FIRST+IMS_SITES*IMS_SITE_STATES)

#define IMS_STOPPED 0u
#define IMS_REFUSED 2u
#define IMS_ANY_FB 0177777u
typedef struct { agc_word z,fb; } ims_stop;
typedef struct {
 uint8_t cm;
 agc_word stop_z,stop_fb;   /* round trip: the caller's continuation; else 0 */
 agc_word superbank;
 uint16_t interrupt_enabled;
 agc_word channel[4];       /* 11, 12, 13, 14 */
} ims_case;
/* Stops (Z, FB) of a case other than its caller continuation, ending with
 * z 0. Any Z below 02000 also stops. */
const ims_stop *ims_externals(const ims_case *);
/* Erasable words a TC reaches as a return point the native code executes
 * (TC Q at 2, SCALPREP's TC MPAC +2 at 0156, INTER-BANK's TC BUF2 at 0133
 * and TC RUPTREG3 at 072); the oracle runs through them. Any other Z below
 * 02000 stops both programs. */
int ims_executed_erasable(agc_word z);
/* Mission (1 Comanche) and physical key of the routine case n enters,
 * without building its image; 0xffff when the case has no site. */
uint16_t ims_target(uint16_t n,uint8_t *cm);
void ims_input(apollo_overlay_state *, ims_case *, uint16_t n);
/* Runs case n natively; returns the kind (3: anything else), the final
 * interrupt-enable state and channels 11-14. */
unsigned ims_run(apollo_overlay_state *, uint16_t n, uint16_t *interrupt_enabled, agc_word channel[4]);
uint32_t ims_digest(apollo_overlay_state *, uint16_t n);
/* Routine r (0..143): Luminary 0..71, Comanche 72..143, table order. */
uint16_t ims_routine_physical(uint16_t r);
#endif
