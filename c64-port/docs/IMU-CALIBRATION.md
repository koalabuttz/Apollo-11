# Translate Comanche055 IMU_CALIBRATION_AND_ALIGNMENT

Lane D1 (IMU and hardware). Milestone `cm-imu-calibration`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Comanche055/IMU_CALIBRATION_AND_ALIGNMENT.agc` (`src/cm_imu_calibration.c`) is native C in full. It is the CSM's IMU performance tests and prelaunch alignment:
- P07's drift and PIPA tests: IMUTEST, POSITON, the torquing and timing loops;
- the optimum prelaunch alignment and calibration filter (ALFLT and its loops), gyrocompassing and erection (ALWAYSG, COMPGS);
- earth-rate compensation (EARTHR, ERTHR);
- P03's optical verification with V06 N93 (GCOMPVER, OPTDATA, TARGDRVE, TAR/EREF, LITTLSUB);
- FINETIME, which reads the scalers;
- CHKCOMED, which watches channel 30 for liftoff.

## Scope

| Words | Routines |
|---|---|
| 1,077: 33,2000-3775 (1,001), 32,2000-2016 (ERTHRVSE, 15), 04,2613-2644 (LATAZCHK, 26), 02,4553-4574 (FINETIME, fixed-fixed) | 395: the entries (IMUTEST, FINETIME, EARTHR, PROUT, P11OUT, GCOMPVER, RESTEST1), every return after an outside call, every word after a DANZIG (`resume_keys`), the display and restart returns (`resume_orphans`) and the idle points |

The file's erasables are in E5 (`EBANK= POSITON`). Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines:
- basic: BANKCALL, TWIDDLE, PRIOCHNG, 2PHSCHNG, PHASCHNG, NEWPHASE, CHANG1, CHECKMM, DOWNFLAG, FINDVAC, JOBWAKE, LONGCALL, NEWMODEA, NEWMODEX and ALARM with their inline words, POSTJUMP, and ENDEXT, POODOO, ENDMARK, ENDOFJOB, JOBSLEEP and TASKOVER (no return);
- interpretive: CALL and STCALL to routines in other files, RTB PULSEIMU and SGNAGREE, and RVQ or GOTO to a caller outside;
- CHANG2, when a DANZIG finds NEWJOB positive. The Executive resumes the job at NEWOPS with LOC +1, a routine of the file;
- ONCEMORE's TC LAT(SPL) (E7,1400), the erasable program that the performance test loads (`erasable_calls`).

RTB LOADTIME, 1STO2S and CDULOGIC are native (`src/rtb_angles.c`).

## Hardware inputs

FINETIME reads LOSCALAR and HISCALAR (channels 4 and 3), and CHKCOMED reads channel 30. With the spec key `channel_inputs`, those reads compile. They read the inputs that the caller latches for the run through `AGC_CHANNEL(n)` (`include/agc_input_channels.h`). That header is interim: the Kernel lane is adding the channel fields to `agc_hardware`, and the macro will move there.

Two waits spin until hardware moves an input. Each ends the run there with `EXEC_IDLE`, and its key runs the wait once more:
- CHECKG1, when the BZF finds PIPAX unchanged;
- FINETIME +1, when LOSCALAR reads 37777 (a scaler latched for the run does not move).

FINETIME's second READ LOSCALAR (the scaler moved between its READ and the RXOR) cannot happen with latched inputs.

## Evidence

Hashes: `evidence/cm-imu-calibration/SHA256.json`. Bulk files are assets of the `evidence-cm-imu-calibration` release.

- **Source map.** `cm-imu-calibration-source-map.json` lists every emitted word with its routine. All 1,077 words are compiled, and none is untranslated.
- **Independent assembly comparison.** `make cm-imu-calibration-differential` runs 512 states per routine (202,240 cases). Every case equals yaAGC running the unmodified rope in Z, all 2,048 erasable words and the interrupt-enable state.
  - Each case latches input channels 3, 4 and 030-033 from its seed, and the oracle sets yaAGC's input channels to the same values. LOSCALAR is 37777 in a quarter of the cases.
  - NEWJOB is positive in a quarter of the cases. Those runs stop at CHANG2 at the first DANZIG, in the oracle as in the native code.
  - TIME1, TIME2 and the CDUs vary in every case. A quarter of the cases put edge values into MPAC, the top of the pushdown list and every operand the file names: +0 and -0, one LSB, 1/2, 1/4, just below 1, half-unit axis vectors (a matrix's rows a signed permutation), unit vectors in a coordinate plane and tiny vectors, with the CDUs at 0, 45, 90, 180 or 270°.
  - The states are the ones the file's own code leaves: small table indexes (POSITON, PIPINDEX, RESULTCT, NDXCTR, RUN), the CCS classes of its counters and flags, and the saved returns of its call sites. The ALFLT, compass and TAR1 loops get their own index registers, steps and pushdown pointer, because a wider index stores over CMPX1 or ALX1S and the next LXC or LXA would load data as an index.
- **Host and sanitizer results.** `make test-cm-imu-calibration sanitize-cm-imu-calibration` runs the 537 frozen cases: every 640th, plus a greedy cover of every branch outcome any case reaches (2,197 outcomes over the file, the interpreter bodies and the image ops; `outcome_cover` in `cm-imu-calibration-proof.json`).
- **C64 execution results.** `make vice-cm-imu-calibration` runs 21 diagnostics with 478 cases, chosen by branch outcome. They reach 2,194 of the 2,197 outcomes, among them DANZIG's NEWJOB +0 continuation wherever a case reaches it. The three left are that continuation out of ALCGKK, ALKCG and ALKCG2 (`src/cm_imu_calibration.c:5090`, `:5104`, `:5118`): every case that takes it runs through more routines than one diagnostic holds. The log gives each PRG's SHA-256.
