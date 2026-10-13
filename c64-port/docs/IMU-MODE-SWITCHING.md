# Run Luminary and Comanche IMU mode switching as native routines

Lane D1. Milestone `imu-mode-switching`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

`Luminary099/IMU_MODE_SWITCHING_ROUTINES.agc` and `Comanche055/IMU_MODE_SWITCHING_ROUTINES.agc` are compiled native C (`src/imu_mode_switching.c`). Each of the file's entries, and each point where it resumes after an outside call, is a routine of the control-transfer ABI at its original address. It runs the source from there to the point where control leaves the file. It leaves Z, Q, A, L, the banks, the interrupt-enable state and channels 11-14 as the original does.

The earlier IMU mode files (`src/imu_pulse*.c`, `src/imu_coarse.c`, `src/imu_mode_*.c`, `src/imu_standby.c`) model pieces of the same source behind their own contracts. They stay in the tree for the code that uses them. This milestone is the whole file on the shared ABI.

## Scope

The file spans four banks. Comanche addresses follow from the shared layout (`ims_comanche_address()`).

| Part | Luminary099 | Comanche055 | Routines per mission |
|---|---|---|---|
| ZEROICDU | fixed-fixed 5457 | fixed-fixed 5521 | 1 |
| Mode switching, coarse and fine align, PIPA use, gyro torquing, mode ends, stalls, GOMANUR | 07,2714-3772 | 07,2577-3651 | 48 |
| R02BOTH, R02ZERO | 04,3253-3273 | 07,3652-3672 | 4 |
| P06/P05 (standby) | 37,3655-3773 | 26,3660-3776 | 20 |

R02ZERO (TC UPFLAG, ADRES IMUSE, TCF SWRETURN) is a routine of its own: R02BOTH reaches it with TC, and P51-P53 with `TC BANKCALL` (CADR R02ZERO).

Each routine ends where control leaves the file:
- a TC or TCF to another file: WAITLIST, VARDELAY, FIXDELAY, TASKOVER, ALARM, POODOO, BAILOUT1 (Luminary MODABORT), IBNKCALL, BANKCALL, SWRETURN, JOBSLEEP, JOBWAKE, MAKECADR, POSTJUMP, TPAGREE, SHORTMP, FINETIME +1, UPFLAG, DOWNFLAG, PHASCHNG, VARALARM or GOTOPOOH;
- the caller's return of a subroutine entry. `TC Q` executes the TC that Q holds and leaves Q at 3;
- `TC MPAC +2` (SCALPREP's return, Q 0157);
- an erasable TC that STRTGYR2's computed jump reaches when LGYRO's step count is odd.

Where the original continues after such a call, the continuation is a routine of its own. Local subroutines (CAGETEST, CAGETSTQ, CAGETSTJ, NOATTOFF, ZEROICDU, SETCOARS, RNDREFDR, GSELECT, SCALPREP) are compiled into their callers with the original Q effects.

- Each instruction is one `agc_image_ops` register effect, in source order. INDEX is an explicit address offset.
- INHINT and RELINT set the mission io's `interrupt_enabled`.
- RAND, WAND and WOR use channels 12, 13 and 14 in the io: `lm_imu_io` for Luminary (its first member is the `p66_bias_io` other Luminary routines use), `cm_native_io` for Comanche. Channel ops follow the hardware: `A = (A op channel) & 077777`, sign-extended. A channel-13 write keeps bits 12-14 clear.
- READ DSALMOUT (ENDIMU) reads channel 11 from the program's `agc_hardware`. Routines that can reach ENDIMU (2777, the FIXDELAY continuation of COARS2, IMUFINED, NORESET, ENDIMU) refuse when the program has none.
- The word labelled `-GYROMIN` (OCT 77601) sits in the instruction stream and executes as `MASK 7601`. It reads fixed-fixed 07601: 030001 in Luminary, 052131 in Comanche. The native code uses the mission's word.
- The code differences between the missions are in the mission record and the address map: Luminary GOODEND has an extra `EXTEND; BZF ENDMODE +1`; Luminary MODABORT is `DXCH BUF2; TC BAILOUT1` where Comanche has `TC POODOO`.

Refused input (`EXEC_OUT_OF_SCOPE`, image, io and channels unchanged). No original producer creates these states. Each is a place where an index or computed jump would leave the words the source addresses:
- subroutine entries (ZEROICDU, SETCOARS, RNDREFDR, NOATTOFF) with Q outside 010-07777, where `TC Q` would not execute a TC;
- BADEND and GOODEND with A other than 0, 1 or 2 (the MODECADR index);
- the MAKECADR continuation with RUPTREG2 above 2;
- the JOBWAKE continuation with LOCCTR above 01613 (`INDEX LOCCTR ADS LOC`);
- the WAITLIST continuation of IMUPULSE with the low eight bits of MPAC +5 above 0372;
- the TPAGREE continuation with MPAC +4 above 0376, or a further GYROAGRE pass indexing past 01776;
- STRTGYRO with LGYRO negative or at 040000 and above, or a GSELECT step whose LOW8 index reaches 0377;
- any GYROAGRE or GSELECT access under EBANK 0 at index 0-7, where 1400-1407 are the central registers (A, L, Q, the banks, Z, ZERO);
- SCALPREP's FINETIME continuation with MPAC +2 outside 010-07776;
- the channel-13 writers (the PHASCHNG lead-in of P06, and POSTAND) with channel 13 bits 12-14 set. Hardware never holds the trap enables there.

## Source map

`tools/imu_mode_switching_source.py` reads fresh host assembly listings of both unmodified programs. It assigns every emitted word of the file to the native function that compiles it, by source address, and fails on an unowned word or an owner that covers none. It also checks each fixed constant the native code uses as a literal against the rope. These are 34 `K_` constants, GYROFRAC, the `MASK 7601` operand and KALEBCON.

| Mission | Words | Native ranges | Literals |
|---|---|---|---|
| Luminary099 | 660 | 102 | 37 |
| Comanche055 | 656 | 102 | 37 |

The CADRs after `TC BANKCALL`/`IBNKCALL` and the 2CADRs after `TC WAITLIST` are the callers' operands. Nothing reads the rope at run time.

Result: `evidence/imu-mode-switching/imu-mode-switching-source-map.json`.

## Evidence

Hashes: `evidence/imu-mode-switching/SHA256.json`. Bulk files are assets of the `evidence-imu-mode-switching` release (`python3 tools/evidence.py fetch imu-mode-switching`).

- **Independent assembly comparison.** `make imu-mode-switching-differential` runs the unmodified ropes in yaAGC (host-only, `tools/imu_mode_switching_reference.c`). Every case runs from the same image, Z, channels and interrupt state to the same stop. It equals the native run in all 2,048 erasable words, the interrupt-enable state and channels 11-14. Per-case digests are frozen in `tests/imu_mode_switching_vectors.h`.

  The oracle uses the pinned yaAGC engine without its gyro, IMU CDU drive and optics peripheral block (`tools/prepare_no_imu_device_engine.py`, `build/no-imu-device.patch`). That block drains GYROCTR and bursts the CDU commands into erasable memory between instructions while the channel-14 drive bits are set. Those are device effects, which the port keeps in its device layer. The script checks the pinned engine's hash and the removed block's hash. No instruction, counter or interrupt code changes.

  | Cohort | Cases | Stops reached |
  |---|---|---|
  | Entry runs, 146 routines x 384 states | 56,064 | every external above, the subroutine return, the STRTGYR2 erasable targets 0100, 0202 and 0302 |
  | Round trips: every genuine `TC BANKCALL`/`TC IBNKCALL` site whose CADR is one of these routines (Luminary 74, Comanche 54) x 64 | 8,192 | caller continuation 2,182; BAILOUT1, POODOO, ALARM, VARALARM, UPFLAG, WAITLIST, JOBSLEEP, JOBWAKE 5,917; SETISSW 93 |

  States are drawn inside each routine's domain. They cover every CCS class of LGYRO, MODECADR and MPAC, odd and even STRTGYRO steps, small gyro and coarse-align commands, gyro pairs at GSELECT's skip (major ±0, minor ±0 or under GYROMIN pulses, at 0177 and 0200 too) for every step STRTGYRO reads, in a quarter of its states, CDUs within a few bits of THETAD, SCALPREP's genuine retry returns, and every EB and superbank.

  **SWRETURN after `TC Q`.** The original reaches SWRETURN and ISWRETRN with Q 3 after `TC Q`, for example CAGETSTQ's `TCF SWRETURN`, and the IBNKCALL returns of SETCOARS and RNDREFDR. INTER-BANK_COMMUNICATION's native SWRETURN and ISWRETRN do not read Q and accept it, so those 928 round trips run on to the caller continuation in both programs.

  Results: `evidence/imu-mode-switching/imu-mode-switching-proof.json`.
- **Host and sanitizer results.** `make test-imu-mode-switching sanitize-imu-mode-switching` (`evidence/imu-mode-switching/imu-mode-switching-host.log`) checks:
  - the digests;
  - that every case stops;
  - that both tables are sorted with 73 routines;
  - the Comanche address map;
  - each refusal above in both missions, with image, io and channels unchanged (128,990 checks).
- **C64 execution results.** `make vice-imu-mode-switching` (`evidence/imu-mode-switching/imu-mode-switching-vice.log`, with each PRG's sha256) runs nine diagnostics. Each third of the routines (`IMS_C64_PART`) gets its own program, because all of them do not fit beside the test image, with its entry cases, Luminary round trips and Comanche round trips. The rows are chosen by outcome, per mission (`tools/imu_mode_switching_cover.py`): the fewest cases that take every edge the host stream takes in every linked native source (`src/imu_mode_switching.c`, the INTER-BANK routines, the image ops, the transfer trampoline and the word, multiword and divide helpers; by gcc trace-pc and clang edge coverage, since each misses some), every routine, every call site and every stop kind. That is 683 entry cases, 350 Luminary and 255 Comanche round trips: 1,288 rows. The script checks them under gcov at -O0 over the same sources. They take every branch outcome the host stream takes in each mission (Luminary 473 of 1,033, Comanche 477 of 1,033; the rest are refusal guards, the other mission's paths and helper paths these routines never use). Result: `evidence/imu-mode-switching/imu-mode-switching-c64-cover.json`.
