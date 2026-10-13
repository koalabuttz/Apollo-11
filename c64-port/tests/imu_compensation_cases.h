/* SPDX-License-Identifier: MIT */
#ifndef IMU_COMPENSATION_CASES_H
#define IMU_COMPENSATION_CASES_H
#include "imu_compensation.h"
#include "p66_bias.h"
#include "cm_native_image.h"
/* imu-compensation cases. tools/imu_compensation_reference.c runs the same
 * image from the same Z in the unmodified rope.
 *   entry cases: every routine of lm_ and cm_imu_compensation_routines
 *     (11 + 9), ICP_ENTRY_STATES random states each, run until control
 *     leaves the file (SWRETURN, BANKCALL, IBNKCALL, NOVAC or ENDOFJOB);
 *   round trips: every genuine TC BANKCALL site of 1/PIPA and GCOMPZER
 *     (LM 4, CM 2), ICP_SITE_STATES states each, from BANKCALL through the
 *     native INTER-BANK_COMMUNICATION routines, this file, NOVAC
 *     (lm_exec_routines or cm_exec_routines, canonical Executive) and
 *     SWRETURN back to the caller's Q+1, or to NOVAC's alarms;
 *   plain round trips: every site again (ICP_PLAIN_STATES each) without
 *     the Executive, stopping at NOVAC, so the C64 diagnostics of round
 *     trips fit without the Executive routines.
 * Every case ends with the 2048 erasable words and the interrupt-enable
 * state compared with the original program. */
#define ICP_ROUTINES 20u
#define ICP_ENTRY_STATES 512u
#define ICP_ENTRY_CASES (ICP_ROUTINES*ICP_ENTRY_STATES)
#define ICP_SITES 6u
#define ICP_SITE_STATES 512u
#define ICP_LM_SITES 4u
#define ICP_TRIP_FIRST ICP_ENTRY_CASES
#define ICP_PLAIN_FIRST (ICP_TRIP_FIRST+ICP_SITES*ICP_SITE_STATES)
#define ICP_PLAIN_STATES 128u
#define ICP_CASES (ICP_PLAIN_FIRST+ICP_SITES*ICP_PLAIN_STATES)
#define ICP_LM_ROUTINES 11u

#define ICP_STOPPED 0u     /* reached the case's stop or an external address */
#define ICP_REFUSED 2u
typedef struct {
 uint8_t cm;               /* 0 Luminary099, 1 Comanche055 */
 uint8_t exec;             /* round trip through native NOVAC */
 agc_word stop_z,stop_fb;  /* round trip: the caller's continuation; else 0 */
 agc_word superbank;
 uint16_t interrupt_enabled;
} icp_case;
/* External stop addresses (fixed-fixed) of an entry case, 0-terminated. */
const agc_word *icp_externals(const icp_case *);
void icp_input(apollo_overlay_state *, icp_case *, uint16_t n);
/* Runs case n natively; returns the kind (3: anything else) and the final
 * interrupt-enable state. */
unsigned icp_run(apollo_overlay_state *, uint16_t n, uint16_t *interrupt_enabled);
uint32_t icp_digest(apollo_overlay_state *, uint16_t n);
#endif
