# Translate Comanche TVCSTROKETEST (V68 stroke test initialization and HACK waveform)

Lane G. Milestone `cm-tvc-stroke`.

## Scope

`Comanche055/TVCSTROKETEST.agc`, all of it: bank 20 `3501`-`3611`, 73
assembled words (including the HACKWLST 2CADR and the FCADDY..FCARD6
constants). Nothing is left in the file.

`cm_tvc_stroke(image, io)` runs from one of the file's original entry
points to the next transfer out of the file. `io` holds output channel 14
(WOR CHAN14) and the interrupt enable (INHINT in STRKCHK):

| Entry | Reached from |
|---|---|
| `3501` STRKTSTI | Job set up by V68 (EXTENDED VERBS `2CADR STRKTSTI`) |
| `3533` HACK | `TC HACK` from the pitch and yaw TVC DAPs (TVCDAPS) |
| `3541` | WAITLIST return past HACK's 2CADR (`TCF +3`) |
| `3542` HACKWLST | Waitlist task (HACK's 2CADR) |

TSTINIT (`3512`) is reached only by `TCR` from STRKTSTI; its `TC Q` runs
the TC held in Q (STRKCHK), leaving Q = 3.

Admitted state: FB bank 20 (`040000`), EB E6, BB `040006`, any raw A/L/Q,
erasable contents, channel 14 and interrupt enable. HACK, its WAITLIST
return and HACKWLST also need CARD -4..-1: only TSTINIT (which sets -4)
and the waveform (which steps it up and stops at -0) write CARD, and CARD
indexes the FCARD table. Results:

- `CM_TVC_STROKE_OK` at the frontier the original reaches: `ENDOFJOB 5217`
  (STRKCHK, interrupts inhibited); `WAITLIST 5245` with Q `3537` (2CADR
  HACKWLST) and A = 20MS; or `Z = BUNKER 1657` from `TC BUNKER` with Q
  `3556`, `3575`, `3577` or `3612`. BUNKER holds the instruction to run
  next: the DAP's return saved by `QXCH BUNKER` (entry from HACK) or
  `TC TASKOVER` (TCTSKOVR, entry from HACKWLST).
- `CM_TVC_STROKE_OUT_OF_SCOPE`: image and io unchanged.

There is no loop in the file, so there is no non-returning case.

### How it is translated

As in `cm-tvc-restarts`: one qualified `agc_image_ops` primitive per
original word, under a C label named for its address, with every CCS
four-way skip and the reachable TS overflow skip (`TS CADDY` at `3606`
after `DOUBLE`). WOR is written from the yaAGC definition. The other TS
words store constants, CCS results, a negated constant or a loaded word,
which cannot overflow.

## Source map

`evidence/cm-tvc-stroke/cm-tvc-stroke-source-map.json`
(`tools/cm_tvc_stroke_source.py`): every word with its source line, rope
check and native label (`L3501`..`L3610` in `src/cm_tvc_stroke.c`), the 8
fixed-constant reads (BIT14, FCADDY, FREVS, FCARD, 20MS, TCTSKOVR, BIT11,
ONE) matched to the native literals, and the native `fcard[]` matched to
rope words `3524`-`3532`.

Independent assembly comparison: `evidence/cm-tvc-stroke/cm-tvc-stroke-assembly-comparison.json`
(`tools/cm_flight_assembly_comparison.py`), every word yaYUL emits for the
unchanged file against the original Comanche055 rope, without reading the
native code.

## Evidence

Hashes: `evidence/cm-tvc-stroke/SHA256.json`; bulk files in release
`evidence-cm-tvc-stroke`.

- Oracle: `deps/cm-flight-fixture` (`tools/cm_flight_prepare_reference.py`,
  original CM rope `6e3150ae...`). The runner stops at the first frontier
  above. As in `cm-tvc-restarts`, yaAGC's optics CDU model empties
  OPTY/OPTX (`035`/`036`) when channel 14 bits 11/12 are set and its gyro
  and IMU CDU drive models act on bits `01000`/`070000`; the cases start
  with the drive bits clear and the runner keeps `035`/`036` as the program
  left them (the file never addresses them).
- Host and sanitizer results: `make test-cm-tvc-stroke
  sanitize-cm-tvc-stroke`. 65536 cases (`tests/cm_tvc_stroke_cases.c`:
  every entry; DAPDATR1 bit 14 set and clear; CARD -4..-1; each CCS class
  of REVS; CADDY classes; N small and raw, so DOUBLE overflows; raw
  STROKER, ESTROKER, TVCPITCH, A/L/Q, channel 14, interrupt enable and
  unowned words); every case reaches a frontier, and the two sides agree on
  all of them. Every native branch is taken over the cases apart from the
  entry refusals, which the host test exercises. Stream identity
  `cm-tvc-stroke-source-stream.json`, deltas
  `cm-tvc-stroke-reference.deltas.xz`, log `cm-tvc-stroke-checks.log`.
- C64 execution results: `make vice-cm-tvc-stroke`, 128 frozen rows
  (`tools/cm_tvc_stroke_freeze.py`). `cm-tvc-stroke-results.json`.

## Open

ABI registration of the four entries follows in the Lane G routine-table
step, after Lane A's ISWRETRN fix.
