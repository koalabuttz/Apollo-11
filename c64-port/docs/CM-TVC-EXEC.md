# Translate Comanche TVCEXECUTIVE (TVCEXEC task, roll ladders, gain/mass and trim updates)

Lane G. Milestone `cm-tvc-exec`.

## Scope

`Comanche055/TVCEXECUTIVE.agc`, all of it: bank 16 `2720`-`3137`, 144
assembled words (including the two 2CADRs and FCORFRAC). Nothing is left in
the file.

`cm_tvc_exec(image, channel11)` runs from one of the file's original
continuation points to the next transfer out of the file:

| Entry | Reached from |
|---|---|
| `2720` TVCEXEC | Waitlist task (its own 2CADR, TVCINITIALIZE) |
| `2730` ROLLPREP, `2746` NEEDLEUP | WAITLIST returns past the 2CADRs |
| `2750` VARGAINS, `2773`, `2775` | IBNKCALL returns (NEEDLER, FIXCW, S40.15) |
| `3003`, `3011`, `3040`, `3043`, `3121`, `3132` | TVCRESTARTS EXRSTRT via TVCEXADR (EXECCOPY+1, 1SHOTCHK, TEMPSET, CORSETUP, CORCOPY+1, CNTRCOPY) |

Admitted state: FB bank 16 (`034000`), EB E6, BB `034006`, any raw A/L/Q and
erasable contents, and channel 11 (DSALMOUT, engine-on bit 13) from the
caller's device record. Results:

- `CM_TVC_EXEC_OK` at the frontier the original reaches: `Z = WAITLIST 5245`
  with A the delay and Q `2726`/`2744` (the 2CADR), `Z = IBNKCALL 4740` with Q
  `2747`/`2772`/`2774` (the CADR), or `Z = TASKOVER 5324`. Those three are
  Executive/Waitlist and INTER-BANK entries (Lane A); the inline words are in
  the compiled site tables.
- `CM_TVC_EXEC_NO_EXIT`: CCS VCNTR found VCNTR negative (the "NOT USED"
  branch) and the source spins on `TCF +0` at `2763`.
- `CM_TVC_EXEC_OUT_OF_SCOPE`: image unchanged.
- `CM_TVC_EXEC_FAULT`: cannot occur (a TS overflow skip at `3135` into the
  FCORFRAC data word; A there is +0 or ±1).

### How it is translated

As in `cm-tvc-massprop`: one qualified `agc_image_ops` primitive per original
word, under a C label named for its address, with every TS overflow skip and
CCS four-way skip. A skip over EXTEND runs the next word as the basic
instruction its bits name (`MP -BIT14` as `MASK -BIT14`, `DCA PDELOFF` as
`CA PDELOFF+1`). Two extracodes have no primitive and are written here from
the yaAGC definitions: MSU (two's-complement difference of the 15-bit operands,
sign-extended, minus one when negative) and RAND on channel 11
(overflow-corrected A AND the channel, sign-extended).

Reachable overflow skips: `TS MASSTMP` at `2777` (CS TENMDOT + CSMMASS) and
`TS TEMPDAP` at `3051` (2·TEMPDAP+1). The others are unreachable because A is a
constant, a CCS result, a load, or an MSU or MP result. CCS negative/−0 on the
RAND result is unreachable. They are translated anyway.

## Source map

`evidence/cm-tvc-exec/cm-tvc-exec-source-map.json` (`tools/cm_tvc_exec_source.py`):
every word with its source line, rope check and native label (`L2720`..`L3135`
in `src/cm_tvc_exec.c`), plus the 11 fixed constants (OCT60000, .5SEC,
-BIT14, THREE, BIT13, NINETEEN, SIX, FOUR, FCORFRAC, NEGONE, ZERO) read from the
rope and matched to the native literals.

## Evidence

Hashes: `evidence/cm-tvc-exec/SHA256.json`; bulk files in release
`evidence-cm-tvc-exec`.

- Oracle: `deps/cm-flight-fixture` (`tools/cm_flight_prepare_reference.py`,
  original CM rope `6e3150ae...`). The runner stops at the first of
  WAITLIST/IBNKCALL/TASKOVER, or reports no exit after 4000 steps.
- Host and sanitizer results: `make test-cm-tvc-exec sanitize-cm-tvc-exec`.
  65536 cases (`tests/cm_tvc_exec_cases.c`: every entry; FLAGWRD6 TVC/RCS
  bits; engine on/off; each CCS class of VCNTR and CNTR; REPFRAC sign; LEM
  on/off; random trims, masses, A/L/Q, IRQ, unowned words); 64676 reach a
  frontier and 860 spin, and the two sides agree on all of them. Stream identity
  `cm-tvc-exec-source-stream.json`, deltas `cm-tvc-exec-reference.deltas.xz`,
  log `cm-tvc-exec-checks.log`.
- C64 execution results: `make vice-cm-tvc-exec`, 128 frozen rows
  (`tools/cm_tvc_exec_freeze.py`). `cm-tvc-exec-results.json`.

## Open

The ABI registration of these entries (and of MASSPROP/FIXCW) with a
trampoline round trip VARGAINS → IBNKCALL → FIXCW → ISWRETRN → `2773` →
IBNKCALL → S40.15 is a separate Lane G step. It agrees with yaAGC on 8192
cases locally, once Lane A's ISWRETRN routine accepts the genuine `Q = 3`
return state (reported to Lane A).
