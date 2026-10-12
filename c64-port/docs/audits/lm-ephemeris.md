# Certificate review: Luminary099/LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc

Reviewed 2026-10-12 by the certificate reviewer at `port/c64-native-c` 9b1bdb76,
against the certificate drafted by milestone `lm-ephemeris` (PR #7, reworked in
PR #67 for auditor issue #27).

Verdict: **accepted**. This review also refreshes one stale hash (see below).

## What was checked

- Every #27 item is fixed:
  - The comparison cites the yaAGC oracle stream (`lm-ephemeris-source-stream.json`
    and its snapshot archive), not the source map.
  - LSPOS (= LUNPOS, 15,3664), LUNVEL (15,3775) and the eight inner frontier
    points are LOC-keyed `agc_transfer` routines
    (`src/lm_ephemeris_routine_table.c`).
  - The domain declares canonical state, interrupts and jobs, and the untested
    UNIT abort.
- All six cross-bank callers are admitted, with their QPRETs:
  - ORBITAL_INTEGRATION :165, :518, :583, :604
  - P51-P53 :372
  - S-BAND_ANTENNA_FOR_LM :94
- SOLPOS does not exist in Luminary099.
- The four evidence categories cite four distinct records.

## Reproduced

- `test-lm-ephemeris` from a clean build: the yaAGC oracle equals native on all
  65,536 states. The regenerated archive equals the committed `a2593a19…`
  byte for byte, and its decoded sha is `c3617d86…`.
- `test-lm-ephemeris` also passes 16 frozen states, 21 guards, 54 phase steps,
  and 65,536 trampoline round trips.
- `sanitize-lm-ephemeris` passes strict and under ASan/UBSan.
- `vice-lm-ephemeris` (x64sc 3.7.1 PAL) passes 12/30/6/6 checks with 0
  failures, and all four PRGs rebuild byte for byte.
- The phase and target freezers reproduce the committed vectors.
- gcov over the oracle's native producer and the routine test leaves untaken
  only these branches:
  - primitive and admission refusal guards;
  - the declared-untested UNIT 1302 abort;
  - per-instruction operand forms this file never uses.
  Every data-dependent AGC branch (BOFSET FREEFLAG, BZE POSITE, BPL POSITA, the
  UNIT overflow break) takes both outcomes.

## Refreshed in this review

`evidence/lm-ephemeris/lm-ephemeris-source-map.json` recorded the
`native_sha256` of `src/lm_ephemeris.c` from before PR #67. `tools/lm_ephemeris_source.py`
regenerates it with only that field changed (`8c1430b1…` to `d775cf3f…`). The
ownership of all 74 words is unchanged. The committed map, its `SHA256.json`
entry and the certificate hash are updated to the regenerated file.

## Notes for the lane

- Make `test-lm-ephemeris` fail when the fresh source map differs from
  `evidence/`, so this drift is caught.
- `test-` and `sanitize-lm-ephemeris` still race on the same `.xz.part` when
  run in parallel (noted in #27).
- The C64 runs check each phase step through the trampoline. The whole chain
  into a native caller runs on the host only, as `external_continuations`
  declares.
