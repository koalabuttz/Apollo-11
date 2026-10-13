# AGC_BLOCK_TWO_SELF-CHECK: completion certificates

Lane A2 (restart, telemetry and support). Two `PORT_SCOPE` certificates, one per mission, for `Luminary099/AGC_BLOCK_TWO_SELF_CHECK.agc` and `Comanche055/AGC_BLOCK_TWO_SELF-CHECK.agc`, both resting on the one milestone that translates the file, `self-check` (`docs/SELF-CHECK.md`, `src/self_check_body.inc` with `src/lm_self_check.c` / `src/cm_self_check.c`). Records: `evidence/lm-self-check-complete/` and `evidence/cm-self-check-complete/`, written and checked by `tools/self_check_complete.py` (it fails on any hash mismatch, untranslated word, oracle difference, unexecuted instruction or failed check; `--check` also fails if a record would change).

| Mission | Owns | Words |
|---|---|---|
| Luminary099 | bank 43 3232-3723 | every assembled word of the file |
| Comanche055 | bank 43 3234-3725 | every assembled word of the file |

Nothing is excluded.

## Records (one per category, per mission)

- Source-to-native map: `<id>-source-map.json` names `docs/SELF-CHECK.md`, the native files with their sha256 and the source file's sha256, and the word check: every assembled word of the file lies in the milestone's range. Per word (`<id>-line-map.json`): kind (instruction or data), native location (the `L<address>` label in `src/self_check_body.inc`, Comanche055 numbering, or the operand table compiled from the listing) and whether the oracle runs executed it.
- Independent assembly comparison: `<id>-comparison.json` names the yaAGC proof record (`evidence/self-check/self-check-proof.json`: 4,096 random-state and 43 whole-rope cases per mission). The tool reruns the oracle on the unmodified rope in both modes, built with the coverage trace (`tools/cert_trace.h`), and requires the output to equal the snapshots the freeze checked byte-identical to the native output. The trace shows every instruction executed but the two words no state reaches (-1CHK's `TCF PRERRORS` on A = +0 and ADRSCHK's `TC CONTINU` on L = +0; see `docs/SELF-CHECK.md`). Instructions after EXTEND or INDEX count with their prefix, because yaAGC runs them in the same step. Data words are compiled tables checked against the rope.
- Host and sanitizer results: `<id>-host.json` names `evidence/self-check/self-check-host.log` with its sha256, PASS lines and check counts, no failures.
- C64 execution results: `<id>-c64.json` names `evidence/self-check/self-check-vice.log` (both mission PRGs with the REU rope image), no failures. The rows are chosen by native branch outcome (`tools/a2_c64_rows.py`), so they take every gcov branch arc the host cases take. The record also names both PRG sha256s from the log. `<id>-c64-cover.json` is the c64_branch_outcomes record: it names the mission's `evidence/self-check/self-check-<m>-c64-cover.json` (random-state, with host cases and rows per entry slot, every slot run on the C64) and `self-check-<m>-rope-c64-cover.json` (whole-rope) with their sha256; each lists the linked sources with their sha256 and [host, rows] arc counts, the rows and the host arcs they miss (none). The tool fails on a missed arc or a linked source changed since the cover was taken.

## What the certificate added to the milestone

- FXADRS, the fixed-fixed rope read, was untranslated. It now reads the rope image (`include/rope_image.h`), which `tools/ci_guard.py` restricts to this file, per the steward's decision; with no image loaded it refuses.
- Whole-rope cases run ROPECHK and SHOWSUM over every bank of each rope to ENDSUMS, through SUPDACAL, POSTJUMP and harness stand-ins for ADVAN and LEFT5, and the C64 runs a full ROPECHK out of the REU.

## Cross-bank entries

Every label of the file reached from another bank is a registered `agc_transfer` routine: SELFCHK (EXECUTIVE's SELFBANK BBCON, FRESH_START's LESCHK GENADR, EXTENDED_VERBS' BZF), STSHOSUM and NXTBNK (EXTENDED_VERBS' SHOWSUM). ADRS1, S+1, S+ZERO and SELFADRS are constants EXTENDED_VERBS reads. The returns from SHOWSUM's SDISPLAY and from every TC out of the file are registered too.

## Continuations outside the file

ALARM2 (ALARM_AND_ABORT), POSTJUMP and SUPDACAL (INTER-BANK_COMMUNICATION), ADVAN, NUCHANG2, NUDIRECT and SELFRET (EXECUTIVE), CHANG1 and LEFT5, SDISPLAY and verb 91's display (EXTENDED_VERBS / Pinball). ADVAN and LEFT5 are test-harness stand-ins in `tests/self_check_cases.c` until their lanes register them; integrated C64 application acceptance is outside this certificate.
