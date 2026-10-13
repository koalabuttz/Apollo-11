# Run the Luminary IMU performance tests (P07) as native routines

Lane D1. Milestone `lm-imu-performance`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

`Luminary099/IMU_PERFORMANCE_TEST_2.agc` and `Luminary099/IMU_PERFORMANCE_TESTS_4.agc` are compiled native C (`src/lm_imu_performance.c`). Both files are bank 37 code of P07, the IMU performance test: 585 emitted words at 37,2002-3112. Each entry of the files, and each point where they resume, is a routine of the control-transfer ABI at its original address. There are 43 basic routines keyed by Z and 125 interpretive routines keyed by LOC at NEWOPS 06070. FINETIME (fixed-fixed 04102-04123, which CHECKG calls) is compiled in as a local subroutine.

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

The keys are every point where the files resume:
- the basic entries and the returns from outside calls;
- 37,2027, where `RTB EXIT 1STO2S` (37,2025) resumes: after 1STO2S, DANZIG runs the EXIT still pending in EDOP, which does `INDEX LOC; TC 1`;
- every order-pair and store word (125). Each can follow a DANZIG, and a DANZIG with NEWJOB positive goes to CHANG2. CHANG2 suspends the job with LOC negated, and the Executive resumes it at NEWOPS with LOC +1 (EXECUTIVE: CHANG2, ENDPRCHG, INTRSM). That is the word after the pair, which is the native code's own stop point;
- two hardware events: 02354 (CHECKG after a PIPA pulse moved PIPAX between its two reads) and 04112 (FINETIME after LOSCALAR moved between its READ and the RXOR);
- the two idle points below.

Two waits spin until hardware moves an input. Each ends the run there as a continuation (`EXEC_OK`), and its key resumes it:
- CHECKG1 (37,2335), when the BZF at 2352 finds PIPAX unchanged. The key runs CHECKG1 again: CHANG1 if a job is waiting, else the next read of PIPAX. The continuation is also an interrupt point: the run ends with interrupts inhibited, and the original's RELINT window (2336-2345) has no ABI boundary, so the scheduler takes interrupts at 2335 before resuming;
- FINETIME +1 (04103), when the BZF at 04117 finds LOSCALAR 37777. The key reads the scaler again. SCALPREP (IMU_MODE_SWITCHING_ROUTINES) also enters here with `TC FINETIME +1` and returns to 37,3717.

io is `lm_ipt_io`. INHINT and RELINT set `control.interrupt_enabled`. READ LOSCALAR and READ HISCALAR read the frozen scaler words.

## Refused input

Refused input ends `EXEC_OUT_OF_SCOPE` with the image and io unchanged. A refusal can come after a run has started, so the routine saves the entry image in `io->undo`, which the caller supplies. On the C64 the undo image lives outside the overlay slot. No original producer creates these states, or the compiled interpreter orders do not follow them:
- Z or LOC is not one of the files' instruction or order-pair words, as when a CCS takes a slot that holds data;
- an INDEX whose sum leaves the erasable words the instruction names (POSITON, PIPINDEX, RESULTCT, L in ZEROING), or NDXCTR's computed TC beyond bank 37;
- a TC Q or erasable TC whose word is not a TC to fixed memory, or is a TC into FINETIME, SWCALL or the interpreter;
- NEWJOB negative at a DANZIG;
- an RVQ whose QPRET is not a fixed CADR, a fixed operand outside the constants the code names, or SIN/COS outside `src/sband_trig.c`'s domain. VXSC GEORGEJ (37,3013) with a scalar in MPAC would read a vector at GEORGEJ, past the named constants;
- an indexed operand or store with X1 or X2 outside ±0377. Only a store through another wild index leaves one there; at such an index, `INDEX ADDRWD` changes the instruction;
- AINGOTN's DDV (37,2217) with a divisor whose major part is ±0 once its words agree in sign. That is the original's BUFZERO path, and it is unreachable: AINGOTN divides by the test's elapsed time, about 584 s. (The compiled DDV follows BUFZERO; the certificate review ran 180 such cases equal to yaAGC.);
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

- **Independent assembly comparison.** `make lm-imu-performance-differential` runs the unmodified rope in yaAGC (host-only, `tools/lm_imu_performance_reference.c`). The oracle is the pinned engine without its gyro, IMU CDU drive and optics block (`tools/prepare_no_imu_device_engine.py`), with the scalers frozen. Each of the 168 routines runs 384 states, 64,512 cases. Every case the native code runs (56,580) equals the original in all 2,048 erasable words and the interrupt-enable state, from the same image, Z, scalers and interrupt state to the same stop. The stop is the first instruction outside the files, the interpreter, SWCALL and FINETIME (`ipt_leaves()`), or an idle point (`ipt_idle()`). At an idle point with no job waiting, the oracle runs on until it is back there and requires every word unchanged, so the spin is a fixed point. The 7,932 refused cases are the domain limits above, and the native image is unchanged for them. Most are other EBs (a quarter of the states), random MODE or MPAC at the resume keys (SIN/COS domain, VXSC's operand), and random QPRETs. Per-case digests are frozen in `tests/lm_imu_performance_vectors.h`.

  States are drawn inside each routine's domain, each class on its own draw. They cover:
  - every CCS class of NDXCTR, LENGTHOT, TORQNDX (with LENGTHOT +0 and TORQNDX +0 together), ALTIM and OVFLOWCK;
  - small POSITON, PIPINDEX and RESULTCT;
  - genuine return words in QPLACE, QPLACES and ZERONDX, and in the CALL return QPRET (also OVERFFIX's);
  - ALX1S and CMPX1 as +0 or -(12 to 132) and ±0. The flight values are ALX1S 144, then 132 down to 0 in steps of 12, and CMPX1 ±1; the certificate review's probe with those values takes the same branch outcomes;
  - NEWJOB +0 and positive, PIPAX unchanged at CHECKG1 in half its states, and LOSCALAR 37777 in a quarter of all states;
  - the five VAC areas, every superbank, and other EBs.

  Results: `evidence/lm-imu-performance/lm-imu-performance-proof.json` (stops reached per routine, refusals per routine). The host stream stops 7,997 times at CHANG2, 493 times at CHECKG1 and 277 times at FINETIME +1.
- **Host and sanitizer results.** `make test-lm-imu-performance sanitize-lm-imu-performance` (`evidence/lm-imu-performance/lm-imu-performance-host.log`) checks:
  - the digests and that each case either stops or is refused;
  - that each routine runs in at least a fifth of its states (the resume keys draw MODE, MPAC and the pushdown list at random);
  - that both idle points end runs as continuations;
  - that both tables are sorted;
  - the three genuine CALL returns the cases use, through `lmi_call`;
  - each refusal above with image and io unchanged (129,258 checks).

  It runs once as built and once with the C64 parts' dispatch (`IPT_C64_PART=0`).
- **C64 execution results.** `make vice-lm-imu-performance` (`evidence/lm-imu-performance/lm-imu-performance-vice.log`) runs seven diagnostics, one per part (`IPT_C64_PART`, `tests/lm_imu_performance_cases.h`). The interpretive orders and both files' code do not fit beside two images in C64 RAM, so each part links only some interpretive words and basic blocks. Each part ends below the mailbox page at $C000, which the build checks in the link map; inlining is off for these builds because LTO inlining pushed four parts past it (#145). The ALFLT loop is split at ALKLP (37,2676); no loop crosses that point. A part's run is a chained step: it stops where control reaches a registered key the part leaves out, and that key's cases continue from there in the part that has it.

  The freeze builds every part for the host too and checks every row a part runs (kinds 0 and 2) against the oracle run with the same stops. It also checks that the part-0 build, which takes the part at run time, replays each part exactly.

  The C64 rows are 218 rows that together take every edge the host stream takes and start at every key (`tools/lm_imu_performance_cover.c`, greedy over trace-pc edge sets), plus every 149th case in the first part that runs it, plus 7 rows (`EXTRA_ROWS`) for outcomes in the shared sources the edge cover misses: 657 rows. `tools/lm_imu_performance_cover.py` replays them through the part-0 build under gcov, over every linked native source (the file, the interpreter bodies, the trig, divide and image ops). They take 1,231 of the 2,176 branch outcomes, every one that the host stream and the host's part rows both take. One outcome only the part-0 stream takes is listed apart: the 16-bit word guard in `src/sband_trig.c:9`, which no part reaches because a chained step stops at the key first. Result: `evidence/lm-imu-performance/lm-imu-performance-c64-cover.json`, which also lists the outcomes the host never takes (the C64 part tests and the refusal guards no drawn state reaches).
