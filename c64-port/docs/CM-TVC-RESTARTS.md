# Translate Comanche TVCRESTARTS (TVC restart package, outcounter enables, EXRSTRT)

Lane G. Milestone `cm-tvc-restarts`.

## Scope

`Comanche055/TVCRESTARTS.agc`, all of it: bank 16 `3140`-`3271`, 90
assembled words (including the EXRSTRT 2CADR and the TVCCADR/TVCEXADR
table). Nothing is left in the file.

`cm_tvc_restarts(image, io)` runs from one of the file's original entry
points to the next transfer out of the file. `io` holds output channels 12
and 14, which the file ORs into with WOR:

| Entry | Reached from |
|---|---|
| `3140` REDOTVC | T5RUPT via T5LOC (FRESH START AND RESTART, `2CADR REDOTVC`) |
| `3174` ENABL2, `3205` CMDSOUT | T5RUPT via T5LOC (this file's TVCCADR GENADRs) |
| `3232` CHKSTRK | IBNKCALL return from `SWICHOVR +5` (TVCINITIALIZE) |
| `3243` FINCOPY +3 | ISWCALL return from the pitch or yaw copycycle (TVCDAPS) |
| `3253` EXRSTRT | Waitlist task (REDOTVC's 2CADR) |

Admitted state: FB bank 16 (`034000`), EB E6, BB `034006`, any raw A/L/Q,
erasable contents and channels 12/14, with two index limits the source
relies on: EXRSTRT needs TVCEXPHS 1..6 (its INDEX into TVCEXADR, then
`INDEX A; TCF 0`), and REDOTVC with TVCEXPHS +0 and an odd positive
TVCPHASE needs TVCPHASE <= 9 (FINCOPY's INDEX into TVCCADR, kept inside the
table). Results:

- `CM_TVC_RESTARTS_OK` at the frontier the original reaches: `WAITLIST
  5245` with Q `3150` (2CADR EXRSTRT) and A the 9 cs delay; `RESUME 5333`
  (ENABL1) or `NOQRSM 5335` (ENABL2) with T5LOC/TIME5 set; `POSTJUMP 4701`
  with Q `3227` (MRCLEAN) or `3237` (TVCINIT4); `IBNKCALL 4740` with Q
  `3231` (SWICHOVR +5); `ISWCALL 4744` with Q `3243` and A the copycycle
  CADR; or one of TVCEXECUTIVE's restart points `3003`-`3132` (cm-tvc-exec
  entries). Q is left as a canonical 15-bit word.
- `CM_TVC_RESTARTS_OUT_OF_SCOPE`: image and channels unchanged.

There is no loop in the file, so there is no non-returning case.

### How it is translated

As in `cm-tvc-exec`: one qualified `agc_image_ops` primitive per original
word, under a C label named for its address, with every CCS four-way skip.
WOR has no primitive and is written here from the yaAGC definition
(overflow-corrected A OR the channel, to both A and the channel). The TS
words store a CAF constant or `-0` plus a 15-bit word, which cannot
overflow, so they take no skip. Unreachable CCS slots: `-0` at `3154` (A
is TVCPHASE masked with `040001`) and negative at `3224` (A is a
diminished absolute value).

## Source map

`evidence/cm-tvc-restarts/cm-tvc-restarts-source-map.json`
(`tools/cm_tvc_restarts_source.py`): every word with its source line, rope
check and native label (`L3140`..`L3253` in `src/cm_tvc_restarts.c`), the
14 fixed-constant reads (NINE, OCT37776, BIT2, BIT8, BIT11, PRIO6, ZERO,
TVCADDR, TVCADDR +2, TVCADDR +4) matched to the native literals, and the
native `table[]` matched to rope words `3257`-`3271`.

## Evidence

Hashes: `evidence/cm-tvc-restarts/SHA256.json`; bulk files in release
`evidence-cm-tvc-restarts`.

- Oracle: `deps/cm-flight-fixture` (`tools/cm_flight_prepare_reference.py`,
  original CM rope `6e3150ae...`). The runner stops at the first frontier
  above. yaAGC's optics CDU model empties OPTY/OPTX (`035`/`036`) whenever
  channel 14 bits 11/12 are set, and its gyro and IMU CDU drive models act
  on channel 14 bits `01000`/`070000`; those are the simulator standing in
  for hardware, not the program. The cases start with the drive bits clear
  (the file only sets bits 11 and 12) and the runner keeps `035`/`036` as
  the program left them (the file never addresses them).
- Host and sanitizer results: `make test-cm-tvc-restarts
  sanitize-cm-tvc-restarts`. 65536 cases (`tests/cm_tvc_restarts_cases.c`:
  every entry; each TVCPHASE restart value -2..3 and -0, and raw ones;
  TVCEXPHS 1..6 for EXRSTRT and each CCS class elsewhere; STROKER classes;
  raw PCMD/YCMD, trims, A/L/Q, channels and unowned words); every case
  reaches a frontier, and the two sides agree on all of them. Native line
  coverage over the cases is complete apart from the entry refusals, which
  the host test exercises. Stream identity
  `cm-tvc-restarts-source-stream.json`, deltas
  `cm-tvc-restarts-reference.deltas.xz`, log `cm-tvc-restarts-checks.log`.
- C64 execution results: `make vice-cm-tvc-restarts`, 128 frozen rows
  (`tools/cm_tvc_restarts_freeze.py`). `cm-tvc-restarts-results.json`.

## Open

ABI registration of the six entries (with cm-tvc-exec's) follows in the
Lane G routine-table step, after Lane A's ISWRETRN fix.
