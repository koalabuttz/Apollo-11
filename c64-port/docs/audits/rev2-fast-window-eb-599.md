# Second review: the fast_window EB check (#599)

Second certificate reviewer, 2026-10-18. Lane E's #599 (head 935ec8aa, merged as aa339613) adds an EB check at entry to the four `fast_window` specs: cm-entry-dap, cm-rcs-dap, cm-imu-calibration and lm-kalman. **Verdict: accepted, and the CM entry DAP, RCS-CSM DAP and IMU calibration certificates are re-cited to this record.**

Previous records:

- CM_ENTRY_DIGITAL_AUTOPILOT: rev2-recite-g527.md, plus the #586 fast_window review (Notes 1 of that review is the gap this PR closes).
- RCS-CSM_DIGITAL_AUTOPILOT: rev2-recite-g515.md, plus the #588 fast_window review.
- IMU_CALIBRATION_AND_ALIGNMENT: lane-d1-208.md, plus reviewer 1's reviews of the fast_window refreeze.

## Code

- **The check.** `tools/interp_compile.py` adds `if(W[3]!=<ebank<<8>)return 0;` to every `sane` and `basic_sane` of a `fast_window` file. The window forms treat an operand in 01400-01776 as a cell of `AFR_EBN`, so an entry under another EB is now refused instead of reading the wrong cell. Files without `fast_window` are unchanged.
- **Exempt routines** get `basic_sane_any_eb`, the same checks without the EB line:
  - `sets_eb()` exempts a basic routine whose first two words are `CA K` (K in fixed memory) and `TS EBANK` (054003). CM/DAPON and CM/DAPIC load EBAOG, which is EBANK6 and so equal to AFR_EBN. IMU 33,3034 loads EBANK7 and then leaves at once by `TC LAT(SPL)`, before any erasable access.
  - The new `any_eb_entries` key exempts IMU's FINETIME and FINETIME +1. They touch only L, LOSCALAR/HISCALAR and the fixed constant POSMAX. P06's standby code calls them under its own EB.
  - No other `TS EBANK` appears in the four generated files.
- **Domains.** All three certificates now state the rule: other EBs are refused, with the exempt routines named.

## Evidence (at 935ec8aa; master equals it for all three specs)

- **Precheck.** `evidence.py fetch` passes and all hashes match. `cert_precheck` gives 0 FAIL on all four files. The NEWJOB WARN on CM entry DAP predates this PR.
- **cm-entry-dap.** Differential 7,168 cases over 138 routines equal the original programs. Host 224 checks. VICE 83 checks, 0 failures (was 82), matching vice_targets. Edges are host = C64 at 1,338 and branches 922 = 922; the 18 new edges are both outcomes of the EB checks.
- **cm-rcs-dap.** Differential 4,352 cases over 71 routines. Host 136 checks. VICE 45 checks, 0 failures.
- **cm-imu-calibration.** Differential 176,640 cases over 345 routines. Host 488 checks. VICE 468 checks, 0 failures; the C64 cover file is updated.
- **lm-kalman** (refrozen too, not re-cited here). Differential 2,048 cases. Host 64 checks. VICE 3 checks, 0 failures.

## Notes (non-blocking)

1. `sets_eb()` does not check that K equals `ebank << 8`. Today that only matters at IMU 33,3034, which exits before any erasable access. Lane E will add the check the next time it touches the generator.
2. The generated `basic_sane_any_eb` is followed by a second copy of its leading comment: `body = c[bs0:eb_at] + c[eb_at+1:]` picks up the comment lines appended just before. This is cosmetic, and Lane E will fix it at the same time.
