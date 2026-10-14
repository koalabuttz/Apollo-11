# Certificate review: Luminary099/AOTMARK.agc

Reviewed 2026-10-14 by the certificate reviewer at `port/c64-native-c` 797f98ca,
after Lane D1 PR #294, against the certificate drafted by milestone `lm-aotmark`.
The certificate, `src/lm_aotmark.c`, `src/agc_transfer.c` and the spec are
unchanged at 1d7d45da.

This is the third review.
- The first (issue #261) rejected the certificate on two counts: `sane()`
  refused NOVAC FIXLOC 0110, and the MARKSTAT=0 entry paths were never compared.
- The second (after #282) found both fixed, but the "only VAC5 free" path at the
  AOTMARK entry (AGC 07,2024-2025, `src/lm_aotmark.c:434-437`) never ran.

Verdict: **accepted**.

## What was checked

- **The VAC5-free path now runs.** Lines 434-437 run in 129 of the 62,464
  cases, all equal to yaAGC. Four host vectors and one C64 row (case 9876)
  take the path too, and that row passes in VICE.
- **Earlier fixes still hold.** These are any_fixloc, the MARKSTAT=0 entries
  with MKVACFND searching through to BAILOUT1, `index_carry16` and `image_w`.
- **Reproduced at 797f98ca.**
  - `make lm-aotmark-differential`: 62,464 cases equal yaAGC. The vectors and
    groups regenerate unchanged, and the proof, C64 cover record and source map
    are byte-equal to the evidence.
  - `make test-lm-aotmark sanitize-lm-aotmark`: 2,030 checks each, 0 failures.
  - VICE: 21 PRGs and 302 checks, 0 failures. This equals `vice_targets`, and
    all 21 PRG hashes equal the log.
  - The 302 C64 rows are exactly the proof's case ids, and every digest equals
    the frozen one.
  - All evidence hashes match, and `cert_precheck.py` reports 0 FAIL.
- **C64 cover.** An independent per-object `gcc -O0 --coverage` build finds
  0 host arcs missing on the C64 (1,639 arcs, counting one harness arc the lane
  omits). Per call site, 18,691 of 18,693 edges are taken. The two left are
  shared-body edges called from 15,2007 and 15,2041
  (`agc_image_ops.c:193`, `lm_interp_orders.c:290`). Both are taken on the C64
  from 28 and 3 other sites, and the routine outcome at 15,2007 is unchanged.
- **Transfer ABI.** The routine keys and stop table are unchanged from the
  second review. The `src/agc_transfer.c` refactor behaves the same.

## Notes for the lane (not blocking)

- **Doc wording.** docs/LM-AOTMARK.md says "a quarter of them" take the
  VAC5-only state. It is a quarter of all AOTMARK entry cases (131 of 512).
- **Two cases stop early.** 2 of those 131 cases stop before line 434,
  presumably at an earlier domain refusal. This was not checked.
- **Remaining FIXLOC 0110 refusals.** These are at a few display returns. Those
  returns run in the VAC job, not the NOVAC one, so they do not block.
