# Compiled interpretive bodies for Comanche055

Lane A. Milestone `cm-interp-bodies`.

The compiled interpretive order bodies and dispatch steps now build for Comanche055 as well as Luminary099. These are the 62 paths in `src/lm_interp_orders.c` and `src/lm_interp_steps.c`, first written for Luminary by Lane B and listed in `docs/LM-TIME-FREE-FALL.md` and `docs/LM-CONIC-KEPLER.md`. CM interpretive code can now be ported the same way as LM code, as compiled steps that call these bodies. Callers include the files of the IMU, targeting and CM flight lanes.

## How

The two programs' `INTERPRETER.agc` files contain the same code, apart from two alarm exits (below). Their erasable assignments are identical. Only fixed addresses differ:
- the fixed-fixed interpreter code is 031 lower in Comanche (DANZIG 06061 becomes 06030, NEWOPS 06070 becomes 06037);
- bank 01 is 025 higher;
- the constant pool and SWRETURN are 044 higher;
- bank 00 is unchanged up to SQRTABRT.

So the body sources are shared rather than copied:
- **Addresses.** Each fixed address the bodies use is written `IF_(luminary,comanche)` (`include/interp_mission.h`). These are TC return addresses in Q and continuation addresses in Z.
- **Comanche build.** `src/cm_interp_orders.c` and `src/cm_interp_steps.c` compile the same sources with `INTERP_COMANCHE`. They use `cmi_` names, declared in `include/cm_interp.h`.
- **Generated files.** `include/cm_interp.h` and the rename list `src/cm_interp_names.h` are generated from the Luminary headers by `tools/cm_interp_generate.py`.
- **Address check.** `tools/cm_interp_generate.py --check` checks all 85 `IF_` pairs against both assembly listings. A Comanche address must hold the Luminary instruction under the same label, at the same offset, with the same three instructions before and after it.
- **Program-specific code.** Three places are written per program:
  - SQRTABRT: Luminary does `DXCH LOC, TC POODOO1`; Comanche does `TC POODOO`, with the alarm code at 03403.
  - The `INDEX v; CAF k` table over the fixed-fixed constant pool: Comanche's pool, at 04774, has no OCT33 or OCT35.
  - TPMODE's bank: Luminary 10,3633; Comanche 22,3565. This one appears only in the differential.

The Luminary build is unchanged. Its differential passes as before.

Not covered: POLY's case generator uses Luminary's T(X) table, so POLY is compiled for Comanche but not exercised there. ARCSIN/ARCCOS (ACOSABRT also differs between the programs) and SIN/COS are not in the body library yet.

## Evidence

Hashes: `evidence/cm-interp-bodies/SHA256.json`.

- **Independent assembly comparison.**
  - `make cm-interp-bodies-differential` runs `tools/lmi_body_differential.py --mission cm`. It builds the same case families as the Luminary differential: order and operand words come from Comanche055 interpretive code in its listing, and fixed operands come from its rope. Every Luminary address in the generator becomes the same instruction's Comanche address.
  - Each case runs the unchanged Comanche055 rope in the pinned yaAGC engine from the same image, to the same stop.
  - 3,000 cases per body, 61 bodies (183,000 cases) equal yaAGC in all 2,048 words and the stop address: `evidence/cm-interp-bodies/cm-interp-bodies-proof.json` and the run's log `evidence/cm-interp-bodies/cm-interp-bodies-differential.log`. The Luminary differential still passes.
  - The same run freezes 244 vector cases (4 per body, from a generator of their own) into `tests/cm_interp_vectors.h`, with yaAGC's digests. Each vector case is an xorshift16 fill plus that case's edits, so the C64 can rebuild it.
- **Host and sanitizer results.** `make test-cm-interp-bodies sanitize-cm-interp-bodies` (`evidence/cm-interp-bodies/cm-interp-bodies-host.log`) checks the generated files and the 85 address pairs. It then runs the vector cases against yaAGC's digests.
- **C64 execution results.** `make vice-cm-interp-bodies` (`evidence/cm-interp-bodies/cm-interp-bodies-vice.log`) runs all 244 vector cases in VICE against the same digests, in three programs (bodies 0-19, 20-39 and 40-61), because one program holding every body and vector does not fit below the `$c000` mailbox.
