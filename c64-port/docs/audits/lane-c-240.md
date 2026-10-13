# Certificate review: Luminary099/EXTENDED_VERBS.agc after Lane C PR #240

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` def640e3.
The certificate is the Lane C EV certificate (evidence/lanec-ev-cert, docs/LANEC-EV-CERT.md).
The previous review was rejected in issue #115: four helper call sites had an outcome
that no C64 row took.

Verdict: **accepted**.

## The #115 gaps

C64 rows are now picked per helper call site (`LANEC_COVER_SITES`,
`cover.py pick --keep`). A printf probe over every frozen row confirms that each
missing outcome is now taken at its own call site:

| Site | Outcome | C64 row |
|---|---|---|
| src/lanec_ev_dap.c:149 | −0 | 137 |
| src/lanec_ev_short.c:103 | −0 | 212 |
| src/lanec_ev_short.c:168 | −0 | 1659 |
| src/lanec_ev_align.c:129 | positive | 51 |
| src/lanec_ev_align.c:129 | negative | 683 |

All four CCS classes at align:129 now run on the C64.

## Reproduced

- **Host, sanitizer and VICE.** The test, sanitize and vice targets pass for all eight EV module suites, ev-transfer, lanec-ev-cert and lanec-pin-transfer.
  - The differentials compare 28,820 cases: 0 differ, and 512 are refused with no state change.
  - VICE runs 481 checks with 0 failures (front 23, short 122, align 104, dap 46 and the unchanged suites). This equals vice_targets.
- **Evidence integrity.**
  - All nine PRG sha256s match `lanec-ev-c64-results.json`.
  - The oracle CRCs and diff summaries match `lanec-ev-comparison.json`.
  - Every file matches SHA256.json and the certificate hashes.
  - The source map is byte-identical.
- **Evidence categories.** The four categories are distinct files. The cover record uses no legacy, host-only or pool arcs.
- **Independent coverage.** An independent `gcc -O0 --coverage` build with per-call-site counters finds that the C64 rows take every host arc in all nine suites, for example short 238, align 335 and dap 226.
- **Unchanged since the previous review.** The EV module sources and case generators are unchanged, so these earlier findings still hold:
  - the transfer ABI scan: GOEXTVB via PINBALL:979, TESTXACT, SDISPLAY, ENDSUMS, TRIMDONE, TRMTRACK;
  - NEWJOB ≠ 0 is compared;
  - no genuine caller is refused.
- **Channel 11 fix in #240.** The fix is one line in each LM flight-store wrapper (`src/lanec_ev_routines.c:34`, `src/lanec_pin_routines.c:79`). It does not touch `src/flight_io.c`, and no certified evidence depends on those wrappers.

## Notes (not blocking)

- No C64 row exercises the APSFLAG/01046 ascent-minimum −0 case. Every arc is still taken; adding one row would be cheap.
- The transfer harnesses read `hardware.channel11`, not the device record's copy. That is why this evidence could not see the channel 11 loss.
- docs/LANEC-EV-LMI.md:79-83 still has stale wording.
