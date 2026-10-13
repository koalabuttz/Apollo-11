# Lane C: Pinball CHARIN split at its key dispatch (lanec-pin-keys)

Luminary099 CHARIN (40,2077) and Comanche055 CHARIN (40,2000) handle every
DSKY keypress: they take the display lock, light KEY REL when CADRSTOR holds
a flashing display, and jump through a table on the key code. The earlier
key modules (`src/dsky_charin_*.c`, `src/cm_charin_*.c`, `src/dsky_enter.c`)
each run CHARIN inline from its entry, so the transfer-ABI routine at CHARIN
had to try all of them, and its flight-store closure linked every key's
module (21 KB LM, 16 KB CM): more than the 16 KB job slot.

`src/lanec_pin_keys.c` splits that path at the dispatch:

- `lanec_pin_keys_charin` runs CHARIN from its entry to the key's label, as
  the original does: CAF ONE, XCH DSPLOCK, TS 21/22REG, CCS CADRSTOR (all
  four branches; ELRCODE1 skips the light, -0 lands on AD MPAC), RELDSPON's
  WOR of BIT5 into channel 11, XCH MPAC, TS CHAR, INDEX A, TC +1 and the
  key's table word. It stops at CHARALRM, NUM, 89TEST, NUM -2, VERB, ERROR,
  VBRELDSP, POSGN, NEGSGN, ENTERJMP, CLEAR or NOUN with Q past that table
  word. Any key code 0-037 is in its domain.
- `lanec_pin_keys_handler` runs from each of those labels. VERB and NOUN
  (`lanec_pw_nv_key_run`), VBRELDSP (`lanec_pw_operator_release_run`) and
  CHARALRM (`lanec_pw_source_alarm_run`) already have modules entered at
  the label, and they are tried first on any state. Otherwise the state must
  be one CHARIN's dispatch leaves for a key of that label (CHAR, A, Q,
  DSPLOCK = 1, MPAC and channel 11 as CHARIN sets them from CADRSTOR); the
  routine rebuilds the state CHARIN started from (Z = CHARIN, DSPLOCK =
  21/22REG, MPAC = the key) and runs the earlier key module from there. Its
  inline CHARIN redoes exactly what the head did: the lock and MPAC get the
  same values and KEY REL is already on. Anything else is refused unchanged.

## Suites

`pin-keys` (CHARIN) and `pin-key-labels` (the labels) draw their states from
the CHARIN cases of the earlier key suites (pw-charin-alarm, -clear,
-number, -nv, -release, -reset, -sign, pw-cm-charin-number, -nv and
pw-enter), 256 per source. One source rewrites the key to the alarm codes
12-17, 23-30 and 35. Every fourth pair sets CADRSTOR to +0, -0 or another
full value, so every CCS branch runs for every key. A label case is the same
state after the native CHARIN head. Both suites compare complete results
with the original program in Virtual AGC (`make test-lanec-pin-keys`):
2718 cases each match, and 98 are refused unchanged, all outside the earlier
modules' domains.

## Flight store

`tools/lanec/routines.py pin` gives CHARIN and the key labels to the two
suites alone (no other module is tried there) and writes their routines in
two generated files, each one flight module:

- `src/lanec_pin_keys_num_routines.c`: number entry, i.e. CHARIN, NUM, NUM -2
  and 89TEST (`lanec_pin_keys_num_{lm,cm}_routines`);
- `src/lanec_pin_keys_cmd_routines.c`: the other keys' labels
  (`lanec_pin_keys_cmd_{lm,cm}_routines`).

Both run through the shared trampoline `src/lanec_pin_trampoline.c` (moved
out of `src/lanec_pin_routines.c`, unchanged), so their links carry neither
the rest of Pinball nor its tables. In the whole-file source maps
(`tools/lanec/wholefile_pin.py`), pin-keys owns the words its own code runs
(CHARIN through the dispatch table, and RELDSPON); the words past the labels
stay with the earlier modules that carry them.
