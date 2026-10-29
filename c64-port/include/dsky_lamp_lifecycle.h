/* SPDX-License-Identifier: MIT */
#ifndef DSKY_LAMP_LIFECYCLE_H
#define DSKY_LAMP_LIFECYCLE_H
#include "dsky_lamp_release.h"
#include "dsky_mode_admit.h"
#include "lm_waitlist_image.h"
#include "cm_waitlist_extended.h"
enum dsky_lamp_lifecycle_route {DSKY_LAMP_LIFECYCLE_END,DSKY_LAMP_LIFECYCLE_TASKOVER,DSKY_LAMP_LIFECYCLE_PIN,DSKY_LAMP_LIFECYCLE_BLANK,DSKY_LAMP_LIFECYCLE_ADMISSION_ALARM,DSKY_LAMP_LIFECYCLE_WAIT_ALARM};
/* Uninterrupted actual VBTSTLTS/TSTLTS1→WAITLIST→END;
 * TSTLTS2→NOVAC→TASKOVER; TSTLTS3→BANKCALL DSPMM→NOVAC→SWRETURN
 * →KILMONON/FLASHOFF/POSTJUMP→RELDSP/JOBWAKE→END/PIN/BLNKSUB1,
 * or genuine source alarm entry. Literal requests, no added poll.
 * Branch-sensitive frozen scheduler/wake domains; see source evidence. */
enum p66_bias_status dsky_lamp_lifecycle(apollo_overlay_state *,dsky_lamp_io *,enum dsky_lamp_lifecycle_route *);
enum exec_status cm_lamp_lifecycle(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light,enum dsky_lamp_lifecycle_route *);
#endif
