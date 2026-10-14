# Certificate review: Luminary099 P40-P47 after PR #410

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
1ff2eb24 (PR #410 merged), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/P40-P47 | `lm-p40-p47` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL (the one WARN is the uncached nightly VICE manifest). The
  `source_revision` is 911e5c02.
- **Refusal sites.** There are 422:
  - 385 entry guards: 275 `at_newops`, 21 `at_basic` and 89 `at_basic_nv`;
  - the TC Q/TC E guards;
  - the `lmi_switch` guards of SET, CLEAR and CLRGO;
  - the BOV dispatcher defaults;
  - the `agc_ch_op` calls, which refuse only without a hardware record.

  Some of these exits cannot trigger:
  - The TS-overflow skips at `src/lm_p40_p47.c` 3387, 3417, 3830, 4032,
    4278 and 4305. A is a CAF constant or the ±1 an overflowing TS leaves.
  - The CCS-negative exits at 3358, 4260 and 5252, which follow a MASK.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py lm-p40-p47 --cases 512` (the frozen
  size) prints PASS: all 197,119 cases equal across 385 routines, none
  refused, none waiting for a channel.
  - The case set hashes to the proof's `cases_sha256` (f40878c0) and the
    oracle rows to `oracle_sha256` (7eb33c7e).
  - A salted case set (`IRC_SALT`) gives 197,120 more equal cases.
  - The one excluded case resumes at 27,3021 with PUSHLOC past the push-down
    list. A vector PUSH zeroes QPRET, and the BOFF STEERSW QPRET at 27,2471
    then cycles through erasable CADR 0 on both sides (yaAGC at its step
    limit). In flight QPRET always holds CALL S40.8's fixed return.
  - The strict `--cover-check` verifies 468 rows (27 truncated rows rebuilt),
    0 missing.
- **Host and sanitizer.** `make test-lm-p40-p47 sanitize-lm-p40-p47` gives
  308 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL.
  - Its 40 PASS lines sum to 468 checks. That equals `vice_targets`, the
    proof's C64 rows and the rows in `tests/lm_p40_p47_c64_vectors.h`.
  - All 40 PRGs rebuilt at the tip are byte-identical to the logged ones.
- **Independent C64 branch check.**
  - All 468 header rows match a regenerated case, and the regenerated set
    hashes to the frozen one.
  - Per-object gcov over all host cases shows 0 missing,
    `include/agc_channel_ops.h` included.
  - The 40 KEEP-group builds with `-DIRC_CUTS` take all 2,564 host
    outcomes, 0 missing.
  - The 27 truncated rows' cuts and digests equal the record, and yaAGC run
    to each cut equals the native run.
- **Channels.**
  - The 9 channel orders run through `agc_ch_op` on the program's
    `agc_hardware`: WAND/WOR CHAN12 in TRIMGIMB and PITCHOFF, and RAND/WRITE
    DSALMOUT and WOR CHAN14 in ENGINOF3.
  - Every case sets channels 11, 12 and 14 on both sides and compares them
    after the run. Channel 14 is set without the IMU CDU and gyro drive bits.
  - As a check, dropping the channel-14 store makes 505 compared cases and
    one C64 row differ.
  - The file reads no input channel.
- **Accounting.** The 931 words are 895 compiled words in 385 routines, plus
  36 constants:
  - 22 fixed operands;
  - 5 inline basic operands;
  - the 9 data words SEC15DP to 26SECS (36,3747-3757), which other files
    read.

  Every value equals the assembled listing. The file has no BOVB TCDANZIG
  (#414).
- **Domain.**
  - P40, P41, P42 and P47 are started only from the V37 table
    (`FRESH_START_AND_RESTART.agc` 1075-1078) through SPVAC.
  - The Waitlist tasks and their returns use `sane_nv`, which takes any
    FIXLOC and NEWJOB -0. These are TRIMGIMB, PITCHOFF, STARTP47, ENGOFTSK,
    COASTSET, IMPLBURN and ENGINOF1-4, plus the FIXDELAY, VARDELAY, TWIDDLE,
    NOVAC and IBNKCALL returns. That covers STARTP47's Waitlist restart
    (05014) and the 4.11 ENGOFTSK restart (40114).
  - The 21 VAC-guarded basic routines run only in FINDVAC or SPVAC jobs: the
    S40.13, S40.9, POSTBURN and REDO6.7 restarts and the SERVICER exits.
  - Every phase-change re-entry (36,3151, 3344, 3412, 3452, 3475, 3646) is a
    registered routine. The file has no RESUME.
  - The POODOO1 (12,981) and ALARM (8,534) stops of the divide and
    square-root orders all equal yaAGC.
  - `trusted_image` affects only `RK_FIXED_IMAGE` builds, and no cited build
    is one.
- **Not blocking.**
  - The channel orders are 18 words, not the 17 the scope lists; 27,3222 is
    the one missing.
  - The latched inputs 030-033 are zeroed by the channel edits in every case,
    not "unchanged".
  - The counters 010-061 are 0 in every case, so the TIME2 that ENGINOF4 and
    NSTEER read is always 0. Every outcome of the TIG arithmetic is still
    taken.
  - `entry_contracts` gives the VAC and NEWJOB guard only "at NEWOPS", but
    the 21 basic routines apply it too.
