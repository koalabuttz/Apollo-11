# Second review: re-cite cm-p30-p37 and cm-p32-p33

Second certificate reviewer, 2026-10-15. Both files were complete on docs/audits/lane-e-194.md. #488 (cm-p30-p37) and #480 (cm-p32-p33) refroze them with `superbank_bit`, `trusted_image`, `index_carry16` and `image_w`. **Verdict: both re-cited.** Domains and pins are unchanged except as noted.

## Code changes

- **superbank_bit.** yaAGC (agc_engine.c 723/803) maps FB 30-37 to 40-47 only when channel 7 has 0100 set, so refusing on 0100 alone is faithful. The guard sits at the routine's entry (cm-p30-p37 r64135/r64140, cm-p32-p33 r63610/r63616), before TRACE or any write.
- **index_carry16.** `ixw` computes `sum=(a+b)&0177777; if(sum<a) sum+1`, an exact end-around carry.
- **trusted_image** gates only the whole-image scan in `sane()` under `!RK_FIXED_IMAGE`. **image_w** takes `W` from agc_image_w.h.

## cm-p30-p37 (Comanche055/P30-P37.agc), at c2283298

- The case set is unchanged (cases_sha256 c3dc0a06, oracle 850bcdc4). The differential passes 73,216 cases over 143 routines, and resume_flow passes 56/56.
- Host and sanitizer pass 572 checks each.
- VICE: 6 PRGs byte-identical, 179 checks = vice_targets = proof rows.
- `--cover-check` verifies: 919/919, 0 missing. `cert_selfcheck` is OK; the agc_transfer.c linkage arc is a shared NOTE. `cert_precheck` gives 0 FAIL.
- This PR corrects the scope text from 90 to 143 routines, the count the proof and doc use.

## cm-p32-p33 (Comanche055/P32-P33_P72-P73.agc), at ee0dd3df9

- `--cover-check` verifies: 208,384 cases, cases_match, 477 C64 rows (2 truncated, both rebuilt), host 1,852 / C64 1,862, 0 missing.
- `cert_selfcheck cm-p32-p33` gives OK, no gaps (gcov 1,852/1,852).
- `cert_precheck --fetch` gives 0 FAIL. The VICE log has 41 PASS and 0 failures, 477 checks = vice_targets.
- supported_domain changes only from three truncated rows to two.
