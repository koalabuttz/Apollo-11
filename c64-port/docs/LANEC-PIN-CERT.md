# PINBALL_GAME_BUTTONS_AND_LIGHTS full-file certificates

Lane C. Milestone `lanec-pin-cert`.

This drafts the `pending_complete_certificate` of
`Luminary099/PINBALL_GAME_BUTTONS_AND_LIGHTS.agc` and of
`Comanche055/PINBALL_GAME_BUTTONS_AND_LIGHTS.agc` in
`docs/port-coverage.json` for the reviewers. The files stay `partial` until
a reviewer flips them.

## Transfer ABI

Every labelled entry, every return into the file from a callee outside it,
every BANKCALL return point and every word the file reaches through
POSTJUMP or BANKJUMP (GOVNUPDT, UPDAT1 +2, DSPOCTIN, OCTBACK; registered by
`lanec-pin-gaps`) is a routine of the native control-transfer
ABI (`docs/NATIVE_TRANSFER_ABI.md`): 280 Luminary099 routines in
`lanec_pin_lm_routines` and 279 Comanche055 routines in
`lanec_pin_cm_routines` (`src/lanec_pin_routines.inc`), and 13 per mission
at CHARIN and its key labels in the keypress tables
(`docs/LANEC-PIN-KEYS.md`), generated and
checked against the listings by `tools/lanec/routines.py pin`. The
internal subroutines it lists are reached only by a TC inside the file and
are compiled into their callers. Each routine runs the earlier Pinball
modules that claim its point, with the head families
(`docs/LANEC-PIN-HEADS.md`) in place of the composite modules at the points
they claim. See `docs/LANEC-PIN-TRANSFER.md` (milestone
`lanec-pin-transfer`): 2,276 round trips from the genuine external callers
and from every routine's own entry give 1,336 complete states identical to
the original and 940 chains refused unchanged; `pin-refuse` refuses all
1,530 entry states with a broken register contract unchanged; and on the C64
one program per module and mission runs the round trips in VICE, and the
Pinball flight modules run the rest from the REU store.

## What it claims

Every assembled word of each file has exactly one owner:

| Words (LM / CM) | Owner | Native code |
|---|---|---|
| 107 / 108 | `lanec-pin-gaps`: DSP2DEC/END2DEC, NVSUBB, LOADLV1, DSPDCWD1, GOVNUPDT, DSPOCTIN, the octal display's WDAGAIN/OCTBACK loop and the other words no earlier module carried | `src/lanec_pin_gaps.c` |
| 110 / 110 | `lanec-pin-heads`: the head families, each run by its `pin-head-*` suite | `src/lanec_pin_head_*.c` |
| 49 / 45 | `lanec-pin-keys`: CHARIN and the key labels | `src/lanec_pin_keys*.c` |
| 1,902 / 1,903 | `lanec-pw`: the earlier Pinball modules, each run by its `pw-*` suite | `src/dsky_*.c`, `src/cm_*.c` through `src/lanec_pw*_runs.c` |
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
`pin-gaps`, the ten `pin-head-*` suites, the keypress modules and the
`pw-*` suites whose module still runs at some Pinball point (76; the
composites the heads replace or `dropped` removes stay host tests) through
the oracle with `LANEC_TRACE` and fails unless:

- every word has exactly one owner;
- every instruction word but the constants and the unreachable ones was
  started by the unchanged original program in Virtual AGC under one of the
  suites' cases (2,101 LM and 2,098 CM words), and the native modules
  reproduce those cases' complete results;
- each data word after a call is that call's argument, and every other data
  word is a constant;
- the constant and unreachable instruction words match the listing.

The earlier modules overlap, so a word executed under several suites is
given to the first that runs it (`pin-gaps`, then the heads, the keypress
modules and the `pw-*` suites in name order); every suite that runs it
reproduces the original there.

## Evidence

Hashes: `evidence/lanec-pin-cert/SHA256.json`.

- Source-to-native maps: `lanec-pin-lm-source-map.json` and
  `lanec-pin-cm-source-map.json`. They give every word's original line,
  address, assembled word, owner, native module, transfer-ABI routine and
  how the oracle covered it.
- Independent assembly comparison: `lanec-pin-comparison.json`: 91
  suites (`pin-gaps`, the ten `pin-head-*` suites, the 76 `pw-*` suites,
  `pin-transfer`, `pin-transfer-c64`, `pin-decdsp3` and `pin-refuse`),
  149,340 cases, zero differing and 4,608 refused with the image and I/O
  unchanged.
- Host and sanitizer results: `lanec-pin-checks.log`. It has the strict and
  ASan/UBSan runs of every suite and the whole-file map run.
- C64 execution results: `lanec-pin-c64-results.json`, with every PRG's
  hash: `vice-lanec-pin-gaps` (50 checks), `vice-lanec-pw-1` to `-5` (86
  programs, 926 checks), `vice-lanec-pin-transfer-1` to `-4` (89 programs,
  254 checks) and
  `vice-lanec-pin-flight` (95 checks: the frozen rows of the ten
  `pin-head-*` suites and `pw-fixed-display`, both missions, run one
  routine per case through the Pinball flight modules loaded from the REU
  store, `docs/MODULE-TEST.md`), all on a VICE PAL C64 with no failures.
- C64 branch outcomes: `lanec-pin-c64-cover.json` (`tools/lanec/cover.py
  record --pool`). For each suite it lists the host cases' branch arcs over
  every linked source and the frozen C64 rows. Every host arc is taken by
  some C64 program's rows, with three groups listed apart:
  - the 119 `pin-transfer-c64` rows of the 37 programs that do not fit
    below $C000 do not count (`c64_rows_not_run`); the DI round trips
    (`di-transfer-c64`) are pooled in, since they run the same INTER-BANK
    routines on the C64;
  - seven `pw-*` suites whose module does not fit a program with the
    harness: their modules run on the C64 in their own milestones' programs
    (`legacy_c64_programs`, `host_branches_on_c64_only_in_legacy_programs`);
  - the four arcs of the INTER-BANK routines' SUPDACAL path
    (`host_branches_not_on_c64`): it reads the fixed-memory rope, and its
    only Pinball caller, DSPFMEM (verb 27), is in the fixed-display
    module, whose transfer program with the rope does not fit. That path is
    checked on the host only.

The supported domain and the external continuations are written into the
certificates themselves.

Regenerate the evidence with (the suites of `tools/lanec/wholefile_pin.py`
but the keypress pair, whose evidence is `module-test-lanec-pin-keys`):

    S=$(python3 -c "import sys; sys.path.insert(0, 'tools/lanec'); import wholefile_pin as w; \
      print(' '.join(s[0] for s in w.SUITES if not s[0].startswith('pin-key')))")
    python3 tools/lanec/cert_evidence.py lanec-pin evidence/lanec-pin-cert $S pin-transfer pin-transfer-c64 \
      pin-decdsp3 --refusals $S pin-transfer pin-transfer-c64 pin-decdsp3 --refusals-only pin-refuse \
      --wholefile tools/lanec/wholefile_pin.py \
      --c64 pin-gaps=lanec-pin-gaps $(for n in 1 2 3 4 5; do echo pw=lanec-pw-$n; done) \
      $(for n in 1 2 3 4; do echo pin-transfer-c64=lanec-pin-transfer-$n; done) pin-flight=lanec-pin-flight
    python3 tools/lanec/cover.py record evidence/lanec-pin-cert/lanec-pin-c64-cover.json $S pin-transfer-c64 \
      di-transfer-c64 --pool --not-run pin-transfer-c64=<the rows of LANEC_PIN_C64_TOO_BIG's programs> \
      --host-only 'src/lm_interbank_routines.c:9=<reason>' --host-only 'src/cm_interbank_routines.c:9=<reason>'

The C64 runs take about 70 minutes, `vice-lanec-pin-flight` builds the
flight store first (`tools/flight_store_build.py`), and the record traces
every host case of every suite (about two hours; it keeps each suite's arcs
in `build/lanec-cover` and resumes from them).
