/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_SITES_H
#define CM_ALARM_SITES_H
#include "agc_transfer.h"
#include "cm_alarm_complete.h"
#include "cm_alarm2_image.h"
#include "cm_abort_word.h"
/* Flight entries for Comanche055 ALARM_AND_ABORT inline-operand callers.
 * Every genuine `TC ALARM/ALARM2/BAILOUT/POODOO` site of the original program
 * is compiled into cm_alarm_sites (src/cm_alarm_sites.inc, generated from the
 * source by tools/agc_sites_generate.py cm-alarm). A fixed caller Q that is not
 * such a site is EXEC_OUT_OF_SCOPE: no original producer supplies it, and the
 * original rope is never read. Erasable inline words are read from the image. */
const agc_site_operand *cm_alarm_site_table(uint16_t *count);
/* The compiled inline code for logical caller address q, if q is in fixed
 * memory and a genuine site under the current FB/superbank. */
bool cm_alarm_site_code(const apollo_overlay_state *, const cm_native_io *, agc_word q, agc_word *code);
/* ALARM/VARALARM/CURTAINS/BORTENT through cm_alarm_complete. */
enum exec_status cm_alarm_site_call(apollo_overlay_state *, cm_native_io *, enum cm_alarm_record_entry);
/* Direct ALARM2 entry (05652) through cm_alarm2_image. */
enum exec_status cm_alarm2_site_call(apollo_overlay_state *, cm_native_io *);
/* Post-VAC5STOR BAILOUT 07762 / POODOO 05726 / CCSHOLE 05754 through cm_abort_word. */
enum exec_status cm_abort_site_call(apollo_overlay_state *, cm_native_io *);
#endif
