# WAITLIST CCS positive-zero insertion

The original WTLST5 branches distinguish both zeros. At an early CCS, positive
zero skips the interval addition and proceeds to the next CCS. At the final
slot, positive zero skips FILLED through NOOP/AD ONE and inserts at slot eight;
negative zero skips AD ONE. The previous native loop added intervals skipped
by the source and rejected final positive zero as a full queue.

`wl_schedule` now preserves that source control flow and raw zero encoding.
The independent unchanged-assembly regression checks 256 complete 29-word queue
states across eight CCS positions, positive/negative/plus-zero/minus-zero input,
live/end-marker discarded tails, and four task bank words. The previous binary
has 688 field mismatches; the fix has zero. Snapshot SHA:
`83344e2046c4c6b53583bf9246dc3486e91356e7f26637b3c3af3c09c0daaa83`.
The original 196-case, 306-checkpoint full Waitlist comparison remains unchanged
and passes. Earlier retained evidence identities are preserved.

Strict host and ASan/UBSan checks and all 256 full-state PAL C64 checks pass;
mailbox `a50000001d`. The PRG is 22,120 bytes, with 6,632 text, 15,474 read-only,
63 BSS and 78 static-stack bytes. Extra zero page is zero; dynamic hardware and
software sentinel extents are 12/6 bytes. These are finite diagnostic measures.
`tools/waitlist_zero.mk` supplies the regression targets alongside the existing
Waitlist targets; its differential and VICE tools reproduce the new checks.
