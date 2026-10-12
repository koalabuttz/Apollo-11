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
- **Host and sanitizer results.** `make test-sband-antenna sanitize-sband-antenna` runs the 1,552 frozen cases (every 32nd).
- **C64 execution results.** `make vice-sband-antenna` runs seven diagnostics with 194 cases. Every routine has C64 cases.
