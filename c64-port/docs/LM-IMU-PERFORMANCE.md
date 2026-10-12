# Run the Luminary IMU performance tests (P07) as native routines

Lane D1. Milestone `lm-imu-performance`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

`Luminary099/IMU_PERFORMANCE_TEST_2.agc` and `Luminary099/IMU_PERFORMANCE_TESTS_4.agc` are compiled native C (`src/lm_imu_performance.c`). Both files are bank 37 code of P07, the IMU performance test: 585 emitted words at 37,2002-3112. Each entry of the files, and each point where they resume after an outside call or an RTB, is a routine of the control-transfer ABI at its original address. There are 40 basic routines keyed by Z and 14 interpretive routines keyed by LOC at NEWOPS 06070. FINETIME (fixed-fixed 04102-04123, which CHECKG calls) is compiled in as a local subroutine.

## How the code is written

- Basic code: each instruction is one `agc_image_ops` effect on the image, under a case label at its address, in source order. Straight-line code falls through as the source does. A TC or TCF inside the files is a `goto`; one to another file ends the run with Q and Z set. INDEX is an explicit address offset. CCS has a label at every slot.
- Interpretive code: each order-pair word (and each store word) is a case of its own. It runs the compiled `lm_interp_steps` and `lm_interp_orders` paths with the pair's words as literal arguments. MXV, VXM and LXC came from PR #66. SIN and COS are the original bank-0 bodies in `src/sband_trig.c`. Branches (BPL, BMN, TIX, BZE/GOTO, CALL, RVQ, RTB, EXIT) follow the outcomes those paths return.
- DANZIG at the end of a pair goes on to NEWOPS with NEWJOB +0 and to CHANG2 with NEWJOB positive.
- The erasable returns the code executes (TC Q, TC QPLACE, TC QPLACES, TC ZERONDX) are run as TC to the word they hold. So are `TC INTPRET` from basic code and RTB's trip through SWCALL.
- Fixed operands are the constants the code names (DEC585 through GEORGEK at 37,3055-3112, OMEG/MS, DPPOSMAX). `tools/lm_imu_performance_source.py` checks them against the rope. Nothing reads the rope at run time.

A run ends where control leaves the files:
- a basic address of another file: BANKCALL, TWIDDLE, FINDVAC, CHANG1, CHANG2, JOBSLEEP, JOBWAKE, ENDOFJOB, TASKOVER, LONGCALL, NEWMODEX, NEWMODEA, ENDEXT, DOWNFLAG, ALARM, CCSHOLE or VARALARM;
- NEWOPS with LOC outside the files: CALL and STCALL to CALCGA, ERTHRVSE and EARTHR callers elsewhere, RVQ or GOTO to a caller outside;
- the target of an RTB in another bank: 1STO2S, CDULOGIC, SGNAGREE, LOADTIME or PULSEIMU;
- an erasable Z other than the returns above (TC ATIGINC in EBANK 7).

After `RTB EXIT 1STO2S` (37,2025) the original continues at DANZIG with the EXIT still pending in EDOP. No ABI key covers that point, so the run stops at 1STO2S.

Two keys are hardware events rather than calls:
- 02354 is CHECKG after a PIPA pulse moved PIPAX between its two reads. Without one, CHECKG only leaves through CHANG1.
- 04112 is FINETIME after LOSCALAR moved between its READ and the RXOR.

io is `lm_ipt_io`. INHINT and RELINT set `control.interrupt_enabled`. READ LOSCALAR and READ HISCALAR read the frozen scaler words.

## Refused input

Refused input ends `EXEC_OUT_OF_SCOPE` with the image and io unchanged. A refusal can come after a run has started, so the routine saves the entry image in `io->undo`, which the caller supplies. On the C64 the undo image lives outside the overlay slot. No original producer creates these states, or they are paths that never end without an interrupt:
- Z or LOC is not one of the files' instruction or order-pair words, as when a CCS takes a slot that holds data;
- an INDEX whose sum leaves the erasable words the instruction names (POSITON, PIPINDEX, RESULTCT, L in ZEROING), or NDXCTR's computed TC beyond bank 37;
- a TC Q or erasable TC whose word is not a TC to fixed memory, or is a TC into FINETIME, SWCALL or the interpreter;
- CHECKG's wait with NEWJOB not positive (it spins until an interrupt moves PIPAX);
- FINETIME with LOSCALAR 37777 (it spins until the scaler moves);
- NEWJOB negative at a DANZIG;
- an RVQ whose QPRET is not a fixed CADR, a fixed operand outside the constants the code names, or SIN/COS outside `src/sband_trig.c`'s domain;
- an interpretive order with FIXLOC not one of the five VAC areas, PUSHLOC outside FIXLOC to FIXLOC+42, EDOP not +0, or FB not BANKSET's;
- no undo image, or more than 4,000 steps.

## Source map

`tools/lm_imu_performance_source.py` reads the fresh host assembly listing of the unmodified Luminary099 and its rope. It checks:
- that every interpretive case carries the files' words as its octal literals, in order, from its LOC;
- that every basic label is an instruction word of the files;
- that every emitted word of the files is owned by a basic label, an interpretive case, or an operand or constant (CADR, 2CADR, ADRES, GENADR, ECADR, DEC, 2DEC, OCT, VN, MM);
- the 23 `K_` constants, the 19 `X_` addresses and the three operand tables against the rope and symbol table.

Result: `evidence/lm-imu-performance/lm-imu-performance-source-map.json`.

## Evidence

Hashes: `evidence/lm-imu-performance/SHA256.json`.

- **Independent assembly comparison.** `make lm-imu-performance-differential` runs the unmodified rope in yaAGC (host-only, `tools/lm_imu_performance_reference.c`). The oracle is the pinned engine without its gyro, IMU CDU drive and optics block (`tools/prepare_no_imu_device_engine.py`), with the scalers frozen. Each of the 54 routines runs 512 states, 27,648 cases. Every case the native code runs (26,689) equals the original in all 2,048 erasable words and the interrupt-enable state, from the same image, Z, scalers and interrupt state to the same stop (`ipt_leaves()`). The 959 refused cases are the domain limits above, and the native image is unchanged for them. Per-case digests are frozen in `tests/lm_imu_performance_vectors.h`.

  States are drawn inside each routine's domain. They cover every CCS class of NDXCTR, LENGTHOT, TORQNDX, ALTIM and OVFLOWCK, small POSITON, PIPINDEX and RESULTCT, genuine return words in QPLACE, QPLACES and ZERONDX and in the CALL return QPRET, the ALX1S and CMPX1 values the ALKCG loop produces, NEWJOB +0 and positive, the five VAC areas, every superbank, and other EBs.

  Results: `evidence/lm-imu-performance/lm-imu-performance-proof.json` (stops reached per routine, refusals per routine).
- **Host and sanitizer results.** `make test-lm-imu-performance sanitize-lm-imu-performance` (`evidence/lm-imu-performance/lm-imu-performance-host.log`) checks:
  - the digests and that each case either stops or is refused;
  - that each routine runs in at least half of its states;
  - that both tables are sorted;
  - the three genuine CALL returns the cases use, through `lmi_call`;
  - each refusal above with image and io unchanged (55,414 checks).
  
  It runs once as built and once with the C64 parts' jump dispatch (`IPT_C64_PART=0`).
- **C64 execution results.** `make vice-lm-imu-performance` (`evidence/lm-imu-performance/lm-imu-performance-vice.log`) runs five diagnostics. The interpretive orders and both files' code do not fit beside two images in C64 RAM, so each part (`IPT_C64_PART`, `tests/lm_imu_performance_cases.h`) links only some interpretive words and basic blocks. A jump to a block a part leaves out goes back through the dispatch with Z set, which is the same next instruction. The freeze script builds every part for the host too and checks that each case a part runs gives the native row exactly. Every 37th case (748) goes to the first part that runs it, and 685 of them run on the C64. The other 63 reach the ALFLT/NORMLOP loop (37,2605-2772, with its vector, matrix and trig orders), which needs about 60 KB alone, or they need code from more than one part (EARTHR with ESTIMS or GEOSTRT4). They are checked on the host only. The proof lists them per routine.
