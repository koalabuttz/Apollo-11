# Second review: re-cite kalcmanu-steering (LM and CM KALCMANU_STEERING.agc)

Second certificate reviewer, 2026-10-15. D1's #497 (merged as 84d550f5) refroze kalcmanu-steering. Both files were complete on docs/audits/lane-d1b-231.md. **Verdict: re-cited.**

## What changed

- **NEWJOB -0 at the task entries.** In flight, the waitlist tasks UPDTCALL, MANUSTOP and GOODMANU run with NEWJOB at -0 while DUMMYJOB idles. The frozen domain never drew that value, and the LM P63 flight run refused all 49 entries.
  - The spec adds `any_newjob`.
  - `sane_nv()` (basic routines that never name FIXLOC) now also accepts NEWJOB 077777.
  - `sane()` (interpretive and FIXLOC entries) is unchanged.
  - This is the #403 rule for task entries. It widens the domain to a value flight holds and narrows nothing.
- **Other flags.** `trusted_image` gates the image scan under `!RK_FIXED_IMAGE`. `index_carry16` makes `ixw`'s end-around carry exact. `image_w` takes `W` from agc_image_w.h. These are the same changes accepted for cm-p30-p37 and cm-p32-p33 (rev2-recite-p30-p33.md).
- **Domain text.** supported_domain names the -0 case: "NEWJOB +0 or positive (also -0 for those basic routines … spec any_newjob)".

## Evidence at 84d550f5 (reproduced)

| Check | Result |
|---|---|
| Hashes | All 6 files match SHA256.json; the 5 cited hashes match in both certificates |
| Differential | 26,624 cases equal yaAGC (52 routines) |
| Host + sanitizer | 840 checks, 0 failures each |
| VICE | 2 PRGs byte-identical; 2/2 PASS, 78 checks = vice_targets |
| `--cover-check` | verified, cases_match; host 774, C64 774, 0 missing |
| `cert_selfcheck kalcmanu-steering` | OK, no gaps; 918/918 rows equal on yaAGC; gcov 803/803 |
| `cert_precheck` (both files) | 0 FAIL (vice-manifest WARN; vice_targets checked by hand) |
