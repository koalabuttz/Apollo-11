# Certificate review: LM and CM DISPLAY_INTERFACE_ROUTINES after PR #329

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
5cdc9bee (PR #329 merged), with the yaAGC reference rebuilt from the tip.
Earlier rejection: #125.

| File | Milestones | Verdict |
|---|---|---|
| Luminary099/DISPLAY_INTERFACE_ROUTINES | `lanec-di-cert`, `lanec-di-transfer` | accepted |
| Comanche055/DISPLAY_INTERFACE_ROUTINES | `lanec-di-cert`, `lanec-di-transfer` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py verify` passes for the eight DI milestones
  and `tools/cert_precheck.py` gives 0 FAIL for both files. The WARNs are the
  unfetched nightly VICE manifest and the 686 refused cases, counted twice as
  1,372. The `source_revision` is 911e5c02. The refusal sites are:
  - the entry-contract guards (`lanec_registers_valid`, bank 10);
  - the index-domain guards: COPINDEX/COPMPAC 0-2, LOCCTR, FACEREG/TEMPOR2
    0/1/-0, the OUTHERE offset, and JOBXCHS A ≤ 1, which matches
    SETMARK/SETNORM's CAF ZERO/ONE;
  - `caller_ok` on TC Q;
  - dispatcher defaults.
- **yaAGC comparison, rerun by the reviewer.** `tools/lanec/diff.py` on all
  ten suites, 22,364 cases, gives 0 differing; 686 are refused unchanged.
  - Every diff summary and oracle CRC file is byte-equal to the evidence.
  - `routines.py di --check` passes: 165 LM / 164 CM routines, all 36 callee
    returns per mission.
  - `wholefile.py` regenerates both source maps byte-equal. The oracle
    executes 612/612 LM and 609/609 CM instruction words.
- **Host and sanitizer.** The strict and ASan/UBSan runs of all ten suites
  pass with the counts in `lanec-di-checks.log`.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL.
  - Its 12 PASS lines sum to 397 checks. That equals `vice_targets`, the
    cover record's rows and the header rows. The four `di-transfer-c64m`
    programs give 111.
  - The six module PRGs rebuild byte-identical at 5cdc9bee.
  - The six transfer PRGs rebuild byte-identical from e21957ca, the lane's
    evidence commit. At the merge they differ through the base's
    `src/agc_transfer.c` transfer-step speed-up. Host results are unchanged,
    and the PR head 11134deb carries the same code as the merge. Its CI
    `c64 (0)` job passed.
- **Independent C64 branch check.**
  - All 397 header rows match a regenerated case: the oracle CRC, or the
    unchanged state for the 19 refused rows.
  - Each of the 111 c64m rows sits in exactly one program.
  - A rerun of `cover.py record --pool` gives 0 missing.
  - The 229 arcs marked host-only are in the Executive sources (executive.c,
    lm_exec_*, lm_basic_*, lm_admission_image.c, p66_native.c), the
    di-transfer runner's set-up, or `agc_classify`. None is in DI, INTER-BANK
    or transfer code. Lane A's `vice-lm-exec-jobs` links all of these sources,
    and every DI-side step of the Executive round trips is a registered return
    point taken by C64 rows.
- **#125 items.**
  - BANKJUMP's Q<0100 and overflow paths are taken on the C64
    (`src/lm_interbank.c` and `src/cm_interbank_complete.c` 18/34/35/47).
  - So are the SAVELOCS return (`lanec_di_dsp.c` 187) and arcs 4 and 6 of
    `lanec_di_mark.c` 71.
  - The mark, sleep, idle and nvdsp round trips run on the C64 in both
    missions, from real callers: POSTJUMP PINBRNCH, MARKEND, and GOMARK* or
    KLEENEX from SXTMARK, P20-P25, P51-P53, EXTENDED_VERBS and
    AGS_INITIALIZATION.
- **Accounting.** LM 670 and CM 667 words: six module owners plus the
  39-word constant table, no gaps.
- **Domain.**
  - Every caller runs in job context (V37, V46 PRESWTCH, mark and display
    jobs). INITDSP is the X.1 restart's FINDVAC job.
  - The file has no RESUME, no PHASCHNG and no task entry. Its code reads
    neither FIXLOC nor NEWJOB.
  - Comanche round trips stop at the Executive pending `cm_exec_sites`, as
    the certificate states.
- **Not blocking.**
  - `docs/LANEC-DI-{DSP,MARK,IDLE}.md` and the module VICE text logs still
    give older C64 row counts: 37/36/35/38 for dsp/mark/play/idle, against
    42/46/36/43.
  - The next nightly manifest should record the merge-tip transfer PRGs.
  - The C64 rows are split by mission, so the CM rows leave some shared-code
    arcs to the LM rows in the same object.
