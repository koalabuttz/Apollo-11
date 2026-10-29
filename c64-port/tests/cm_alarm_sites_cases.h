/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_SITES_CASES_H
#define CM_ALARM_SITES_CASES_H
#include "cm_alarm_sites.h"
/* Case n: kind n%3 (ALARM, ALARM2, BAILOUT/POODOO word), site (n/3)%sites,
 * context n/(3*sites). Q (or ALMCADR) is the site's operand word under the
 * FB/SUPERBNK that address it; the rest comes from the legacy generators. */
#define CM_ALARM_SITES_CONTEXTS 64u
enum cm_alarm_sites_kind {CM_SITES_ALARM,CM_SITES_ALARM2,CM_SITES_ABORT};
uint32_t cm_alarm_sites_cases(void);
enum cm_alarm_sites_kind cm_alarm_sites_input(apollo_overlay_state *,cm_native_io *,uint32_t);
/* Native flight entry for case kind. */
enum exec_status cm_alarm_sites_run(apollo_overlay_state *,cm_native_io *,enum cm_alarm_sites_kind);
#endif
