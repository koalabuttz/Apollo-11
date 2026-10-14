# Run the CM RCS DAP executive job as native routines

Lane A. Milestone `cm-rcs-dap-exec`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

This milestone covers Comanche055 RCS-CSM_DAP_EXECUTIVE_PROGRAMS.agc, the whole file: AMBGUPDT and the QUADANGL constant (22,3447-3510). The RCS DAP starts AMBGUPDT as a job (its 2CADR at 21,2356 and 21,2614).

AMBGUPDT does the following:
1. It leaves (TCF ENDOFJOB) unless FLAGWRD6 selects the RCS autopilot (bit 15 clear, bit 14 set), so it does not disturb the TVC DAP on a switchover.
2. It computes the gimbal-to-jet-axis matrix terms AMGB1 (sin CDUZ), CAPSI (cos CDUZ), AMGB8, AMGB4, AMGB5 and AMGB7 from CDUX less the 7.25-degree jet quad offset.
3. It ends the job.

`tools/interp_compile.py cm-rcs-dap-exec` (spec in `tools/interp_specs.py`, `'basic': 'full'`) compiles the file into `cm_rcs_dap_exec_basic` (`src/cm_rcs_dap_exec.c`), with five routines:
- the AMBGUPDT entry;
- the four returns after its TC SPSIN2, SPCOS2, SPCOS1 and SPSIN1.

The sine and cosine routines (SPSIN and SPCOS in another file) and ENDOFJOB are transfers out of the file. The run stops there, at Z.

The spec carries `index_carry16` and `image_w` (#260).

## Evidence

Hashes: `evidence/cm-rcs-dap-exec/SHA256.json`.
- **Source map.** `evidence/cm-rcs-dap-exec/cm-rcs-dap-exec-source-map.json` lists the file's 33 words: 32 instructions and the QUADANGL constant, each with its native routine.
- **Executed-instruction closure.** `make cm-rcs-dap-exec-closure` reruns the differential with the oracle tracing the words it executes (`tools/oracle_trace`). `tools/interp_executed_closure.py` then checks that every instruction of the file ran in the original program. Result: 32 of 32 (`evidence/cm-rcs-dap-exec/cm-rcs-dap-exec-closure.json`).
- **Independent assembly comparison.** `make cm-rcs-dap-exec-differential` runs the unmodified Comanche055 rope in yaAGC (host-only, `tools/interp_routine_differential.py`): 512 cases at each of the five routines, 2,560 in all, and every one equals the native run (`evidence/cm-rcs-dap-exec/cm-rcs-dap-exec-proof.json`).
  - The comparison covers Z, all 2,048 erasable words and the interrupt-enable state.
  - FLAGWRD6 is mostly the RCS autopilot, with TVC, both, neither and ±0 for the two gates.
  - CDUX and CDUZ take 0, ±1 LSB, the 7.25-degree offset and its neighbors, and 90, 180 and 270 degrees.
  - AMBGUPDT runs to SPSIN 284 times and to ENDOFJOB 228 times.
- **Host and sanitizer results.** `make test-cm-rcs-dap-exec sanitize-cm-rcs-dap-exec`: the 160 frozen cases (every 16th), 0 failures.
- **C64 execution results.** `make vice-cm-rcs-dap-exec`: 13 cases that the differential picks by edge coverage. They take every branch outcome the host cases take (157 gcov branches, and 102 of 102 trace edges). 13 checks, 0 failures.

The older hand translation (`src/cm_rcs_matrix.c`, `docs/COMANCHE-RCS-MATRIX.md`) stays for its existing callers.
