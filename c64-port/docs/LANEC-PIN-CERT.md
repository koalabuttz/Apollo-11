# PINBALL_GAME_BUTTONS_AND_LIGHTS full-file certificates

Lane C. Milestone `lanec-pin-cert`.

This drafts the `pending_complete_certificate` of
`Luminary099/PINBALL_GAME_BUTTONS_AND_LIGHTS.agc` and of
`Comanche055/PINBALL_GAME_BUTTONS_AND_LIGHTS.agc` in
`docs/port-coverage.json` for the reviewers. The files stay `partial` until
a reviewer flips them.

## Transfer ABI

Every labelled entry, every return into the file from a callee outside it
and every BANKCALL return point is a routine of the native control-transfer
ABI (`docs/NATIVE_TRANSFER_ABI.md`): 267 Luminary099 routines in
`lanec_pin_lm_routines` and 262 Comanche055 routines in
`lanec_pin_cm_routines` (`src/lanec_pin_routines.inc`), generated and
checked against the listings by `tools/lanec/routines.py pin`. The
internal subroutines it lists are reached only by a TC inside the file and
are compiled into their callers. Each routine runs the earlier Pinball
modules that claim its point. See `docs/LANEC-PIN-TRANSFER.md` (milestone
`lanec-pin-transfer`): 2,048 round trips from the genuine external callers
and from every routine's own entry give 1,212 complete states identical to
the original and 836 chains refused unchanged; `pin-refuse` refuses all
1,530 entry states with a broken register contract unchanged; and on the C64
one program per module and mission runs the round trips in VICE.

## What it claims

Every assembled word of each file has exactly one owner:

| Words (LM / CM) | Owner | Native code |
|---|---|---|
| 79 / 80 | `lanec-pin-gaps`: DSP2DEC/END2DEC, NVSUBB, LOADLV1, DSPDCWD1 and the other words no earlier module carried | `src/lanec_pin_gaps.c` |
| 2,089 / 2,086 | `lanec-pw`: the earlier Pinball modules, each run by its `pw-*` suite | `src/dsky_*.c`, `src/cm_*.c` through `src/lanec_pw*_runs.c` |
| 210 / 213 | constants | compiled `symbols.txt` WORD rows or literals |

Both module counts include each call's inline arguments (65 words per
mission: CADR, ADRES and the like after a TC), which the native call
compiles in. The constant words are 199 / 202 data words and 11 instruction
words that other code reads as data and that never execute: INRELTAB + 15,
MIXAD, the three DISPLACE words, NVSBENDL, ENDINST, TCNOVAC, TCWAIT,
TCTSKOVR and TCFINDVC (`tools/lanec/wholefile_pin.py` gives each one's
readers).

Two Luminary099 and three Comanche055 instruction words are unreachable:

- LEGALTST `TC CCSHOLE`: `AD NEG2`, `CCS A` reaches it only for a sum of
  +0, which the one's-complement adder never gives when one operand (NEG2)
  is nonzero.
- Comanche055 NEGOPT `TC CCSHOLE`: the same, after `AD 20BIAS`.
- ALL3DEC's first `TC FORCEV25`: `CS OCT34BAR`, `MASK DECBRNCH` leaves at
  most 34 octal, and `AD OCT34BAR` (-34) gives a negative sum or -0, so
  `CCS A` never takes the positive branch to it.

## How it is checked

`tools/lanec/wholefile_pin.py` (`make test-lanec-pin-cert`) rebuilds both
maps from the pinned listings and the unchanged original files. It runs
`pin-gaps` and the 92 `pw-*` suites through the oracle with `LANEC_TRACE`
and fails unless:

- every word has exactly one owner;
- every instruction word but the constants and the unreachable ones was
  started by the unchanged original program in Virtual AGC under one of the
  suites' cases (2,101 LM and 2,098 CM words), and the native modules
  reproduce those cases' complete results;
- each data word after a call is that call's argument, and every other data
  word is a constant;
- the constant and unreachable instruction words match the listing.

The earlier modules overlap, so a word executed under several suites is
given to the first that runs it (`pin-gaps`, then the `pw-*` suites in name
order); every suite that runs it reproduces the original there.

## Evidence

Hashes: `evidence/lanec-pin-cert/SHA256.json`.

- Source-to-native maps: `lanec-pin-lm-source-map.json` and
  `lanec-pin-cm-source-map.json`. They give every word's original line,
  address, assembled word, owner, native module, transfer-ABI routine and
  how the oracle covered it.
- Independent assembly comparison: `lanec-pin-comparison.json`. @COMPARISON@
- Host and sanitizer results: `lanec-pin-checks.log`. It has the strict and
  ASan/UBSan runs of every suite and the whole-file map run.
- C64 execution results: `lanec-pin-c64-results.json`. @C64@
- C64 branch outcomes: `lanec-pin-c64-cover.json` (`tools/lanec/cover.py
  record`): for each suite, the host cases' branch arcs over every linked
  source and the frozen C64 rows, which take every one of them.

The supported domain and the external continuations are written into the
certificates themselves.

Regenerate the evidence with:

    python3 tools/lanec/cert_evidence.py lanec-pin evidence/lanec-pin-cert \
      pin-gaps $(sed -n 's/.*"\(pw-[a-z0-9-]*\)".*/\1/p' tests/lanec_pw_*_cases.c | sort -u) \
      pin-transfer pin-transfer-c64 --refusals <the same suites> --refusals-only pin-refuse \
      --wholefile tools/lanec/wholefile_pin.py \
      --c64 pin-gaps=lanec-pin-gaps pw=lanec-pw pin-transfer-c64=lanec-pin-transfer
    python3 tools/lanec/cover.py record evidence/lanec-pin-cert/lanec-pin-c64-cover.json <the same suites but pin-transfer> \
      --pool --not-run pin-transfer-c64=<its rows no program runs> --legacy pw-fixed-display=vice-legacy-dsky-fixed-display ...
