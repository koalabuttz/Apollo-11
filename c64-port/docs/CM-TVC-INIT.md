# Translate Comanche TVCINITIALIZE (TVC DAP initialization, CSM/LM switch-over, LOADCOEF, S40.15)

Lane G. Milestone `cm-tvc-init`.

## Scope

`Comanche055/TVCINITIALIZE.agc`, all of it: bank 17 `2147`-`2551`, 259
assembled words (including the CSMN10/LBN10 coefficient tables, the
GENADRs, constants and the DAPINIT5/INITLOC2 2CADRs). Nothing is left in the
file.

`cm_tvc_init(image, io)` runs from one of the file's original entry points
to the next transfer out of the file. `io` holds channel 11 (DSALMOUT, read
by `RAND` in TVCINIT4) and the interrupt enable (INHINT in SWICHOVR):

| Entry | Reached from |
|---|---|
| `2147` TVCDAPON | T5RUPT via T5LOC (DOTVCON, P40-P47) |
| `2152` MRCLEAN | POSTJUMP from TVCRESTARTS PHSCHK2 |
| `2171` TVCINIT1 | T5RUPT via INITLOC2 |
| `2176` | IBNKCALL return from MASSPROP |
| `2324` TVCINIT4 | IBNKCALL return from NEEDLER; POSTJUMP from TVCRESTARTS CHKSTRK |
| `2343` | WAITLIST return past `2CADR TVCEXEC` |
| `2351` PRESWTCH | V46 (EXTENDED VERBS `CADR PRESWTCH`) |
| `2361` SWICHOVR +5 | IBNKCALL from TVCRESTARTS PHSCHK2 |
| `2462` S40.15 | IBNKCALL from TVCEXECUTIVE |

TVCINIT2/3, ATTINIT, NEEDLEIN, SWICHOVR, LOADCOEF and S40.15 +7 are reached
only from inside the file, by fall-through or by `TC`/`TCR` with the file's
own Q; their `TC Q` returns run the TC held in Q, leaving Q = 3.

Admitted state: FB bank 17 (`036000`), EB E6, BB `036006`, any raw A/L/Q,
erasable contents, channel 11 and interrupt enable. S40.15, left by `TC Q`
to its caller, needs Q `4000`-`7777` (a fixed-fixed return, as IBNKCALL
provides). Results:

- `CM_TVC_INIT_OK` at the frontier the original reaches: `RESUME 5333`
  (ENDMRC with T5LOC = INITLOC2, ENDTVCIN with T5LOC = DAPINIT5);
  `IBNKCALL 4740` with Q `2175` (MASSPROP) or `2323` (NEEDLER); `WAITLIST
  5245` with Q `2341` (2CADR TVCEXEC) and A = .5SEC; `Z = RTRNLOC 1613`
  from `TC RTRNLOC` with Q `2422` (RTRNLOC holds the return QXCH saved);
  the caller's return from S40.15; or `Z = 1703` with Q `2503`.
- `CM_TVC_INIT_OUT_OF_SCOPE`: image and io unchanged.

The last frontier is the source's own overflow path: when `TS VARK`
overflows (KTLX/I times IAVG/TLX doubled twice), the skip passes over
`TC Q` into the CSMN10 table, whose first words run as `CA 7777`, `AD
7657`, `TC 1703`. The native code does the same with the rope's
fixed-fixed words.

The MRCLEAN and MCLEANJR loops count down from NZERO and NZEROJR (odd) by
two and always end, so there is no non-returning case.

### How it is translated

As in `cm-tvc-restarts`: one qualified `agc_image_ops` primitive per
original word, under a C label named for its address, with every CCS
four-way skip and the reachable TS overflow skips (`TS MASSTMP` at `2204`,
`TS 1/CONACC` at `2470`, `TS VARK` at `2476`). The other TS words store
constants, loads, CCS or MP results. RAND on channel 11 is written from the
yaAGC definition, as in `cm-tvc-exec`. INDEX on the file's own counters
(CNTR 0/1, TTMP1, CNTRTMP, COEFFADR) is resolved against the source's
tables: CSMCFADR/HBCFADR, TCORR, BIT2/BIT3 (`INDEX A` on -CNTR), and
LOADCOEF's `INDEX COEFFADR; DCA k` over CSMN10 or LBN10 in fixed memory or
HBN10 in erasable memory.

Unreachable but translated: the CCS negative and -0 slots on the MRCLEAN,
ATTINIT and MCLEANJR counters, and the CCS +0 slot at `2371`.

## Source map

`evidence/cm-tvc-init/cm-tvc-init-source-map.json`
(`tools/cm_tvc_init_source.py`): every word with its source line, rope check
and native label (`L2147`..`L2472` in `src/cm_tvc_init.c`), the 28
fixed-constant reads (CAF, CA, CS, AD, MASK, MP on fixed memory) matched to
the native literals, and the native coefficient table, 2CADRs, INDEX tables
and fixed-fixed `7777`/`7657` matched to the rope.

## Evidence

Hashes: `evidence/cm-tvc-init/SHA256.json`; bulk files in release
`evidence-cm-tvc-init`.

- Oracle: `deps/cm-flight-fixture` (`tools/cm_flight_prepare_reference.py`,
  original CM rope `6e3150ae...`). The runner stops at the first frontier
  above; for the S40.15 entry, also at the caller's return.
- Host and sanitizer results: `make test-cm-tvc-init sanitize-cm-tvc-init`.
  65536 cases (`tests/cm_tvc_init_cases.c`: every entry; DAPDATR1 bits 13
  and 14; engine on/off; ERRBTMP, IXX, KTLX/I and IAVG/TLX small, zero,
  negative or raw, so the attitude limits and the three overflow skips are
  all taken; raw EMDOT, CSMMASS, trims, A/L/Q, channel 11, interrupt enable
  and unowned words). Every case reaches a frontier, and the two sides agree
  on all of them. Every native branch is taken over the cases except the
  entry refusals, which the host test exercises, and the unreachable slots
  above. Stream identity `cm-tvc-init-source-stream.json`, deltas
  `cm-tvc-init-reference.deltas.xz`, log `cm-tvc-init-checks.log`.
- C64 execution results: `make vice-cm-tvc-init`, 128 frozen rows
  (`tools/cm_tvc_init_freeze.py`). `cm-tvc-init-results.json`.

## Open

ABI registration of the nine entries follows in the Lane G routine-table
step, after Lane A's ISWRETRN fix.
