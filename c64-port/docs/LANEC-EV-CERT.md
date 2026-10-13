# EXTENDED_VERBS full-file certificate (Luminary099)

Lane C. Milestone `lanec-ev-cert`.

This drafts the `pending_complete_certificate` of
`Luminary099/EXTENDED_VERBS.agc` in `docs/port-coverage.json` for the
auditor's review. The file stays `partial` until the auditor flips it.
Comanche055's EXTENDED_VERBS follows once its interpretive words are
translated.

## Transfer ABI

Every labelled entry and every return into the file from a callee is a
routine of the native control-transfer ABI (`docs/NATIVE_TRANSFER_ABI.md`):
313 basic routines in `lanec_ev_lm_routines` and the 29 interpretive entries,
keyed by LOC, in `lanec_ev_lm_interpretive`, generated and checked against
the listing by `tools/lanec/routines.py ev`. 468 round trips through
`agc_transfer_run`, from Pinball's `POSTJUMP GOEXTVB` and every other
BANKCALL/POSTJUMP caller of the file, match the original program, 64 of
them on a C64, and 512 broken entry states are refused unchanged. See
`docs/LANEC-EV-TRANSFER.md` (milestone `lanec-ev-transfer`). The source map
names, for every word, the C function that carries it and, for an entry,
the routine it is registered as.

## What it claims

Every one of the file's 994 assembled words has exactly one owner:

| Words | Owner | Native module |
|---|---|---|
| 86 | `lanec-ev-front`: the front end (verb fan, TESTXACT, XACT0/XACTALM) | `src/lanec_ev_front.c` |
| 182 | `lanec-ev-short`: the short verb bodies | `src/lanec_ev_short.c` |
| 222 | `lanec-ev-align`: the alignment and radar-designate bodies | `src/lanec_ev_align.c` |
| 150 | `lanec-ev-radar`: the radar test verbs (R04, R77), with the unreachable word | `src/lanec_ev_radar.c` |
| 156 | `lanec-ev-dap`: V48 DAP data load (R03) | `src/lanec_ev_dap.c` |
| 39 | `lanec-ev-time`: V55 clock update and ATTCK2 | `src/lanec_ev_time.c` |
| 60 | `lanec-ev-track`: basic code of V69 and the tracking and state-vector verbs | `src/lanec_ev_track.c` |
| 55 | `lanec-ev-lmi`: the interpretive words (RRLOSDSP, TRMTRAK1, ATTACHIT, TCPIN) | `src/lanec_ev_lmi.c` |
| 44 | constants | compiled `symbols.txt` WORD rows or literals |

The module counts include each call's inline arguments (116 words in all:
CADR, 2CADR, ADRES and the like after a TC), which the native call compiles
in. Of the 44 constant words, 43 are read by native code: OC24100 as a
literal in `src/lanec_ev_front.c`, the rest as `symbols.txt` WORD rows. The
other one, -LOKONFG, is named by no source line of the program and stays in
the rope as data.

One instruction word is unreachable. At 43,2661, `TC ALM/END` follows
`CS FLGWRD11`, `MASK LRBYBIT`, `CCS A` in RDRUSECK. LRBYBIT is BIT15, so A
is +0 or negative and CCS never takes the positive branch to it. The radar
module has no such path to translate.

## How it is checked

`tools/lanec/wholefile_ev.py` (`make test-lanec-ev-cert`) rebuilds the map
from the pinned listing and the unchanged original file. It runs the eight
extended-verb suites through the oracle with `LANEC_TRACE` and fails unless:

- every word has exactly one owner;
- every instruction word was started by the unchanged original program in
  Virtual AGC under one of the suites' cases (778 words), or, for the
  interpretive words, was read by the original interpreter through
  `INDEX LOC` under `ev-lmi` (55 words). The native modules reproduce those
  cases' complete results;
- each data word after a call is that call's argument, and every other data
  word is a constant;
- the unreachable word and the three words before it match the listing.

The oracle's trace now also records each word the interpreter reads: an
`INDEX LOC` followed by an instruction with address K reads LOC + K. These
lines start with `LMI` or `CMI`, so they are separate from the executed
words the display certificate reads.

## Evidence

Hashes: `evidence/lanec-ev-cert/SHA256.json`.

- Source-to-native map: `lanec-ev-lm-source-map.json`. It gives every
  word's original line, address, assembled word, owner, native module and
  how the oracle covered it.
- Independent assembly comparison: `lanec-ev-comparison.json`. It holds
  the eight module suites' `diff.py` summaries (27,776 cases), the
  `ev-transfer` and `ev-transfer-c64` round trips (532 cases) and the
  `ev-refuse` refusals (512 cases refused with the state unchanged): 28,820
  compared with zero differing, and the hashes of their per-case oracle CRCs.
- Host and sanitizer results: `lanec-ev-checks.log`. It has the strict and
  ASan/UBSan runs of all eleven suites and the whole-file map run.
- C64 execution results: `lanec-ev-c64-results.json`. It has fresh VICE
  PAL runs of the eight frozen module subsets (each picked by
  `tools/lanec/cover.py` to take every host branch arc) and the transfer round trip
  (457 checks, 0 failures) with the PRG hashes.

The supported domain and the external continuations are written into the
certificate itself.

Regenerate the evidence with:

    python3 tools/lanec/cert_evidence.py lanec-ev build/ev-cert-new ev-front ev-short ev-align \
      ev-radar ev-dap ev-time ev-track ev-lmi ev-transfer ev-transfer-c64 \
      --refusals-only ev-refuse --wholefile tools/lanec/wholefile_ev.py \
      --c64 ev-front=lanec-ev-front ev-short=lanec-ev-short ev-align=lanec-ev-align \
      ev-radar=lanec-ev-radar ev-dap=lanec-ev-dap ev-time=lanec-ev-time \
      ev-track=lanec-ev-track ev-lmi=lanec-ev-lmi ev-transfer-c64=lanec-ev-transfer
