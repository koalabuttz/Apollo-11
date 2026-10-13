# Certificate review: Luminary099/T4RUPT_PROGRAM.agc

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` 94e77312
(after Kernel lane PR #205), against the certificate drafted by milestone
`lm-t4rupt-chain`. That certificate rests on `t4rupt-core`, `lm-t4rupt-imu` and
`lm-t4rupt-rr`.

Verdict: **accepted**.

## Reproduced

- Every evidence hash matches. The four categories are distinct records (closure source map, closure proof, closure host log, closure VICE log), and the C64 branch record is `lm-t4rupt-chain-c64-cover.json`.
- `make test-/sanitize-*` gives 0 failures: core 11400, imu 9582, rr 10379 and chain 12299 checks. The chain run returns 4096/4096 cases, with 0 refused.
- The yaAGC differentials on the unmodified rope are identical:
  - core: 2240 LM + 2176 CM cases;
  - imu: 4736;
  - rr: 3456;
  - chain: 4096 whole interrupts.
- Vectors are unchanged. The proofs and source maps regenerate byte for byte, and the closure map executes 571/571 instruction lines.
- VICE gives core 276, imu 300, rr 216 and chain 266 checks, all with 0 failures. The chain PRG sha equals the log.
- The independent `gcc -O0 --coverage` run, per milestone and per object, compares all host cases against exactly the C64 rows (core measured with LM rows only). The C64 rows take every host arc in `t4rupt_core.c`, `lm_t4rupt_imu.c`, `lm_t4rupt_rr.c`, `agc_transfer.c` and `agc_image_ops.c`. The only misses are two lines in the test case generators.

## Checked

- The edge states below are taken and compared on host and C64:
  - DSRUPTSW CCS outcomes, including SYNCT4 to −0;
  - CDRVE, DSPOUT, NODSPOUT and HANG20;
  - QUIKDSP and QUIKOFF;
  - every IMODES30/33 handler (ITURNON, IMUCAGE, ISSZERO, IMUOP, PIPFAIL, DNTMFAST/UPTMFAST, TLIM, SETISSW);
  - GLOCKMON with CDUZ ±0;
  - the RR monitor branches;
  - the TIME4 paths.
- NEWJOB is randomized and compared.
- Transfer ABI: every cross-bank entry and return point is registered at its original 06,xxxx address. This covers the T4RUPT entry, the NOVAC/ALARM/WAITLIST/VARDELAY/IBNKCALL return points, ENDTNON, UNZ2, ISSUP and SETISSW (IBNKCALL from IMU_MODE_SWITCHING).
- The #198 RESUME superbank fix is present. No case is refused.

## Notes for the lane (not blocking)

- Two CCS outcomes need every bit set at once and are judged implausible in flight: TLIM `CCS IMODES30` = −0, and `CCS FLAGWRD5` = −0. Their native code is correct by inspection but uncompared. Add one case for each, or state them in the supported domain.
- The core C64 set is a pure stride of 16. It happens to take every native arc. A gcov-picked cover would make that hold by construction.
