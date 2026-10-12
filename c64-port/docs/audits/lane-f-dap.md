# Certificate review: LM THROTTLE_CONTROL_ROUTINES, DAPIDLER_PROGRAM, TRIM_GIMBAL_CONTROL_SYSTEM

Reviewed 2026-10-12 by the certificate reviewer at `port/c64-native-c` 44c6c4ac
(the merge of PR #79). This is a re-review after issues #76 and #82. It covers
the certificates drafted by `lm-throttle`, `lm-dapidler` and `lm-trim-gimbal`.

Verdict: **accepted** for all three. TJET_LAW (`lm-tjet-law`) from the same PR
is not accepted yet; its one remaining C64 item is on #76.

## Prior blockers

- **Throttle TPAGREE `CAF ZERO`** (`src/lm_throttle.c:117`, #76 item 1): fixed.
  128 cases take it and every one is compared with yaAGC, including the review
  case THROTTLE+3 A=00466, L=42635, MASS=31401 50337. The C64 rows take it 3
  times.
- **C64 rows** (#76 items 2-3, #82 item 2): fixed. `tools/lane_f_c64_rows.py`
  chooses rows by entry, outcome kind and path, and binds the result as
  `c64_branch_outcomes` (`<m>-c64-cover.json`). The reviewer did not rely on the
  cover file alone. A gcov build over exactly the C64 case set shows the same
  native lines and branch outcomes as the host stream for all three files.
- **Trim gimbal zero-root DOSHIFT** (#82 item 1): fixed. For k outside 0..13,
  `INDEX SHFTFLAG; CA BIT15` is executed as `CA 04735+k`. It reads either
  erasable memory or the compiled 3,072-word table `lm_trim_gimbal_ca_table.inc`.
  That table is generated from the certified rope and checked with `--check` in
  `test-*`, so nothing reads the rope at run time and nothing decodes
  instructions. These cases are compared against yaAGC (paths 85 and 86).
  - The 173 remaining refusals are exactly the addresses outside 0..07777.
  - The producer bound holds. TIMEGMBL approves GTS (ALLOWGTS > 0) only when
    NZACCDOT·BIT12 exceeds |1.6·AOS| without overflow. With ACCDOT > 0 that
    limits |AOS| to about 1279 LSB, inside the CA range of −2525..+1570
    (TRIM_GIMBAL :360-390, TRYGTS Q_R :437-440).

## Reproduced

| Milestone | `test-`/`sanitize-` | yaAGC differential | VICE | PRG sha256 |
|---|---|---|---|---|
| lm-throttle | 16,422 checks, 0 failures | 16,384 equal | 256 checks, 0 failures | `9794689f…` |
| lm-dapidler | 9,260 checks, 0 failures | 9,216 equal | 192 checks, 0 failures | `57c9f683…` |
| lm-trim-gimbal | 33,031 checks, 0 failures | 16,211 equal, 173 refused | 256 checks, 0 failures | `fd8a1a70…` |

For all three, the proofs and source maps regenerate byte for byte, and the
vectors have no diff. Each fresh PRG sha256 equals the one in its VICE log.
gcov leaves untaken only the guards and impossible outcomes already listed in
the #76 and #82 reviews; for trim gimbal it also leaves `a==5` (CA Z), which is
outside the producer bound.

The oracles now link yaAGC's register-level `SimulateDV` for every DV
(`tools/prepare_hardware_dv.py`, with the pinned source hash asserted).
- Throttle and DAPIDLER outputs are unchanged.
- In trim gimbal, 379 compared cases change. In each, stock yaAGC's fast
  division path mishandles L = −0 (A=1, L=−0, K=4 gives 177777 instead of the
  correct 010000). The native code now uses plain hardware division and matches
  the corrected engine.

The change is faithful to the AGC.

## Notes for the lane

- Regenerate the throttle cover JSON: the committed file says "left the file"
  where the tool now writes "kind 0". The data is identical.
- Record the hardware-DV engine variant and its sha256 in each proof.
- Add the TIMEGMBL bound to trim gimbal's `supported_domain`. Fix the C64 row
  count in `docs/LM-TRIM-GIMBAL.md:116`: it says 14, the tool prints 15.
- Have `tools/vice_run.py` print the PRG hash itself.
- Some cohorts are thin. Throttle path 26 has one input triple, trim gimbal
  path 85 has 8 cases, and there is still no directed CCS FUNCTION+1 +0 case.
