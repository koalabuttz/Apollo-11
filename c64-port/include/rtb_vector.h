/* SPDX-License-Identifier: MIT */
#ifndef RTB_VECTOR_H
#define RTB_VECTOR_H
#include "rtb_angles.h"
enum rtb_vector_entry {RTB_VECAGREE=0,RTB_VECSGNAG=1,RTB_NORMUNIT_FRONT=2,RTB_NORMUNX1_FRONT=3,RTB_PULSEIMU_FRONT=4};
/* Nonnull canonical image/io; ordinary15-bit memory/raw16 A/L, coherent banks.
 * Direct VECAGREE completes TC A at fixed incoming Q, without fetching caller.
 * VECSGNAG composes literal BANKCALL/VECAGREE/SWRETURN through DANZIG entry.
 * NORM fronts execute source sign agreement and tiny-vector prescale, then
 * literal POSTJUMP to UNIT+1; UNIT body remains external. PULSE front performs
 * literal BANKCALL to actual IMUPULSE entry; no gyro/hardware body is claimed.
 * RTB fronts require FIXLOC one of five source VAC bases. Exclude producers
 * throughout these basic computations; no scheduler check is introduced. */
enum rtb_status rtb_vector(apollo_overlay_state *,p66_bias_io *,enum rtb_mission,enum rtb_vector_entry);
#endif
