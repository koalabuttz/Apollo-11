# Second review: lm-p51-p53 after #486 (Luminary099 + Comanche055 P51-P53.agc)

Second certificate reviewer, 2026-10-15. #452 rejected the P51-P53 certificate: GRABGRAV (15,2731), a TWIDDLE waitlist task, ran its basic routines under the job checks `sane()` makes (FIXLOC a VAC area, NEWJOB not -0), which a waitlist task cannot promise. D1b's #486 (merged as d8808d46) fixed it. Both files were complete on docs/audits/lane-d1b-245.md. **Verdict: accepted, both certificates re-cited; #452 closed.**

## Code

- **The fix.** The spec sets `any_fixloc` and `any_newjob` for LM only (`per_mission()` in interp_compile.py now takes a {mission: bool}). Generated code puts `at_basic_nv` (`sane_nv`: no VAC test on W[0120], NEWJOB -0 allowed) at the 159 LM basic routines that neither name FIXLOC/NEWJOB nor enter the interpreter, including r32731, r32733 and r32737. This is the #403 rule.
- **Comanche.** The flags stay off: Comanche055/P51-P53.agc has no TWIDDLE, WAITLIST, TASKOVER, VARDELAY or LONGCALL, so its basic routines run only in P51-P53's own jobs. cm_p51_p53.c has no `sane_nv`.
- **Cases at the fixed points.** Of 512 cases each, FIXLOC outside a VAC area: 257 / 265 / 222; NEWJOB -0: 128 / 167 / 128. All equal yaAGC.
- **Other any_fixloc specs** (shared compiler change): lm-aotmark, lm-rcsmon and lm-sps-backup `--cover-check` verified, 0 missing; `interp_compile --check` passes on 11 specs.
- **CSM_GEOMETRY.** #486 also narrows its supported_domain ("any other OVFIND word" to "a large OVFIND word"). A narrower claim needs no new evidence.

## Evidence at d8808d46 (reproduced)

| Check | Result |
|---|---|
| Hashes | 7 files match SHA256.json and both certificates; release assets fetched |
| Differential | 655,872 cases (1,281 routines x 512) equal yaAGC |
| Host + sanitizer | 3,730 checks, 0 failures each |
| VICE | 66 PRGs byte-identical; 66/66 PASS, 1,565 checks = vice_targets |
| `--cover-check` | verified, cases_match; host 5,663, C64 5,663, 0 missing |
| `cert_selfcheck p51-p53` | OK, no gaps; interp_case.c / agc_image_ops.c pins are shared-source NOTEs (#450) |
| `cert_precheck` (both files) | 0 FAIL (vice-manifest WARN; vice_targets checked by hand) |
