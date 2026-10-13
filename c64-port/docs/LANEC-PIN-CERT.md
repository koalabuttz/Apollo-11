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
ABI (`docs/NATIVE_TRANSFER_ABI.md`): 273 Luminary099 routines in
`lanec_pin_lm_routines` and 268 Comanche055 routines in
`lanec_pin_cm_routines` (`src/lanec_pin_routines.inc`), generated and
checked against the listings by `tools/lanec/routines.py pin`. The
internal subroutines it lists are reached only by a TC inside the file and
are compiled into their callers. Each routine runs the earlier Pinball
modules that claim its point. See `docs/LANEC-PIN-TRANSFER.md` (milestone
`lanec-pin-transfer`): 2,096 round trips from the genuine external callers
and from every routine's own entry give 1,232 complete states identical to
the original and 864 chains refused unchanged; `pin-refuse` refuses all
1,530 entry states with a broken register contract unchanged; and on the C64
one program per module and mission runs the round trips in VICE.

## What it claims

Every assembled word of each file has exactly one owner:

| Words (LM / CM) | Owner | Native code |
|---|---|---|
| 107 / 108 | `lanec-pin-gaps`: DSP2DEC/END2DEC, NVSUBB, LOADLV1, DSPDCWD1, GOVNUPDT, DSPOCTIN, the octal display's WDAGAIN/OCTBACK loop and the other words no earlier module carried | `src/lanec_pin_gaps.c` |
| 2,061 / 2,058 | `lanec-pw`: the earlier Pinball modules, each run by its `pw-*` suite | `src/dsky_*.c`, `src/cm_*.c` through `src/lanec_pw*_runs.c` |
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
- Independent assembly comparison: `lanec-pin-comparison.json`: 96
  suites (`pin-gaps`, the 92 `pw-*` suites, `pin-transfer`,
  `pin-transfer-c64` and `pin-refuse`), 128,625 cases, zero differing and
  3,878 refused with the image and I/O unchanged.
- Host and sanitizer results: `lanec-pin-checks.log`. It has the strict and
  ASan/UBSan runs of every suite and the whole-file map run.
- C64 execution results: `lanec-pin-c64-results.json`, with every PRG's
  hash: `vice-lanec-pin-gaps` (50 checks), `vice-lanec-pw` (85 programs,
  917 checks) and `vice-lanec-pin-transfer` (108 programs, 575 checks), all
  on a VICE PAL C64 with no failures.
- C64 branch outcomes: `lanec-pin-c64-cover.json` (`tools/lanec/cover.py
  record --pool`). For each suite it lists the host cases' branch arcs over
  every linked source and the frozen C64 rows. Every host arc is taken by
  some C64 program's rows, with three groups listed apart:
  - the 152 `pin-transfer-c64` rows of the 22 programs that do not fit
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

Regenerate the evidence with:

    S="pin-gaps $(sed -n 's/.*"\(pw-[a-z0-9-]*\)".*/\1/p' tests/lanec_pw_*_cases.c | sort -u | tr '\n' ' ')"
    python3 tools/lanec/cert_evidence.py lanec-pin evidence/lanec-pin-cert $S pin-transfer pin-transfer-c64 \
      --refusals $S pin-transfer pin-transfer-c64 --refusals-only pin-refuse \
      --wholefile tools/lanec/wholefile_pin.py \
      --c64 pin-gaps=lanec-pin-gaps pw=lanec-pw pin-transfer-c64=lanec-pin-transfer
    python3 tools/lanec/cover.py record evidence/lanec-pin-cert/lanec-pin-c64-cover.json $S pin-transfer-c64 \
      di-transfer-c64 --pool --not-run pin-transfer-c64=<the rows of LANEC_PIN_C64_TOO_BIG's programs> \
      --legacy pw-fixed-display=vice-legacy-dsky-fixed-display --legacy pw-cm-dynamic-display=vice-legacy-cm-dyndisp \
      --legacy pw-monitor-consumer=vice-legacy-dsky-monitor-consumer \
      --legacy pw-cm-normal-display=vice-legacy-cm-normal-display \
      --legacy pw-lamp-lifecycle=vice-legacy-dsky-lamp-lifecycle --legacy pw-cm-nv85=vice-legacy-cm-nv85 \
      --legacy pw-decimal-time-consumer=vice-legacy-dsky-decimal-time-consumer \
      --host-only 'src/lm_interbank_routines.c:9=<reason>' --host-only 'src/cm_interbank_routines.c:9=<reason>'
