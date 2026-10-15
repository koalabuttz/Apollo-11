# Second review: re-cite cm-entry-dap after #527

Second certificate reviewer, 2026-10-15. Lane G's #527 (merged as 4214bb6a) refroze cm-entry-dap (Comanche055/CM_ENTRY_DIGITAL_AUTOPILOT.agc) to add a hot table: spec key `hot_points` emits `cm_entry_dap_hot_routines`, 26 entries. It was complete on docs/audits/rev2-recite-g515.md. **Verdict: re-cited.**

## Code

- **Hot table.** All 26 keys use the same `_abi` wrapper, under the same KEEP `#if` guard, as `cm_entry_dap_basic`. The wrappers keep the AGC_CH_HW_SET and `basic_sane` checks, so no hot entry bypasses an entry guard.
- **Use.** Only tests/test_cm_entry_dap_hot.c reads the table today; it checks that each key calls the same function through either table. flight_store_build.py skips `*_hot_routines` tables unless a CM pin or pack lists one, and none does. No runtime path changes.
- **Shared tool.** interp_compile.py adds `hot_table()` only for specs with `hot_points`; `--check` passes on cm-entry-dap, cm-rcs-dap, cm-servicer and p51-p53.
- **Certificate.** Only the proof and VICE-log hashes change; vice_targets stays 79.

## Evidence at 4214bb6a (reproduced)

| Check | Result |
|---|---|
| Hashes | All 5 files match SHA256.json and the certificate |
| Differential | 7,168 cases (138 routines) equal yaAGC; log byte-identical apart from its "froze" line |
| Host + sanitizer | 224 checks, 0 failures each; test-cm-entry-dap-hot 26 entries, 0 failures |
| VICE | 3 PRGs byte-identical to the log; 34 + 29 + 16 = 79 checks = vice_targets |
| `--cover-check` | verified, cases_match; 913/913 branch outcomes, 0 missing |
| `cert_selfcheck cm-entry-dap` | OK, no gaps |
| `cert_precheck` | 0 FAIL (NEWJOB WARN at line 46 predates this; vice-manifest WARN, vice_targets checked by hand) |

**NOTE.** The proof's edge count went 1,263 to 1,262. Recomputing at both the merge and its parent gives 1,262 with 0 host-only edges, so the drop is from the earlier freeze's environment, not this PR, and no coverage is lost.

**NOTE (for Lane G, not blocking).** `hot_full('cm_entry_dap_hot_routines')` in tools/flight_store_build.py yields `cm_entry_dap_routines`, but the full table is `cm_entry_dap_basic`. It is inert today and needs fixing before the hot table joins a CM pack.
