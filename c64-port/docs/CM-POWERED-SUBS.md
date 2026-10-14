# Run the CM powered-flight subroutines as native routines

Lane A. Milestone `cm-powered-subs`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

This milestone covers Comanche055 POWERED_FLIGHT_SUBROUTINES.agc, the whole file (23,3477-3771), basic and interpretive:
- **CDUTRIG, CD\*TR\*G, CDUTRIGS, CD\*TR\*GS.** Copy the CDU angles to CDUSPOT (CDUTRIG and CDUTRIGS only), then store their sines and cosines at SINCDU and COSCDU through CDULOGIC, COSINE and SINE (by USPRCADR). CDUTRIG and CD\*TR\*G are the interpretive forms.
- **QUICTRIG.** The fast form for the guidance cycle: SPSIN and SPCOS of the angles at CDUSPOT, with interrupts inhibited.
- **NBSM.** Loads the angles from the address in S1 and the vector at 32D, then transforms the vector from navigation base to stable member.
- **CDU\*SMNB, TRG\*SMNB, CDU\*NBSM, TRG\*NBSM, \*SMNB\*, \*NBSM\*.** The interpretive interface routines to AX\*SR\*T.
- **AX\*SR\*T.** Rotates the vector in VBUF through the three CDU angles, one axis at a time, with DMPSUB (NB to SM with +3 in A, SM to NB with -3).
- **SMCDURES.** Converts stable-member angle increments (DTHETASM) to CDU increments (DCDU).

`tools/interp_compile.py cm-powered-subs` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/cm_powered_subs.c`, 47 routines:
- `cm_powered_subs_interpretive`: each interpretive entry at NEWOPS, the returns after its TC INTPRET and CALL, and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting;
- `cm_powered_subs_basic`: QUICTRIG, the words after each EXIT, and the returns after USPRCADR, SPSIN, SPCOS, MPACVBUF and DMPSUB.

USPRCADR, SPSIN, SPCOS, MPACVBUF and DMPSUB are transfers out of the file, and so are the callers' returns (TC TEM2, TC A, TC RTNSAVER to a fixed address outside it). The run stops there, at Z.

AX\*SR\*T reads its vector components with `INDEX BUF`, `INDEX DEX1`, `DCA VBUF`: the second INDEX is itself indexed. `tools/basic_compile.py` now compiles an INDEX of INDEX (the inner INDEX reads the erasable word at its address plus the outer index). No other spec's output changes (`tools/interp_compile.py --check`).

The spec carries `index_carry16` and `image_w` (#260).

## The states each resume point starts from

A resume point runs from the middle of a loop, so its cases start from the words the file's own runs leave there:
- CD\*TR\*GS after CDULOGIC, COSINE and SINE (23,3530, 3536, 3546): TEM3 an angle offset (4, 2 or 0); TEM2 a caller's return (the TCs in CDUTRIG, CD\*TR\*G and the four interface routines, or a fixed address outside the file).
- QUICTRIG after SPSIN and SPCOS (23,3566, 3575): ITEMP2 the offset; ITEMP1 a return outside the file.
- AX\*SR\*T after DMPSUB (23,3676, 3714): DEXDEX ±1 to ±3; DEX1 and DEX2 offsets; BUF 0 or 1; RTNSAVER the TC in C\*MM\*N2, or outside.
- NBSM and its LXC,1 resume key: S1 the address of three angles (MARKDATA +2 in P20-P25, a mark VAC in P51-P53, or CDUSPOT itself).
- AX\*SR\*T's `INDEX A`, `DCA INDEXI` reads the INDEXI table (A 0 to 2) from the rope (`fixed_tables`).

## Evidence

Hashes: `evidence/cm-powered-subs/SHA256.json`.
- **Source map.** `evidence/cm-powered-subs/cm-powered-subs-source-map.json` lists the file's 187 words with their native routines: 131 basic instructions, 47 interpretive words, and 8 data words (SINESLOC, INDEXI and the CADRs after USPRCADR).
- **Executed-instruction closure.** `make cm-powered-subs-closure` reruns the differential with the oracle tracing the words it executes, and the fixed words it reads (`tools/oracle_trace`, `AGC_READ_WORDS`). The interpreter reads interpretive code as data (`INDEX LOC`, `CA 0`), so `tools/interp_executed_closure.py` counts an interpretive word when the oracle read it. Result (`evidence/cm-powered-subs/cm-powered-subs-closure.json`):
  - 131 of the 132 basic instructions executed and all 47 interpretive words read;
  - the one left, 23,3732 (TSTPOINT's TC RTNSAVER for +0), cannot be reached. AX\*SR\*T enters with ±3, and DIM of +1 gives -0 in the ones' complement adder, so `CCS DEXDEX` never sees +0. 96 cases at 23,3714 reach TSTPOINT, the ones with DEXDEX +1 among them, and all take the -0 exit, in yaAGC and natively.
- **Independent assembly comparison.** `make cm-powered-subs-differential` runs the unmodified Comanche055 rope in yaAGC (host-only, `tools/interp_routine_differential.py`): 512 cases at each of the 47 routines, 24,064 in all, and every one equals the native run (`evidence/cm-powered-subs/cm-powered-subs-proof.json`).
  - The comparison covers Z, all 2,048 erasable words and the interrupt-enable state.
  - Every routine stops at a transfer out of the file: USPRCADR, SPSIN, SPCOS, MPACVBUF, DMPSUB, the interpreter (INTPRET, NEWOPS), the Executive's CHANG2 (05170) with a job waiting, or a caller's return outside.
- **Host and sanitizer results.** `make test-cm-powered-subs sanitize-cm-powered-subs`: the 1,510 frozen cases, 0 failures.
- **C64 execution results.** `make vice-cm-powered-subs`: 74 cases in two programs (50 and 24) that the differential picks by edge coverage. They take every branch outcome the host cases take (616 gcov branches, and 594 of 594 trace edges). 74 checks, 0 failures.

The older hand translations (`src/cm_quictrig.c`, `src/cm_powered_trig.c`, `src/cm_nbsm.c` and their docs) stay for their existing callers.
