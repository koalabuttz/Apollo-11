# Run the LM powered-flight subroutines as native routines

Lane A. Milestone `lm-powered-subs`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. The Comanche055 form is `docs/CM-POWERED-SUBS.md`.

This milestone covers Luminary099 POWERED_FLIGHT_SUBROUTINES.agc, the whole file, basic and interpretive, in two banks:
- **Bank 23 (23,3537-3766).** CDUTRIG, CD\*TR\*G, CDUTRIGS and CD\*TR\*GS (sines and cosines of the CDU angles through CDULOGIC, COSINE and SINE), QUICTRIG (SPSIN and SPCOS), the interface routines CDU\*SMNB, TRG\*SMNB, CDU\*NBSM, TRG\*NBSM, \*SMNB\* and \*NBSM\*, and AX\*SR\*T (the axis rotation with DMPSUB). Luminary has no NBSM.
- **Bank 10 (10,2030-2212).** CALCSMSC and FLESHPOT. FLESHPOT builds the stable-member-to-navigation-base matrix at the ECADR in A (XNB from CALCSMSC, XNBPIP from SERVICER), under that EBANK, with DMP and DMPSUB. Its middle row calls the interpreter's VXV from basic code: `DCA Z`, `AD FOUR`, `DXCH LOC` (LOC at the `TCF VXV` word, 10,2173), EDOP BIT8, `TCF VXV`. VXV ends at DANZIG, whose CCS EDOP runs EXIT into the basic word after the TCF (10,2174).

`tools/interp_compile.py lm-powered-subs` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/lm_powered_subs.c`, 49 routines:
- `lm_powered_subs_interpretive`: each interpretive entry at NEWOPS, and the returns after its TC INTPRET. No order pair here can end in DANZIG with a job waiting, so the file has no CHANG2 resume keys. CD*TR*GS's CADR CDULOGIC, COSINE and SINE +1 words (23,3567, 3575, 3605) are routines too, keyed at DANZIG (`usprcadr_exits`): USPRCADR leaves LOC there and BIT8 in EDOP, and the interpreter returns through DANZIG, whose CCS EDOP runs EXIT into the next word. FLESHPOT's TCF VXV word (10,2173) is one the same way (`edop_exits`): VXV's DANZIG finds LOC there with EDOP 1 and runs EXIT into 10,2174 (D1's host sweep on 84d550f5 met it in LM P63). These four take any INTBIT15 (`at_exit`): FLESHPOT's DXCH LOC skips INTPRET +2, so INTBIT15 is the caller's, 040000 in P63 after a bank-23 caller. The program puts the DANZIG marker first among its interpretive tables (`tests/lm_powered_subs_target.c`).
- `lm_powered_subs_basic`: QUICTRIG, AX\*SR\*T, FLESHPOT and FLESHPOT -1 (CALCSMSC's BANKCALL), the words after each EXIT, and the returns after BANKCALL, USPRCADR, SPSIN, SPCOS, MPACVBUF, DMP, DMPSUB and VXV (10,2174).

The transfers out of the file are BANKCALL, USPRCADR, SPSIN, SPCOS, MPACVBUF, DMP, DMPSUB, VXV and SWRETURN, plus the callers' returns. The run stops there, at Z.

Two additions to `tools/basic_compile.py`, as this file needs them. Neither changes any other spec's output (`tools/interp_compile.py --check`):
- **A read of Z.** FLESHPOT's `DCA Z` now sees Z as the hardware has it: the address of the next word (10,2167).
- **`fx()`'s bank.** The fixed-table lookup reads INDEXI in bank 23, the tables' own bank, not the bank of the file's lowest entry (10).

The spec carries `index_carry16` and `image_w` (#260), and `edge_values` (MPAC, the push-down list and the operands as edge vectors in a quarter of the cases).

## The states each resume point starts from

As in the CM form, each mid-loop point starts from the words the file's own runs leave there:
- CD\*TR\*GS (23,3570, 3576, 3606): TEM3 an offset 4, 2 or 0, and TEM2 a caller's return.
- QUICTRIG (23,3626, 3635): ITEMP2 the offset, ITEMP1 a return outside, and A across its range and +-0 in about a fifth of the cases, so MP BIT14 runs with a +-0 multiplicand (reviewer 1, #316).
- AX\*SR\*T (23,3723, 3741): DEXDEX ±1 to ±3, DEX1 and DEX2 offsets, BUF 0 or 1, and RTNSAVER a return.
- AX\*SR\*T itself, which other files call: A +3 or -3, and Q a return.
- FLESHPOT after DMP, DMPSUB and VXV: TEM1 the switched address of XNB (1664) or XNBPIP (1545), and TEM2 the caller's EBANK.

## Evidence

Hashes: `evidence/lm-powered-subs/SHA256.json`.
- **Source map.** `evidence/lm-powered-subs/lm-powered-subs-source-map.json` lists the file's 267 words with their native routines: 238 basic instructions, 14 interpretive words, and 15 data words (SINSLOC, INDEXI, XNBECADR, the CADRs after USPRCADR and BANKCALL, and the ADRES words after DMP).
- **Executed-instruction closure.** `make lm-powered-subs-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). Result (`evidence/lm-powered-subs/lm-powered-subs-closure.json`):
  - 237 of the 238 basic instructions executed, and all 14 interpretive words read by the interpreter;
  - the one left, 23,3757 (TSTPOINT's TC RTNSAVER for +0), cannot be reached. AX\*SR\*T enters with ±3, and DIM of +1 gives -0, so `CCS DEXDEX` never sees +0.
- **Independent assembly comparison.** `make lm-powered-subs-differential`: the unmodified Luminary099 rope in yaAGC, 512 cases at each of the 49 routines, 25,088 in all, and every one equals the native run (`evidence/lm-powered-subs/lm-powered-subs-proof.json`). It compares Z, all 2,048 erasable words and the interrupt-enable state.
- **Host and sanitizer results.** `make test-lm-powered-subs sanitize-lm-powered-subs`: the 1,569 frozen cases, 0 failures.
- **C64 execution results.** `make vice-lm-powered-subs`: 71 cases that the differential picks by edge coverage. They take every branch outcome the host cases take (547 gcov branches, and 873 of 873 trace edges and comparison outcomes). 71 checks, 0 failures.

The older hand translations (`src/lm_powered_*.c`, `src/lm_fleshpot*.c`, `src/guidance_rotation.c`, `src/guidance_throttle_service.c`) stay for their existing callers. This milestone does not change them.
