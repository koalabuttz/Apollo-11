# Certificate review: Comanche055/RCS-CSM_DAP_EXECUTIVE_PROGRAMS.agc

Reviewed 2026-10-14 by the certificate reviewer. The review was against the
certificate drafted by milestone `cm-rcs-dap-exec` (Kernel lane A).

- **First review:** at 3d41201b, after #304. It was rejected in issue #307.
- **Re-review:** at 773644c1, after #312.

Verdict: **accepted**.

## What the first review found

1. A = ±0 was never compared at the SPSIN/SPCOS return entries, 22,3470 and
   22,3501. CDUX = 660 or 660±20000 gives ±0 there, and the following
   `MP CAPSI` then takes yaAGC's ±0-multiplicand path.
2. entry_contracts described `sane()` refusals of NEWJOB and FIXLOC. The
   routines use `basic_sane` instead.

## What was checked at 773644c1

- **The A = ±0 cases are fixed.** About 19% of the cases at each entry now
  have A = ±0:
  - 3470: 52 cases with +0 and 44 with −0;
  - 3501: 49 cases with +0 and 51 with −0.

  These come in every sign pairing with CAPSI, and all compare equal to yaAGC.
  No C64 row has A = ±0. That is acceptable, because native `agc_multiply`
  has no separate arc for it, so the per-object cover rule is met.
- **The domain text is fixed.** entry_contracts now matches `basic_sane`
  (src/cm_rcs_dap_exec.c:316-322).
- **The closure reproduces.** 2560 cases equal yaAGC, and 32/32 instructions
  are executed. The vectors and groups regenerate unchanged, and the proof,
  map and closure files are byte-identical.
- **Host and sanitizer:** 160 checks each, 0 failures.
- **VICE:** 13 checks, 0 failures. The count equals `vice_targets`, and the
  PRG hash equals the log.
- **C64 rows.** They are exactly the claimed ids. An independent per-object
  `gcc -O0 --coverage` run finds 0 host arcs missing on the C64. The only
  host-only arcs are LM-only harness lines, which are not built into the PRG.
- **Ledger checks.** All evidence hashes match, and `cert_precheck.py`
  reports 0 FAIL. Its NEWJOB WARN is in dead `sane()`/`at_newops`/`at_basic`
  helpers.
- **Steward rule.** `index_carry16` and `image_w` are set.
- **Transfer ABI.** AMBGUPDT's two NOVAC 2CADRs are registered, and exits go
  through the trampoline. This was checked in the first review, and the
  routine set is unchanged.

## Notes for the lane (not blocking)

- **Doc fixes.**
  - The docs mention "MP AMGB1", but both multiplies are `MP CAPSI`.
  - The text says CDUZ 0 makes A ±0 at these entries. Only CDUX does.
- **Logs.** The logs name base commit e3b1d3ed.
- **C64 gcov count.** The proof's C64 count of 157 comes from a host-flag build.
  The IRC_CM_ONLY build gives 155, the difference being LM-only lines.
- **Dead helpers.** Consider not emitting the `sane`/`at_newops`/`at_basic`
  helpers for files without interpretive code.
