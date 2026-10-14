# Certificate review: Luminary099/INTEGRATION_INITIALIZATION.agc

Reviewed 2026-10-14 by the certificate reviewer, against milestone
`lm-integration-init` (Certificate fixes lane, from Lane B), after #329.
Reviewed at ec35425b. Earlier rejection: issue #165.

Verdict: **accepted**.

## What was checked

- **The #165 gaps are closed.**
  - The MOVE[AP]CSM left-hand RTB (EDOP 0127/0153) is fixed:
    lm_integration_init.c:675, 749, 802 and 842 take both arms on the host and
    on the C64.
  - The C64 rows take every host branch arc. An independent per-object
    `gcov -b` run gives lm_integration_init.c 1,082/1,082. The only arcs
    missing on the C64 are 2 test-harness lines that the C64 build compiles
    out.
- **Flight X1 values.** RECTOUT (:650-655) writes the MUTABLE entry: −10D for
  the Moon, otherwise −2. This matches :182. Both MOONFLAG arms are compared
  with yaAGC.
- **The oracle reproduces.**
  - `make lm-integration-init-differential` gives 41,664 cases over 217
    routines equal to yaAGC.
  - The freeze is clean, and the regenerated source map is byte-identical.
  - The proof and the cover record differ only in arc counts (1,695 against
    1,690), which come from shared-source drift in agc_transfer.c.
- **Host and sanitizer:** 326 checks each, 0 failures.
- **VICE:** 12 PRGs rerun at the tip pass 259 checks with 0 failures, and
  every row matches its yaAGC digest. The PRG hashes differ from the evidence
  log only through shared-source drift (lm_interp_orders.c, lm_interp_steps.c,
  agc_transfer.c, tests/interp_case.c).
- **Rule #414.** The file has no BOVB TCDANZIG, and the spec still maps
  TCDANZIG/ENDT(X) to lm_integration_tcdanzig(s).
- **Domain.** It agrees with sane()/at_newops(). There is no run-time rope
  read and no generic decoding. All evidence hashes match.

## Notes for the lane (not blocking)

- The domain text says X1 is 0 or 2. At DOW.. / DOW..1, X1 is the DIFEQ stage
  (0, −12 or −24) indexing VECTAB.
- DDV* MUEARTH,2 (NORFINAL) leaves the interpreter's adjacent-majors DDV arcs
  untaken. With μ as the divisor, a quotient within 2^-14 of 1 lies outside
  the integration domain (inferred from the scaling). It belongs with #224.
- sane() also accepts FIXLOC 0110, which is wider than the text. Those cases
  are compared.
- Refresh the vice log and the arc counts at the next freeze.
