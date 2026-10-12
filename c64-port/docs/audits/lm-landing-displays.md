# Certificate review: Luminary099/LANDING_ANALOG_DISPLAYS.agc

Reviewed 2026-10-12 by the certificate reviewer at `port/c64-native-c` 777b920c
(the merge of PR #103, which re-chose the C64 rows), against the certificate
drafted by milestone `lm-landing-displays`.

Verdict: **accepted**.

## What was checked

- The four evidence categories cite four distinct records, and none cites the
  source map. All hashes match `SHA256.json` and the certificate.
- Transfer ABI. These are registered at their original addresses:
  - 21,2154 LANDISP;
  - 21,2365 INTLZE;
  - the TWIDDLE return (21,2364);
  - the VARDELAY return (21,2476).

  LANDISP's only outside references are in-bank BZF/TCF in P70-P71.agc:75-100
  (bank 21). Nothing reads the rope at run time.
- Every `#####` line and every taken-0 branch in `src/lm_landing_displays.c`
  is on the documented unreachable list (`tests/test_lm_landing_displays.c:24-40`).
  Each was checked against the AGC source:
  - CCS after MASK of a positive constant;
  - TS after MASK;
  - the NEWDATA CCS signs, which OLDDATA's sign-forcing chain fixes;
  - CCS LATVEL −0, which needs x = ±0, where MP gives +0;
  - the ITEMP5 loop counter.

  The entry guards are defensive.

## Reproduced

- `test-`, `sanitize-`, `-differential` and `vice-lm-landing-displays`:
  - 16384 cases, 0 refusals, 32872 checks, 0 failures on host and on
    ASan/UBSan;
  - 16384 of 16384 equal yaAGC;
  - the source map passes (467 words), the proof regenerates byte for byte,
    and the vectors have no diff;
  - VICE passes 128 checks with 0 failures. The PRG sha256 `435291b6…`
    equals the log.
- Independent gcov `-b` over all seven native sources, run over the host
  stream and over exactly the 128 C64 rows: every line and branch outcome the
  host takes is also taken on the C64.

## Notes

- `lm-landing-displays-c64-cover.json` regenerates with the same rows and
  counts, but different block ids. `tools/lane_f_path_trace.c` numbers
  blocks by hashed absolute PC, which changes with ASLR. Filed with #108.
