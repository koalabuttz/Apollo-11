# Run the LM rendezvous display (R31) as native routines

Lane A. Milestone `lm-r31-file`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Lane E's hand translation is `docs/LM-R31.md`.

This milestone covers Luminary099 R31.agc, the whole file, basic and interpretive, in two banks:
- **R31CALL and DSPDELAY (40,3674-3710, basic).** R31CALL starts the V83CALL job (FINDVAC). DSPDELAY is a waitlist task: every second (FIXDELAY 100) it checks bit 12 of EXTVBACT, and once REV83 has set it after its first pass, it starts the DISPN5X job (NOVAC) and ends (TASKOVER).
- **DISPN5X (37, basic).** Shows V16N54 (range, range rate, theta) through GOMARKF, ends through B5OFF on terminate or proceed, and shows again on recycle.
- **V83CALL (37, basic and interpretive).** With AVEGFLAG set it goes to MUNG?, and from there to GETRVN, or to ONEBASE for the CSM base vector only. On the surface it goes to ONEBASE. Otherwise it integrates both vehicles to now (LOADTIME, LEMPREC, then DOCMBASE and CSMPREC) and keeps the base vectors and BASETIME.
- **REV83 (37, basic and interpretive).** The same flag tests pick GETRVN (RN, VN and the CSM vectors at PIPTIME, under PRIO22), R31SURF (LEMPREC on the surface, then the CSM conic at OTHCONIC), or conic extrapolation of both vehicles from the base vectors (INTSTALL, INTEGRVS, OTHCONIC, OTHINT). COMPDISP then forms RANGE, RRATE and RTHETA: NORMUNX1 of the relative position, the dot product of the relative velocity, and the angle between the LM +Z axis (CDU*NBSM, REFSMMAT) and the line of sight projected into the orbit plane (VPROJ, ACOS). After the EXIT, it ends at ENDEXT if the crew has answered (EXTVBACT bit 5), or sets bit 12 and runs REV83 again.

`tools/interp_compile.py lm-r31-file` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/lm_r31_file.c`, 82 routines:
- `lm_r31_file_basic` (14): R31CALL, DSPDELAY and its returns after FIXDELAY and NOVAC, DISPN5X and the three returns after GOMARKF, V83CALL, REV83, the word after COMPDISP's EXIT (37,3316), and in GETRVN the returns after each PRIOCHNG (37,3330 and 37,3346) and the word after its EXIT (37,3344). GETRVN, MUNG?, ONEBASE and R31SURF run inside these routines.
- `lm_r31_file_interpretive` (68): the words after each TC INTPRET, the returns after STCALL and CALL, and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting.

The transfers out of the file are FINDVAC, NOVAC, FIXDELAY, TASKOVER, BANKCALL (GOMARKF), B5OFF, PRIOCHNG, ENDEXT, the interpreter (INTPRET 06037, NEWOPS 06070, and the subroutines it calls: LEMPREC, CSMPREC, INTSTALL, INTEGRVS and CDU*NBSM), and the Executive's CHANG2 (05126) with a job waiting. The run stops there, at Z.

The two RTBs run through existing native bodies. LOADTIME is `rtb_angles`. NORMUNX1 is the new `lm_normunx1` in `src/lm_normunit.c`: `rtb_vector`'s NORMUNX1 front (CAF ONE, so the shift count goes to X1) followed by `rtb_unit`'s three UNIT phases, the same body as `lm_normunit` with the other front. `lm_normunit` itself is unchanged.

The spec has one `states` entry. At 37,3243 (the RTB NORMUNX1 resume point), MODE (0163) is the vector mode 77776, which is the mode the VSU before it always leaves. With random MODE, UNIT refuses a scalar operand (`rtb_unit_phase` checks it) where yaAGC would run on with a scalar. The spec also carries `index_carry16`, `image_w` (#260), `superbank_bit` (#321), and `edge_values`.

## Evidence

Hashes: `evidence/lm-r31-file/SHA256.json`.
- **Source map.** `evidence/lm-r31-file/lm-r31-file-source-map.json` lists the file's 204 words with their native routines, none untranslated.
- **Executed-instruction closure.** `make lm-r31-file-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). The oracle executed all 53 basic instructions and the interpreter read all 143 interpretive words, none named unreached (`evidence/lm-r31-file/lm-r31-file-closure.json`). The 8 data words are the two 2CADRs (V83CALL and DISPN5X, two words each), the DEC 100 after FIXDELAY, the CADR GOMARKF, the constant V16N54, and 37,3335. That last word is VN (the LM velocity), the operand of GETRVN's STOVL RONE. The tool counts it as data because VN is also the noun-verb constant's opcode, and the interpreter does read it.
- **Independent assembly comparison.** `make lm-r31-file-differential`: the unmodified Luminary099 rope in yaAGC, 512 cases at each of the 82 routines, 41,984 in all, and every one equals the native run (`evidence/lm-r31-file/lm-r31-file-proof.json`). It compares Z, all 2,048 erasable words and the interrupt-enable state.
- **Host and sanitizer results.** `make test-lm-r31-file sanitize-lm-r31-file`: the 2,637 frozen cases, 0 failures.
- **C64 execution results.** `make vice-lm-r31-file`: 139 cases in 22 programs. The differential picks 138 of them by edge coverage, and one is a truncated row (below). 139 checks, 0 failures. The diagnostics build without inlining, with a static stack and zero-page globals (`c64_flags`, as the .mk), so the runs through NORMUNX1 fit.
- **C64 branch outcomes.** `evidence/lm-r31-file/lm-r31-file-c64-cover.json`: the C64 rows take all 1,291 gcov branch outcomes the traced host cases take, with none missing.

## The run too big for one C64 program

GETRVN's BOFF MUNFLAG pair (37,3347, `src/lm_r31_file.c:1246-1247`) has one outcome only long runs take. With MUNFLAG set and no job waiting, the pair falls through to the next order pair (37,3352) instead of stopping at CHANG2. Every such run then goes on through the rest of GETRVN and all of COMPDISP (15 routines, NORMUNX1's UNIT, VXM, VPROJ and ACOS) up to CALL CDU*NBSM. That is about 6.5 KB more than one C64 program holds. Those runs (380 cases at 37,3346 and 37,3347, listed in the proof's `c64_too_big_cases`) run whole on the host only.

The C64 takes that outcome with a truncated row (`c64_cuts`, group 22). One such case runs to the first NEWOPS with LOC at 37,3352, and the yaAGC oracle runs the same case and stops at the same place. The row's digest is the oracle's full state there. The cover record lists the row under `truncated_rows`, and `--cover-check` rebuilds it. The rest of the run, from 37,3352 through COMPDISP, is covered on the C64 by the cases that start at those routines. The proof's trace-edge count (1,129 of 1,130 on the C64) leaves truncated rows out, so it stays a lower bound.
