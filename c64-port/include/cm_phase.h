/* SPDX-License-Identifier: MIT */
#ifndef CM_PHASE_H
#define CM_PHASE_H
#include "cm_restart.h"
enum cm_phase_scratch_word { CM_PH_TEMPG, CM_PH_TEMPP, CM_PH_TEMPNM, CM_PH_TEMPBB,
 CM_PH_TEMPSW, CM_PH_TEMPSW2, CM_PH_TEMPPR, CM_PH_TEMPG2, CM_PH_TEMPP2, CM_PH_TEMPBBCN,
 CM_PH_SCRATCH_COUNT };
typedef struct {
 cm_restart_phases records;
 agc_word tbase[6], longbase[2], time[2]; /* TIME2, TIME1 */
 agc_word scratch[CM_PH_SCRATCH_COUNT];
 agc_register a, l;
 agc_word q, bbank, superbank, interrupt_enabled;
} cm_phase_state;
enum cm_phase_status { CM_PHASE_OK, CM_PHASE_OUT_OF_SCOPE };
/* PHASCHNG / 2PHSCHNG through original final RELINT/DTCB. Parameters are
 * the original inline words in their source order (up to five words).
 * Groups 1..6 only; 2PHSCHNG's first word must be Type A. Q points to the
 * first inline word; caller supplies a stable stream, counters and exclusion.
 * Stored state words are normal 15-bit values; BBANK uses bits 15..11 and
 * 3..1, channel-7 SUPERBNK uses bits 7..5. A/L inputs are overwritten.
 * State, parameter stream and other live memory must be distinct.
 * Supports Type A/B/C table maintenance; this does not execute a restart.
 * The observed A/L return debris retains original assembly addresses.
 * Invalid words, group, stream length or register input change no state. */
enum cm_phase_status cm_phase_change(cm_phase_state *, const agc_word *, uint8_t, uint8_t);
#endif
