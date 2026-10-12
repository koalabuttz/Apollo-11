# Run Luminary and Comanche IMU compensation as native routines

Lane D1. Milestone `imu-compensation`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

`Luminary099/IMU_COMPENSATION_PACKAGE.agc` and `Comanche055/IMU_COMPENSATION_PACKAGE.agc` are compiled native C (`src/imu_compensation.c`). Each of the file's entries, and each point where it resumes after an outside call, is a routine of the control-transfer ABI at its original address in bank 06. It runs the source from there to the point where control leaves the file and leaves Z, Q, A, L and the banks as the original does.

## Scope

Routines (`include/imu_compensation.h`):

| Luminary099 | Comanche055 | Source | Leaves at |
|---|---|---|---|
| 06,3263 | 06,3262 | 1/PIPA: PIPA scale-factor and bias compensation, IRIGX/Y/Z acceleration and drift terms | SWRETURN, or TC NOVAC with 2CADR 1/GYRO at Q |
| 06,3352 | 06,3406 | RELINT; IRIG1 after NOVAC | SWRETURN |
| 06,3507 | 06,3474 | 1/GYRO: scale GCOMP for IMUPULSE | TC BANKCALL, CADR IMUPULSE |
| 06,3535 | 06,3522 | after IMUPULSE | TC BANKCALL, CADR IMUSTALL |
| 06,3537 | 06,3524 | after IMUSTALL | ENDOFJOB |
| 06,3540 | 06,3525 | GCOMP1 (no caller in either program) | ENDOFJOB |
| 06,3555 | 06,3542 | NBDONLY: bias-only compensation job | ENDOFJOB, 1/GYRO's BANKCALL, LM TC IBNKCALL (PIPASR +3) |
| 06,3575 | | LM after PIPASR +3: DELTAT, NBD2, NBD3, FBIASSUB per axis | ENDOFJOB or 1/GYRO's BANKCALL |
| 06,3663 | 06,3636 | LASTBIAS | TC BANKCALL, CADR PIPUSE1 (CM PIPUSE) |
| 06,3665 | 06,3640 | after PIPUSE1/PIPUSE | ENDOFJOB or 1/GYRO's BANKCALL |
| 06,3701 | | GCOMPZER (LM only; Comanche has none) | SWRETURN |

- The local subroutines IRIGX, IRIGY, IRIGZ, GCOMPSUB, DRIFTSUB, FBIASSUB and DRFTSUB2 are reached only from inside the file. They are compiled into their callers with the original register effects. A return through `TC MPAC +2` or `TC BUF +1` runs the TC stored there and leaves Q at 0157 or 0132. `TC Q` leaves Q at 3.
- Each instruction is one `agc_image_ops` register effect, in source order. INDEX is an explicit address offset. The loops index 4, 2 and then the -0 that `1 + NEG1` leaves, which indexes as 0 and makes CCS take the -0 branch out of the loop.
- INHINT and RELINT set the mission io's `interrupt_enabled`.
- The Luminary IRIGX/Y/Z acceleration terms on the surface (TEM1 = SURFFBIT) run inside NBD3 as in the source.
- Destinations outside the file stop the trampoline with `EXEC_UNMAPPED` until their files have routines: IMUPULSE and IMUSTALL (IMU_MODE_SWITCHING_ROUTINES, this lane), PIPASR and PIPUSE1/PIPUSE (SERVICER), ENDOFJOB, and Comanche NOVAC.

Refused input (`EXEC_OUT_OF_SCOPE`, image unchanged), with no original producer:
- The Luminary continuation after `IBNKCALL PIPASR +3` with TEM1 other than +0 or BIT8. NBDONLY stores TEM1 from `FLAGWRD8 & BIT8` just before the call.

Branches with no producer are compiled as the source has them and documented in the code: the negative CCS outcomes of the loop counters (`NOOP`, `LGCOMP`, `COMPCHK`, `V06N30S` sit there), GCOMPSW negative after DRFTSUB2 (it stores only CCS results), and the negative outcome of `CCS A` after `MASK COMPCHK`.

## Source map

`tools/imu_compensation_source.py` reads fresh host assembly listings of both unmodified programs. It assigns every emitted word of the file to the native function that compiles it, by source address, and fails on an unowned word or an owner that covers none. It also checks each fixed constant the native code uses as a literal against the rope: BIT1-BIT12, HALF, POSMAX, NEG1/NEGONE/COMPCHK, PRIO21, PRIO31, SURFFBIT and LGCOMP.

| Mission | Words | Native ranges | Literals |
|---|---|---|---|
| Luminary099 | 282 (06,3263-3714) | 21 | 12 |
| Comanche055 | 247 (06,3262-3650) | 19 | 13 |

The 2CADR 1/GYRO after TC NOVAC and the CADRs after TC BANKCALL/IBNKCALL are the callers' operands in the generated `lm_exec_sites`, `lm_interbank_sites` and `cm_interbank_sites` tables. Nothing reads the rope at run time.

Result: `evidence/imu-compensation/imu-compensation-source-map.json`.

## Evidence

Hashes: `evidence/imu-compensation/SHA256.json`. Bulk files are assets of the `evidence-imu-compensation` release (`python3 tools/evidence.py fetch imu-compensation`).

- **Independent assembly comparison.** `make imu-compensation-differential` runs the unmodified ropes in yaAGC (host-only, `tools/imu_compensation_reference.c`). Every case runs from the same image and Z to the same stop and equals the native run in all 2,048 erasable words and the interrupt-enable state. Per-case digests are frozen in `tests/imu_compensation_vectors.h`.

  | Cohort | Cases | Stops reached |
  |---|---|---|
  | Entry runs, 20 routines x 512 states | 10,240 | SWRETURN, NOVAC, BANKCALL, IBNKCALL, ENDOFJOB as the table above lists |
  | Round trips through the Executive: 4 LM sites, 2 CM sites x 512 | 3,072 | caller's Q+1 2,189; LM BAILOUT1 (a NOVAC alarm) 126; CM NOVAC 757 |
  | Plain LM round trips, 4 sites x 128, stopping at NOVAC | 512 | caller's Q+1 232; NOVAC 280 |

  The round trips start at BANKCALL with Q at the genuine CADR word (SERVICER, P51-P53 and IMU_PERFORMANCE_TESTS_4 in Luminary; SERVICER207 and IMU_CALIBRATION_AND_ALIGNMENT in Comanche). They run BANKCALL, this file, NOVAC (Luminary, `lm_exec_routines` with a canonical Executive), the return at 06,3352 and SWRETURN, back to the caller. States include a negative GCOMPSW (bypass), small coefficients (commands of two pulses or less, no NOVAC), every EB, and surface and free-fall TEM1.

  Results: `evidence/imu-compensation/imu-compensation-proof.json`.
- **Host and sanitizer results.** `make test-imu-compensation sanitize-imu-compensation` (`evidence/imu-compensation/imu-compensation-host.log`) checks:
  - the digests;
  - that every routine reaches each exit the source gives it;
  - that the two-pulse path of 1/PIPA is reached in both programs;
  - that every round trip returns to its caller;
  - the TEM1 refusal.
- **C64 execution results.** `make vice-imu-compensation` (`evidence/imu-compensation/imu-compensation-vice.log`) runs three diagnostics, because the round trips' INTER-BANK code does not fit beside both missions:
  - every 37th entry case (277);
  - every 16th Comanche round trip (64);
  - every 16th plain Luminary round trip (32).
