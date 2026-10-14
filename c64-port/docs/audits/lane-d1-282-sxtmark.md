# Certificate review: Comanche055/SXTMARK.agc

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` fb5558df,
after Lane D1 PR #282, against the certificate drafted by milestone `cm-sxtmark`.
The certificate, `src/cm_sxtmark.c`, the spec and the C64 vectors are unchanged
at 96fb4fcd.

This is the second review. The first (issue #255) was rejected because neither
the DIFCHK accept path nor alarm 122 was ever compared.

Verdict: **accepted**.

## What was checked

- **The #255 paths are now compared.** Every count below is compared with yaAGC
  on the host and taken by C64 rows, given as host cases / C64 rows.
  - DIFCHK `CCS A` +0 → `TC Q`: 246/1.
  - `BZMF -3` taken → `TC Q`: 524/3. The alarm-121 fall-through: 522/1.
  - All three returns into MARKDIF (B20212, B20214, B20216/MKACPT), plus an
    external Q.
  - `BZMF MKACPT`: both arms.
  - MARKCONT `CCS MARKSTAT` +0: 230/3.
  - Both `BZF MARKET3` arms at SXTMARK.agc:334 and :338.
  - MARKET3 → ALARM 122: 171/2.
  - NOVAC MARKDISP: 59/1.
- **The case mix is plausible.**
  - CDUCHKWD runs 1-6.
  - Mark CDUs are within 0 to ±4 bits of CDUX-CDUZ, across the 0/77777 and
    37777/40000 wraps.
  - MKNDX is -1, 0 or 1.
  - MARKSTAT +0 is drawn with the flag bits varied.
  - Half of each routine's cases keep the random states.
- **Reproduced at fb5558df.**
  - `make cm-sxtmark-differential`: 21,504 cases equal yaAGC across 42
    routines, and the log is identical to the release log. The vectors, C64
    vectors, groups and source map regenerate byte for byte.
  - `make test-cm-sxtmark sanitize-cm-sxtmark`: 679 checks each, 0 failures.
  - `make vice-cm-sxtmark`: 73 checks, 0 failures. This equals `vice_targets`,
    and the PRG hash equals the log.
  - The 73 C64 rows are exactly the proof's `case_ids`, matched by digest.
  - All evidence hashes match, and `cert_precheck.py` reports 0 FAIL.
- **C64 cover.** An independent `gcc -O0 --coverage` build, keyed per object,
  finds the same taken arcs on the C64 rows as on the host cases:
  `cm_sxtmark.c` 333, `agc_image_ops.c` 55, `agc_transfer.c` 40.
- **Steward rule.** Both `index_carry16` and `image_w` are set. Every INDEX site
  computes its address with signed arithmetic and range-checks it.
- **Transfer ABI.** The 42 routine keys and the stop table are unchanged from
  the first review. No outgoing transfer ends at a bare Z.

## Notes for the lane (not blocking)

- **Proof counts.** Two derived counts in the proof do not regenerate here:
  `c64_coverage` edges 704 against 699 committed, and `c64_gcov_branches` 489
  against 486. Host equals C64 in both versions, so this is probably the
  toolchain. Refresh the proof at the next refreeze.
- **Dead helpers.** The `sane`/`at_newops` helpers are dead. Removing them
  silences the cert_precheck NEWJOB WARN.
- **Thin C64 coverage.** Each new outcome is taken by only one to three C64
  rows.
