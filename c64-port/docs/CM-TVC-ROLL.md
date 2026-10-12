# Translate Comanche TVCROLLDAP (CM roll DAP during TVC)

Lane G. Milestone `cm-tvc-roll`.

## Scope

`Comanche055/TVCROLLDAP.agc`, all of it: bank 16 `3272`-`3717`, 278
assembled words (including the NOROL1T6 2CADR and the DB, -SLOPE, LMCRATE,
INTERCEP, MINLIM, 1/MINLIM, MAXLIM, TMINFIRE, TMAXFIRE and -ROLL2
constants). Nothing is left in the file.

`cm_tvc_roll(image, io)` runs one task to its exit. `io` holds channel 6
(roll jets, WRITE) and channel 13 (WOR of bit 15, the T6 enable):

| Entry | Reached from |
|---|---|
| `3272` ROLLDAP | Waitlist task (TVCEXECUTIVE `2CADR ROLLDAP`) |
| `3676` NOROLL1 | T6RUPT via T6LOC (NOROL1T6, set by JETROLL) |

Admitted state: FB bank 16 (`034000`), EB E6, BB `034006`, any raw A/L/Q,
erasable contents and channels. `CM_TVC_ROLL_OK` leaves `Z = TASKOVER
5324` (WAIT1/2, EXTENTST, SAMEJETS, RDAPEND) or `Z = NOQRSM 5335`
(NOROLL1); anything else is `CM_TVC_ROLL_OUT_OF_SCOPE`, image and io
unchanged. The file has no loop.

### How it is translated

As in `cm-tvc-exec`: one qualified `agc_image_ops` primitive per original
word, under C labels named for the original addresses, with every CCS
four-way skip and every TS overflow skip the data can reach. MSU, WRITE and
WOR are written from the yaAGC definitions. Three source idioms need care:

- `INDEX I` on a word with a zero opcode field (`0 SGNRT`, `0 A`, `0 DB`,
  ...): I is PRIO30 or BIT15, set at `3343`, so the word runs as CA or CS.
- `TS` overflow followed by `INDEX A; CS LIMITS` (`3506`, `3520`): A is ±1,
  so the CS reads LIMITS+1 or LIMITS-1 (BIT15 or POSMAX).
- `TS DELOGA` at `3335` overflowing skips the EXTEND at `3336`. The BZMF
  word then runs as `AD 3342`, and CAF PRIO30 follows. `TS TEMREG` at `3503`
  overflowing would likewise run `SU OGARATE` as `AD OGARATE`. That case
  is translated but no sample reaches it (A is a DV quotient, ±POSMAX or
  ∓MAXLIM).

## Source map

`evidence/cm-tvc-roll/cm-tvc-roll-source-map.json`
(`tools/cm_tvc_roll_source.py`): every word with its source line, rope
check and native label (`L3313`..`L3676` in `src/cm_tvc_roll.c`), the 47
fixed-constant reads matched to the native literals, and the NOROL1T6
2CADR, LIMITS±1 and the `3342` word matched to the rope.

Independent assembly comparison: `evidence/cm-tvc-roll/cm-tvc-roll-assembly-comparison.json`
(`tools/cm_flight_assembly_comparison.py`), every word yaYUL emits for the
unchanged file against the original Comanche055 rope, without reading the
native code.

## Evidence

Hashes: `evidence/cm-tvc-roll/SHA256.json`; bulk files in release
`evidence-cm-tvc-roll`.

- Oracle: `deps/cm-flight-fixture` (`tools/cm_flight_prepare_reference.py`,
  original CM rope `6e3150ae...`). The runner stops at TASKOVER or NOQRSM.
  Channel 13 starts with its trap bits (`04000`-`020000`, which yaAGC turns
  into hidden trap latches and drops from the channel) and its device-only
  bits (`010`, `01000`, `02000`) clear; the file only ORs in bit 15. The
  runner's scaler is held, so TIME6 does not count.
- Host and sanitizer results: `make test-cm-tvc-roll sanitize-cm-tvc-roll`.
  65536 cases (`tests/cm_tvc_roll_cases.c`: both entries; ROLLFIRE, TEMREG,
  OGA, OGANOW/OGAPAST and 1/CONACC zero, -0, small, medium or raw, so every
  phase-plane region, the DV limits and the reachable TS overflows occur;
  raw ROLLWORD, TIME6, A/L/Q, channels and unowned words). Every case reaches
  an exit, and the two sides agree on all of them. Native line coverage over
  the cases is complete. Stream identity `cm-tvc-roll-source-stream.json`,
  deltas `cm-tvc-roll-reference.deltas.xz`, log `cm-tvc-roll-checks.log`.
- C64 execution results: `make vice-cm-tvc-roll`, 128 frozen rows
  (`tools/cm_tvc_roll_freeze.py`). `cm-tvc-roll-results.json`.

## Open

ABI registration of the two entries follows in the Lane G routine-table
step, after Lane A's ISWRETRN fix.
