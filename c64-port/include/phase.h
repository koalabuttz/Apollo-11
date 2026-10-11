/* SPDX-License-Identifier: MIT */
#ifndef PHASE_H
#define PHASE_H
#include "restart.h"
enum phase_scratch_word { PH_TEMPG, PH_TEMPP, PH_TEMPNM, PH_TEMPBB,
 PH_TEMPSW, PH_TEMPSW2, PH_TEMPPR, PH_TEMPG2, PH_TEMPP2, PH_TEMPBBCN,
 PH_SCRATCH_COUNT };
typedef struct {
 restart_phases records;
 agc_word tbase[6], longbase[2], time[2]; /* TIME2, TIME1 */
 agc_word scratch[PH_SCRATCH_COUNT];
 agc_register a, l;
 agc_word q, bbank, superbank, interrupt_enabled;
} phase_state;
enum phase_status { PHASE_OK, PHASE_OUT_OF_SCOPE };
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
enum phase_status phase_change(phase_state *, const agc_word *, uint8_t, uint8_t);
#endif
