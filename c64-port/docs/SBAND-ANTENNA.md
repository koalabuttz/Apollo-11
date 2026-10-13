# Translate S-BAND_ANTENNA_FOR_LM and S-BAND_ANTENNA_FOR_CM

Lane D1 (IMU and hardware). Milestone `sband-antenna`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/S-BAND_ANTENNA_FOR_LM.agc` (`src/lm_sband_antenna.c`) and `Comanche055/S-BAND_ANTENNA_FOR_CM.agc` (`src/cm_sband_antenna.c`) are native C in full. They are R05, which V64 starts as a job: it advances the state vector by conic integration to now, forms the unit vector to the Earth (from the Moon's sphere, through LUNPOS), rotates it into the vehicle frame, and displays the antenna pitch and yaw angles (V06 N51) until the crew terminates.

## Scope

| Program | Words | Routines |
|---|---|---|
| Luminary099 | 116 (42,3606-3771) | 58: SBANDANT, CONV3, CONV4, CONV5, COVCNV, NOADJUST, SBANDEX, the display returns, and every word after a DANZIG |
| Comanche055 | 80 (42,3652-3771) | 39: the same structure, with EISOI |

Both files are in bank 42, so they run as FB 32 under superbank 4. The compiler keys and decodes them that way (`docs/INTERP-COMPILE.md`).

Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines:
- basic: BANKCALL (R02BOTH, GOMARKFR, and in Comanche DELAYJOB), PRIOCHNG (Luminary only), BLANKET, B5OFF, ENDOFJOB, ENDEXT;
- interpretive: CALL and STCALL to LEMCONIC or CSMCONIC, LUNPOS, CDUTRIG and *SMNB*;
- CHANG2, when a DANZIG finds NEWJOB positive (`resume_keys`). The Executive resumes the job at NEWOPS with LOC +1, which is the key of the next routine.

RTB LOADTIME is native (`src/rtb_angles.c`). SIN, COS, and ASIN (Luminary) or ACOS (Comanche) are the compiled interpreter bodies.

## Evidence

Hashes: `evidence/sband-antenna/SHA256.json`. Bulk files are assets of the `evidence-sband-antenna` release.

- **Source map.** `sband-antenna-source-map.json` lists every emitted word with its routine: 116 Luminary words and 80 Comanche words, none untranslated.
- **Independent assembly comparison.** `make sband-antenna-differential`: 512 states per routine and program (49,664 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. The cases run with the superbank bit set and a random EB. NEWJOB is positive in a quarter of them; those runs stop at CHANG2 at the first DANZIG, in the oracle as in the native code.
  - X2 (VAC+39D), which SLOAD BHIZ X2 tests for the sphere, is 0 (Earth), 2 (Moon) or -0.
  - TIME1, TIME2 and the CDUs vary in every case; TIME1 is near 0 or just below its 37777 wrap in two fifths of them.
  - A quarter of the cases put edge values into MPAC, the top of the pushdown list and every operand the file names (RATT, VMOON, REFSMMAT, RLM or RCM among them): +0 and -0, one LSB, 1/2, 1/4, just below 1, half-unit axis vectors (a matrix's rows a signed permutation), unit vectors in a coordinate plane and tiny vectors. In those cases the CDUs sit at 0, 45, 90, 180 or 270°.
- **Host and sanitizer results.** `make test-sband-antenna sanitize-sband-antenna` runs the 1,564 frozen cases: every 32nd, plus a greedy cover of every branch outcome any case reaches (1,325 outcomes over the file, the interpreter bodies and the image ops; `outcome_cover` in `sband-antenna-proof.json`).
- **C64 execution results.** `make vice-sband-antenna` runs 15 diagnostics with 132 cases, chosen by branch outcome. They reach all 1,325 outcomes the host cases reach, among them DANZIG's NEWJOB +0 continuation and ACOSOVF in the LM ASIN body. The diagnostics build without inlining (`c64_flags`), so the longest of those runs fit one diagnostic. The log gives each PRG's sha256.
