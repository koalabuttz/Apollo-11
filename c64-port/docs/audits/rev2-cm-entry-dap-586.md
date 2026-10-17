# Second review: cm-entry-dap on fast_window (#586)

Second certificate reviewer, 2026-10-17. Lane E's #586 (head ced9f76c) refreezes `cm-entry-dap` with `fast_window`. It also lets `tools/interp_compile.py` accept `fast_window` together with `basic_parts`. **Verdict: accepted and re-cited.** The previous record is rev2-recite-g527.md.

## Code

- **interp_compile.py.** The only change is that `fast_window` with `basic_parts` is no longer refused; `basic_split` is still refused. `--check` regenerates all 80 specs byte-identically, and cm-entry-dap is the only spec that uses both keys.
- **Entry binding.** All 137 `_abi` wrappers start with `AFR_BIND(s);AFO_FIX(s);`, before `basic_sane` and before the part runs. Parts hand off by setting Z and come back in through an `_abi`. The hot table uses the same wrappers.
- **Generated C.** Beyond the bind lines, only three lines change: the include becomes agc_fast_redirect.h, `AFR_EBN` is set to 6, and `W` becomes `AFR_S(s)->words`. No routine body changes.
- **Routing:**
  - A constant operand in 01400-01776 under EB 6 goes to `afw_*`.
  - Other constant plain cells go to `afo_*`.
  - Central registers, editing registers 020-023, indexed operands and `TS EBANK` stay on `ai_*`.
  - agc_fast_ops.c and agc_fast_window.c are unchanged. Their tests pass: 200,000 afo cases, 50,000 afw cases and 1,000,000 MP/DV cases.
- **Domain.** It is unchanged; no pins moved. Every flight caller runs under EB 6:
  - READGYMB is under CM/GYMDT. SETJTAG, ATTRATES and JETCALL are under AOG.
  - CM/DAPON and CM/DAPIC set EB themselves before touching a window cell.

## Evidence (at ced9f76c)

- **Precheck.** `ledgers.py --validate` and `evidence.py fetch` pass, and `cert_precheck` gives 0 FAIL. Its NEWJOB WARN predates this PR.
- **Hashes.** All 5 evidence hashes match the files.
- **Differential.** It was re-run: 7,168 cases over 138 routines equal yaAGC, and the stop table is identical.
- **Cover check.** It verifies: 82 C64 rows and 910/910 outcomes.
- **Host runs.**
  - Host and sanitizer give 224/224 each.
  - The hot table passes 26 entries.
  - A `-DAFR_GENERIC` build also gives 224/224.
- **C64.** The 3 C64 PRGs rebuild byte-identically to the shas in the VICE log. Their 36 + 41 + 5 = 82 checks match vice_targets.

## Notes (non-blocking)

1. **No EB check at entry.** `basic_sane` does not require EB = 6. Under another EB a window operand would read the wrong cell instead of being refused. Flight never enters under another EB, and lm-kalman and cm-imu-calibration have the same gap. A tooling follow-up should emit an EB test for fast_window files, except at CM/DAPON and CM/DAPIC. This has gone to the steward.
2. A duplicate `'trusted_image': True` key was removed from cm-jet-select. Its generated code is unchanged.
3. The gcov counts moved from 913 to 910 branches and from 1262 to 1320 edges, because the fast-op sources are now linked. Nothing is missing.
4. CM-ENTRY-DAP.md says "every image access" goes through the redirect. `ai_word`/`ai_put` do not, though they act on the same bound image.
