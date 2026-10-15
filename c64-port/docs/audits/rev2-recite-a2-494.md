# Second review: re-cite lm-p32-p35 (Luminary099/P32-P35_P72-P75.agc)

Second certificate reviewer, 2026-10-15. A2's #494 (merged as 7a631914) refroze lm-p32-p35 with `superbank_bit`, `trusted_image`, `index_carry16` and `image_w`. It was complete on docs/audits/lane-e-194.md. **Verdict: re-cited.**

## Code

- **Flags.** These are the same four changes accepted for cm-p30-p37 and cm-p32-p33 (rev2-recite-p30-p33.md).
  - The superbank guard refuses only on channel 7 bit 0100. It sits at the entry of the file's three indexed fixed reads (CHANG2 resumes r70744, r70752, r72042), before TRACE or any write.
  - The image scan in `sane()` sits under `!RK_FIXED_IMAGE`.
  - `ixw` has an exact end-around carry.
  - `W` comes from agc_image_w.h.
- **Spec diff.** Only those four keys. The certificate changes only its hashes, plus a supported_domain sentence: no C64 row is truncated now.
- **History.** The PRG drift from #462's shared runner change is gone: all 49 PRGs match the log.

## Domain (pins checked against every writer)

- **CSIALRM (E7,1613) in 1..7.** ALMXIT's `LXC,1` is its only reader, and it is reached only after SCNDSOL's `SXA,2` stores 1..7. Nothing in the CSI loop writes the overlapping TITER, XSMD or PIF words in between.
- **RTX1 / RTX2 (E5,1776-1777) in {-2, -10} / {0, 2}.** These are the values set by SELECTMU/SETMUER, INTEGRATION_INITIALIZATION and the X1/X2 copies in P30_P37, P40-P47 and GENERAL_LAMBERT.
- **ALMXITA X2 in 1..7.** Its only caller is `BZE ALMXITA` after `AXT,2 1D`.
- **PUSHLOC pins.** These are at ROTATE and ADVANCE, whose callers are only in this file. `lmi_push` has no PUSHLOC refusal, so the pin narrows the cases, not the code.
- **Rules #403, #385 and #414:** N/A. The file has no task or interrupt entry, no RESUME and no BOVB.

## Evidence at 7a631914 (reproduced)

| Check | Result |
|---|---|
| Hashes | All 6 files match SHA256.json and the certificate; release assets fetched |
| Differential | 215,552 cases equal yaAGC (421 routines); resume_flow 294/294; case set = proof |
| Host + sanitizer | 842 checks, 0 failures each |
| VICE | 49 PRGs byte-identical; 49/49 PASS, 497 checks = vice_targets |
| `--cover-check` | verified, cases_match; host 2,745, C64 2,745, 0 missing, no truncated rows |
| `cert_selfcheck lm-p32-p35` | OK, no gaps; 1,339/1,339 rows equal on yaAGC; the shared interp_case.c / interp_cut.c pins are NOTEs (#450) |
| `cert_precheck` | 0 FAIL (vice-manifest WARN; vice_targets checked by hand) |

This PR also corrects native_coverage's scope from 135 to 421 routines, the count the code and docs/LM-P32-P35.md use.
