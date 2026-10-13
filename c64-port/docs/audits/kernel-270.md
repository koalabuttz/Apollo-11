# Certificate review: LM and CM WAITLIST after Kernel lane PRs #263 and #270

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` 1001a4bf.
The certificates are drafted by milestone `waitlist-entries`, and the earlier rejections
are in issue #198.

| File | Verdict |
|---|---|
| Luminary099/WAITLIST.agc | **accepted** |
| Comanche055/WAITLIST.agc | **accepted** |

## Earlier findings, now fixed

- **DLY2 -1 normal path (#263).** Every DELAYJOB case also runs a sane-waitlist variant: DELAYLOC free, a positive delay, TIME3 on both sides of its overflow test, and 0 to 8 queued tasks. In 45 LM and 52 CM cases the task is inserted and the job reaches JOBSLEEP. All of them equal yaAGC.
- **The 74 host-only arcs (#263).** C64 rows now take them all.
- **C64 row set vs cover record (#270).**
  - The diagnostic (`platform/c64/waitlist_entries_diagnostic.c:16`) and the cover tool (`tools/lane_a_c64_cover.py:78`) now pick the same rows.
  - The diagnostic, built for the host, produces 130 + 130 + 47 + 36 + 36 = 379 checks. That is 366 stride rows plus 8 cover rows, plus 5 refusals. The total equals VICE, vice_targets and the cover record.
- **LM WAIT2 CCS −0 (#270).**
  - The 57 new LM edge cases put the new task 1 cs before queued task j.
  - The −0 break at `lm_delay_image.c:25` is taken 57 times on the host and 14 times on the C64 rows.
  - All of them equal yaAGC.

## Reproduced

- **Host:** test and sanitize pass 1,437 cases and 1,446 checks with 0 failures, matching the host log.
- **yaAGC differential:** 1,437 out of 1,437 identical. The vectors are unchanged and the proof regenerates byte-identical.
- **VICE:** 379 checks, 0 failures. All five PRG hashes match the evidence log.
- **Evidence:** `evidence.py verify` passes, and all certificate hashes match. `cert_precheck.py` reports 0 FAIL.
- **Independent coverage:** `gcc -O0 --coverage` over the host cases and the exact C64 row set leaves 0 host arcs untaken by the C64 rows, measured per object.
- **Transfer ABI:** unchanged since the previous re-review. Every entry is registered at its listing address. The callers in other files reach registered entries: the T3RUPT vector, DELAYJOB's `TCF DLY2 -1`, and the P-axis `TCF NOQBRSM +1`. None of them is refused. These are basic-code specs, so the INDEX carry rule does not apply.

## Notes (not blocking)

- The +0 branches at `lm_delay_image.c:22-23` and `:27` are never taken. They are not plausible flight states.
- The #144 TASKOVER superbank guard is checked only by a synthetic test, not against yaAGC with the real rope.
- The exec-closure proof does not list waitlist-entries among its milestones.
- docs/NATIVE_TRANSFER_ABI.md still says the interrupt hardware always saves the word at ZRUPT-1. After an unextended INDEX, it saves the indexed instruction instead.
