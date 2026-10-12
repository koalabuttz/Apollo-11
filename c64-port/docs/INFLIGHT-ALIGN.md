# Translate Luminary and Comanche INFLIGHT_ALIGNMENT_ROUTINES

Lane D1b (alignment and maneuvers). Milestone `inflight-align`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

`Luminary099/INFLIGHT_ALIGNMENT_ROUTINES.agc` and `Comanche055/INFLIGHT_ALIGNMENT_ROUTINES.agc` are native C (`src/lm_inflight_align.c`, `src/cm_inflight_align.c`). Each entry, and each point where the file resumes after an outside call, is a routine of the control-transfer ABI keyed by its original address in bank 23. A routine runs the file from there to the point where control leaves it, and leaves the image as the original does.

## How the routines are made

`tools/interp_compile.py inflight-align` generates both files from the yaYUL listings (`tools/interp_listing.py`) and the spec in `tools/interp_specs.py`. It decodes the file from its entries and gives every branch target, STCALL/CALL return, EXIT and TC INTPRET point its own routine. Each order pair becomes the calls the interpreter itself makes for it, in order, with the pair's literal order and operand words:

- NEWOPS (`lmi_newops`, `lmi_store_code`), the address and misc fetches (`lmi_direct`, `lmi_indexed`, `lmi_address_pushup`, `lmi_misc`), then the order body from `src/lm_interp_orders.c` (Comanche: the `cmi_` build of the same bodies, `src/cm_interp_orders.c`), then DANZIG.
- A fixed operand is a `K` array holding the six rope words at its address, so nothing reads the rope at run time.
- `RTB V1STO2S` (CALCGA) is the native `rtb_angles` routine.
- Each routine first checks the image is one the original could be in: NEWOPS with LOC at the routine, BANKSET equal to BBANK, FB its bank, NEWJOB +0, FIXLOC a VAC area and the latches in range. Anything else is `EXEC_OUT_OF_SCOPE` with the image unchanged.
- `--check` fails when the generated files differ from what the listings give.

`<SYM>_KEEP0` masks let a build keep only some routines; the C64 diagnostics use them.

## Scope

Routines (`include/lm_inflight_align.h`, `include/cm_inflight_align.h`):

| Luminary099 | Comanche055 | Source | Leaves at |
|---|---|---|---|
| 23,3247 | 23,3205 | CALCGTA: gyro torque angles from XDC, YDC, ZDC | STCALL OGC, S2: the return ITA saved; the interpreter alarm (LM ALARM1, CM ALARM) when UNIT overflows |
| 23,3267, 3304, 3316 | 23,3225, 3242, 3254 | after CALCGTA's STCALLs to ARCTRIG | the same |
| 23,3320 | 23,3256 | ARCTRIG: angle from sine and cosine | the caller |
| 23,3333, 3344 | 23,3271, 3302 | TRIG1, TRIG2 | the caller |
| 23,3353 | 23,3311 | CALCGA: middle gimbal angle from the stable member and body axes | CALCGA1, or GIMLOCK1 past the lock limit |
| 23,3372, 3406, 3423 | 23,3330, 3344, 3361 | after CALCGA's STCALLs | the same |
| 23,3413 | 23,3351 | CALCGA1: inner and outer gimbal angles, RTB V1STO2S (Comanche also BONCLR CPHIFLAG) | STCALL THETAD, S2: the CADR the caller left in S2 |
| 23,3431 | 23,3371 | GIMLOCK1 | EXIT, TC ALARM (OCT 00401) |
| 23,3432, 3434, 3436 | 23,3372, 3374, 3376 | after EXIT, TC ALARM and TC UPFLAG (ADRES GLOKFAIL) | ALARM, UPFLAG, TC INTPRET |
| 23,3437 | 23,3377 | after TC INTPRET | GOTO CALCGA1 |
| 23,3441 | 23,3401 | AXISGEN: REFSMMAT-style axes from two star pairs | the caller |
| 23,3447, 3476, 3521 | 23,3407, 3436, 3461 | AXISGEN1-3 (TIX loops over the axes) | the caller |

- The two programs differ only in CALCGA1's BONCLR CPHIFLAG (Comanche) and in addresses.
- Destinations outside the file stop the trampoline with `EXEC_UNMAPPED` until their files have routines: the caller's return CADR, ALARM, UPFLAG, and the interpreter's POODOO/ALARM exits from SQRT, UNIT and ARCCOS.

## Source map

`tools/interp_routine_differential.py --freeze` writes `evidence/inflight-align/inflight-align-source-map.json`: each emitted word of both files with the routine that compiles it, or the fixed operand (`K` array) it is part of. It fails when a word has neither. Luminary has 184 words: 180 compiled and 4 operand words. Comanche has 186 words: 182 compiled and 4 operand words.

## Evidence

Hashes: `evidence/inflight-align/SHA256.json`. Bulk files are assets of the `evidence-inflight-align` release (`python3 tools/evidence.py fetch inflight-align`).

- **Independent assembly comparison.** `make inflight-align-differential` runs the unmodified ropes in yaAGC (host only, `tools/interp_routine_reference.c`). It uses the pinned engine with DV always on its hardware model (`tools/prepare_hardware_dv.py`), as the native divide does. 512 states per routine and program (21,504 cases) run from the same image to the same stop. Each equals the native run (`tools/interp_routine_native.c`) in Z, all 2,048 erasable words and the interrupt-enable state.
  - The states use three data shapes: random words, small magnitudes and unit-vector scale. They include random FIXLOC VAC areas, PUSHLOC, MODE and the CADRs the GOTOs and returns read. Caller returns are the genuine CALL/STCALL return words elsewhere in each program, below bank 30.
  - The stops reached per routine are in `inflight-align-proof.json`. They include the caller's return, LM ALARM and UPFLAG, and the SQRT/UNIT alarm exits.
  - Every 32nd case is frozen in `tests/inflight_align_vectors.h`.
- **Host and sanitizer results.** `make test-inflight-align sanitize-inflight-align` runs the 672 frozen cases to their digests (`inflight-align-host.log`).
- **C64 execution results.** `make vice-inflight-align` (`inflight-align-vice.log`) runs eight diagnostics, four per program, each holding a group of routines (`tools/mk/inflight-align.groups`) that fits below $c000 with the shared interpreter bodies. Each runs the two smallest cases of each of its routines whose whole run stays in the group, 84 in all.
