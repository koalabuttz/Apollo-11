/* SPDX-License-Identifier: MIT */
#ifndef P66_BIAS_H
#define P66_BIAS_H
#include "overlay_runtime.h"
typedef struct {uint16_t superbank,interrupt_enabled;} p66_bias_io;
enum p66_bias_status {P66_BIAS_OK=0,P66_BIAS_OUT_OF_SCOPE=1};
/* Fixed STRTP66A initialization through VRTSTART entry. Native arithmetic,
 * explicit source stores and interpreter boundary bookkeeping; no decoder.
 * Image is the sole authoritative erasable state, producers excluded, source
 * bank31/EB7, NEWJOB=0, active core0 with owned VAC/FIXLOC/PUSHLOC at its base.
 * Includes bias construction, RODFLAG, old-PIPA/DELVROD initialization,
 * RODSCAL1, LASTTPIP, FCOLD and FWEIGHT. STOPRATE and display are separate. */
enum p66_bias_status p66_bias_start(apollo_overlay_state *,p66_bias_io *);
#endif
