# Certificate review: Luminary099/MAIN.agc and Comanche055/MAIN.agc

Reviewed 2026-10-14 by the certificate reviewer at `port/c64-native-c`
6fef0b84, after Kernel lane PR #302, against the certificates drafted by
milestone `main-assembly`. Both certificates are unchanged at 4e6268e3.

MAIN.agc holds only `$` includes, so porting it means reproducing the
include set, the include order and the bank layout of the rope.

Verdict: **accepted** for both files.

## What was checked

- **Includes.** The reviewer derived the lists independently from MAIN.agc,
  the pinned yaYUL listing and the rope bins.
  - LM has 89 includes and CM has 84. Each checked-in `.agc` file is included
    once, in the listing's order.
  - MAIN itself emits no word, and no word is emitted twice.
- **Word accounting.** Every bank word in the rope is accounted for.
  - LM: 36,539 emitted + 289 fill + 36 checksums = 36,864.
  - CM: 36,504 + 324 + 36 = 36,864.
  - All 72 bank checksums are correct.
- **Errata.** The two LM errata hold against the listing and the checked-in
  names:
  - LAMBERT_AIMPOINT_GUIDANCE.agc is the checked-in
    GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc;
  - TRIM_GIMBAL_CNTROL_SYSTEM.agc is TRIM_GIMBAL_CONTROL_SYSTEM.agc.
- **Drift check.** `tools/main_assembly.py --check` failed on each of three
  manifest mutations.
- **Oracle.** `tools/main_manifest_reference.c` compares each file's words
  with yaAGC's own load of the rope. It also runs yaAGC from every unused
  TC-to-itself word in banks 0-37: LM 89 files and 57 loops, CM 84 files and
  52 loops, 0 differing. The proof regenerates byte for byte.
- **Host, sanitizer and VICE.** Each runs 177 checks with 0 failures. The VICE
  count equals `vice_targets`, and the PRG hash equals the log.
- **Ledger checks.** `cert_precheck.py` reports 0 FAIL, and `port_coverage.py`
  passes.
- **C64 evidence.** `c64_branch_outcomes` cites the doc. That is acceptable
  here, because the C64 runs the whole check rather than picked rows and
  there is no branching code to cover.

## Notes for the lane (not blocking)

- **Source map no longer regenerates.**
  - The committed source map embeds each include's `coverage_status`
    (tools/main_assembly.py:150).
  - The LM AOTMARK flip in #300 changed one of those statuses, so the map no
    longer regenerates against the certified hash. The lane did not cause
    this drift, and every later flip would repeat it.
  - Drop the field, regenerate the map, and re-hash both certificates.
- **"Same files" overstates.**
  - docs/MAIN-ASSEMBLY.md ("Same files, same order") and
    tools/main_assembly.py:11 claim more than the tools check: only the names
    are compared.
  - Even with comments and whitespace removed, 18 LM and 22 CM checked-in
    files differ from the pinned Virtual AGC sources. Examples are SBANK=
    lines, `TCF` versus `TCF +0`, and an extra EQUALS.
  - A verbatim assembly of the checked-in LM tree gives a rope that differs in
    4 words (SERVICER 33,3210; TRIM_GIMBAL_CONTROL_SYSTEM.agc:382 21,3373;
    plus two checksums). The CM tree does not assemble.
  - The certificate's own wording is scoped to "the pinned MAIN.agc that yaYUL
    assembles" and is accurate. The doc should say "files of the same names"
    and leave content equivalence to each file's own entry.
- **Renames and reorders.** Neither the oracle nor the C64 check catches an
  include rename or reorder; only `--check` does. A per-row include-name hash
  in the generated manifest would close that.
- **Checksum values.** No lane tool checks the bank checksum values; they are
  only located by address. A ± bank-sum check in `main_manifest_check` would
  be cheap.
- **Bank-0-37 limit.** The oracle runs no TC-to-itself word in banks 040-043
  (LM 8, CM 6). The doc honestly says banks 0-37.
- **Parity wording.** "(bank order and parity)": the yaYUL bins carry no parity
  bits.
