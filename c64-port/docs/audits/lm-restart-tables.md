# Auditor review: Luminary099/RESTART_TABLES.agc

Reviewed 2026-10-12 by the port auditor at `port/c64-native-c` 29275b10, against
the pending certificate drafted by milestone `lm-restart-select` (PR #9).

Verdict: **accepted**. `RESTARTS_ROUTINE.agc`, drafted in the same milestone, is
not accepted yet (see the auditor issue filed for it).

## What was checked

- All 132 table words are generated from the assembler listing into
  `src/lm_restart_table_data.inc` and checked against the rope and the source
  text; nothing reads the rope at runtime.
- All 40 table entries, including the EQUALS aliases, are selected by native
  RESTARTS. Each entry's priority or delta, 2CADR and BBCON is compared with
  yaAGC across the full erasable image.
- Each 2CADR reaches its service through the GOLOC erasable words and the
  native control-transfer ABI.

## Reproduced

- `test-lm-restart-select`: "src/lm_restart_table_data.inc matches 132
  Luminary099 RESTART_TABLES words"; 6144 selection and 4096 composition
  cases, 40 table entries, 12703 checks, 0 failures.
- `sanitize-lm-restart-select`: same under ASan and UBSan, 0 failures.
- yaAGC differential rerun: "6144 selection and 4096 composition cases equal
  the original program"; snapshot hashes match the committed proof.json.
- `vice-lm-restart-select` (VICE x64sc 3.7.1, PAL): 640 checks, 0 failures,
  stopping on the same mailbox and cycle (2827973261) as the committed log.
- Every certificate evidence file and native file hash matches.

## Note for the lane

This certificate cites the same evidence files as the RESTARTS_ROUTINE
certificate. When those are regenerated, update this certificate's sha256
values too, or `tools/port_coverage.py` fails.
