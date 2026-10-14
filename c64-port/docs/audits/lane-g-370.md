# Certificate review: Lane G after PR #370

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
aa92adb6 (PR #370 merged), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Comanche055/CONIC_SUBROUTINES | `cm-conics` | re-confirmed (refrozen with the #325 PUSHLOC pin) |
| Comanche055/INTEGRATION_INITIALIZATION | `cm-integration-init` | re-confirmed (domain text only) |

## CONIC_SUBROUTINES

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL (the one WARN is the uncached nightly VICE manifest). All cited
  evidence hashes match; `source_revision` 911e5c02.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py cm-conics --cases 256` (the frozen
  size) prints PASS with all 113,920 cases equal across 445 routines. The case
  set hashes to the proof's `cases_sha256` (4070ee7d), and the stop table is
  identical to the frozen log. The strict `--cover-check` verifies 616 rows,
  2,121 of 2,121 host arcs, 0 missing.
- **Host and sanitizer.** `make test-cm-conics sanitize-cm-conics` gives
  3,560 checks with 0 failures each.
- **VICE.** The log has no FAIL, and its 33 PASS lines sum to 616, equal to
  `vice_targets`, the proof's C64 rows and the header rows. All 33 PRGs
  rebuilt at the tip are byte-identical to the logged ones.
- **Independent C64 branch check.** All 616 header rows match a regenerated
  case. Per-object gcov shows 0 missing, and the 33 KEEP-group builds take all
  host outcomes and lines, 0 missing.
- **The #325 pin.** At 12,2770 (COMMNOUT's BON/CALL NEWSTATE,
  CONIC_SUBROUTINES.agc 1131-1134) PUSHLOC is held to vac+0..+12. In flight
  it is vac+0: DELTIME ends "PL AT 0" (923), as do COMMNOUT's other callers
  TIMETHET (1121) and TERMNVEC, and NEWSTATE assumes PL 0. The pin is a
  superset of flight, and the 12,2761 and 12,2765 entries keep a random
  PUSHLOC. The #325 harness check (an unmapped stop inside the file fails the
  run) has landed. A salted run at 1,024 cases per routine (455,679 cases)
  shows 0 in-file stops. Its 2 differences are not wrong values: one is the
  native step budget (equal to yaAGC at 20,000 steps), and in the other both
  sides loop.
- **Not blocking.** The supported_domain covers the pin only through its
  general "MODE and PUSHLOC as the preceding orders leave them" sentence;
  naming 12,2770 and vac+0..+12 would be clearer. The generated-C changes
  (`index_carry16`, `image_w`, `trusted_image`) are inert in these builds
  (`RK_FIXED_IMAGE` unset, `ixw()` not called).

## INTEGRATION_INITIALIZATION

Only the supported_domain changed. It now names INTWAKUQ = 03704 at 27,2745
(TC INTWAKUQ), the return its QXCH saved. That narrows the stated domain to
what flight reaches, so the evidence stands as accepted in lane-g-258.md.
