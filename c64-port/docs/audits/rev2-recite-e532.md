# Second review: re-cite r30 after #532 (Luminary099 + Comanche055 R30.agc)

Second certificate reviewer, 2026-10-15. Lane E's #532 refroze r30 with `trusted_image`, `image_w` and `index_carry16`. Both files were complete on docs/audits/lane-e-235.md. Reviewed at the PR head eeafcfe7. **Verdict: both certificates re-cited.**

## Code

- **Flags.** These are the changes accepted for cm-p30-p37 (rev2-recite-p30-p33.md): the image scan in `sane()` sits under `!RK_FIXED_IMAGE`, `W` comes from agc_image_w.h, and `ixw` has an exact end-around carry. There is no superbank change and no other semantic change; no shared tool or tests/*.c changes.
- **Certificate.** Only the proof, VICE-log and cover hashes change, plus vice_targets 260 to 259. The domain text is unchanged: host and C64 diagnostics keep the full-image scan, and only RK_FIXED_IMAGE flight builds, whose image is canonical by construction, skip it.
- **260 to 259.** Shared-source changes on the base since the last freeze raised the host arcs from 1,394 to 1,411, and the recomputed row selection changed three CM rows (CM 134 to 133 rows, LM 126). The group repack only narrows KEEP masks, and each mission's groups still keep every routine (LM 106/106, CM 114/114).

## Evidence at eeafcfe7 (reproduced)

| Check | Result |
|---|---|
| Hashes | All 6 files match SHA256.json and both certificates; release assets fetched |
| Differential | 112,640 cases (220 routines) equal yaAGC; case set sha = proof |
| Host + sanitizer | 880 checks, 0 failures each |
| VICE | 6 PRGs byte-identical; 44 + 56 + 26 + 54 + 72 + 7 = 259 checks = vice_targets |
| `--cover-check` | verified, cases_match; host 1,382, C64 1,382, 0 missing |
| `cert_selfcheck r30` | OK, no gaps |
| `cert_precheck` (both files) | 0 FAIL (vice-manifest WARN; vice_targets checked by hand) |

**NOTE.** The scope text in port-coverage.json still says 66 (LM) and 74 (CM) routines, where the proof has 106 and 114; this PR corrects it.
