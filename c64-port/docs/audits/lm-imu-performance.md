# Certificate review: Luminary099/IMU_PERFORMANCE_TEST_2.agc and IMU_PERFORMANCE_TESTS_4.agc

Reviewed 2026-10-12 by the certificate reviewer at `port/c64-native-c` bf0a27d6
(after PR #100), against the certificates drafted by milestone
`lm-imu-performance`. This is a re-review of the rejection in #88.

Verdict: **accepted** for both files.

## What was checked

- The four evidence categories cite four distinct records, and none cites the
  source map. `c64_branch_outcomes` is a fifth, separate record. All five
  hashes match `SHA256.json` and both certificates.
- All six blocking items of #88 are fixed:
  1. The 1STO2S/EXIT return 37,2027 is registered. All 384 cases at that key
     are compared.
  2. FINETIME +1 (04103) is registered, and the BZF at 04117 stops there. The
     oracle stops at the same idle point, then runs another loop and requires
     all 2048 erasable words and the interrupt state to be unchanged, so each
     stop is a real comparison plus a fixed-point check. LOSCALAR 37777 is
     drawn.
  3. All 125 order-pair and store words that can follow a DANZIG are
     registered CHANG2 resume keys; they were checked against the listing.
     The only word without a key is 37,2662, which STADR consumes inside the
     2661 pair, so it can never be a resume point.
  4. The CHECKG1 wait stops at 02335 and resumes through `RT(02335)`. Both
     outcomes of the 02337 BZMF are taken.
  5. TORQNDX, LENGTHOT, ALTIM, DATAPL+1 and NEWJOB are drawn independently.
     Every CCS outcome at the LENGTHOT/TORQNDX tests is taken.
  6. The C64 rows run in seven parts (ALFLT is split at ALKLP).
- New refusals, each checked against the AGC source and found unreachable:
  - X1/X2 outside ±0377 needs malformed ground data;
  - VXSC GEORGEJ with a scalar MODE cannot happen, because 37,3013 always
    follows `PDDL VDEF`;
  - AINGOTN DDV with a zero-major divisor cannot happen, because the divisor
    is about 584 s of elapsed time.

## Reproduced

- `test-`, `sanitize-`, `-differential` and `vice-lm-imu-performance`:
  - host, part 0 and ASan/UBSan each gave 64512 cases, 129258 checks and
    0 failures;
  - 56580 cases equal yaAGC and 7932 are refused;
  - the proof, source map and cover JSON regenerate byte for byte, and the
    vectors have no diff;
  - VICE passes all seven parts (650 checks, 0 failures), and every PRG
    sha256 equals the log.
- Independent gcov `-b` over the host stream and over exactly the 650 C64 rows:
  host takes 764 of 1140 outcomes and C64 takes 765. Nothing is host-only.
  The one C64-only outcome is the chained-step stop at
  `src/lm_imu_performance.c:917`, which part 0 cannot take.

## Notes for the lane (not blocking)

- The AINGOTN refusal rationale is wrong. The shared `lmi_ddv` does not
  mishandle a zero divisor: with address 1 allowed as an oracle stop in
  `ipt_leaves`, 180 of 180 zero-divisor cases equal yaAGC. Restate the
  refusal as unreachable in the source, the header, the doc and
  `supported_domain`.
- `tests/lm_imu_performance_cases.c:104-107` and the doc misstate the genuine
  ALX1S/CMPX1 values. The source gives ALX1S = 144, then 132 down to 0, and
  CMPX1 = ±1. A probe with the genuine values takes the same branch outcomes.
- The CHECKG1 continuation at 02335 leaves interrupts inhibited, and the
  original's RELINT window (02336-02345) has no ABI boundary. Document 02335
  as an interrupt point for the scheduler.
- In parts 1 and 5-7, `.rodata` overlaps the `$c000` mailbox. This is harmless
  because the mailbox is written only after all checks. Soft-stack headroom
  is about 678 bytes.
