/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PIN_KEYS_H
#define LANEC_PIN_KEYS_H
#include "lanec.h"
/* Pinball CHARIN split at its key dispatch (both missions), so the keypress
 * path runs as small routines instead of one routine that links every key's
 * module (docs/LANEC-PIN-KEYS.md).
 *   charin   CHARIN (bank 40), key code 0-037 in MPAC: the display lock, the
 *            key-release light (RELDSPON) when CADRSTOR is full, and the
 *            INDEX/TC dispatch -> the key's label (CHARALRM, NUM, 89TEST,
 *            NUM -2, VERB, ERROR, VBRELDSP, POSGN, NEGSGN, ENTERJMP, CLEAR,
 *            NOUN) with Q past its table word.
 *   labels   each of those labels as CHARIN's dispatch leaves it: the state
 *            CHARIN started from is rebuilt (Z, DSPLOCK, MPAC) and the
 *            earlier key module runs from there (src/dsky_charin_*.c and
 *            the CM twins); VERB, NOUN and CHARALRM try their own
 *            label modules first. A state CHARIN cannot leave is refused.
 * Out-of-scope input returns LANEC_OUT_OF_SCOPE unchanged. */
enum lanec_status lanec_pin_keys_charin(apollo_overlay_state *, lanec_io *, enum lanec_mission);
/* The flight groups, one module each: number entry (CHARIN, NUM, NUM -2,
 * 89TEST), VBRELDSP, POSGN with NEGSGN and ERROR, CLEAR, and VERB, NOUN,
 * ENTERJMP and CHARALRM. dispatch is all of them, for the suites. */
enum lanec_status lanec_pin_keys_num(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_keys_release(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_keys_sign(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_keys_clear(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_keys_cmd(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_keys_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
