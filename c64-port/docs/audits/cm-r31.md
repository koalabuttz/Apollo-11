# Certificate review: Comanche055/R31.agc

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
6d14ce1f (the merge of PR #157). The certificate was drafted by milestone
`cm-r31` (Lane E).

Verdict: **accepted**.

## What was checked

- The four evidence categories cite four distinct records, and none cites the
  source map. `tools/cert_precheck.py Comanche055/R31.agc` reports 0 FAIL once
  the release evidence is fetched. Every hash matches `evidence/cm-r31/SHA256.json`
  and the certificate.
- Transfer ABI. `resume_keys` is on, so every word after a DANZIG is a
  registered routine at its original address. The differential reaches 89
  routines: V83, R31CALL, V83CALL, their basic and interpretive continuations,
  and the FINDVAC and BANKCALL returns. cert_precheck finds no unregistered
  entry and no unregistered CHANG2 resume word.
- Domain. `tools/interp_specs.py` `cm-r31` pins no erasables beyond the
  canonical image. NEWJOB is positive in a quarter of the cases, edge-value
  families are on (`edge_cases`), and LOADTIME runs through `rtb_angles`.
  No case is refused or waits for a channel model.

## Reproduced

- My own build of the pinned reference (prepare_reference,
  cm_services_prepare_reference, fresh listings) ran
  `tools/interp_routine_differential.py cm-r31 --cases 512`. All 45,568 cases
  equal yaAGC on the unmodified Comanche055 rope, across 89 routines, with no
  refusals and no timeouts.
- The host test and an ASan/UBSan build of `tests/test_cm_r31.c` give
  356 checks with 0 failures each.
- The VICE log totals 118 checks with 0 failures over the group PRGs, which
  equals `vice_targets` `vice-cm-r31: 118`.
- Independent C64 branch check: I regenerated all 45,568 cases with the
  spec's own generator. The 118 C64 rows' seeds equal
  `tests/cm_r31_c64_vectors.h`. I built the native runner at
  `-O0 --coverage` over every linked source, ran gcov `-b` per object (so
  `cm_interp_orders.c`'s include of `lm_interp_orders.c` is counted
  separately), and diffed all cases against exactly the 118 rows. Every
  branch outcome the host stream takes is also taken by a C64 row: 0 missing
  in `cm_r31.c`, the CM interpreter orders and steps, `rtb_angles.c`,
  `agc_image_ops.c`, `agc_hardware_divide.c`, `agc_transfer.c`, `agc_word.c`
  and `agc_multiword.c`.

## Notes

These outcomes in `src/cm_r31.c` are never taken by any case, host or C64.
Each was checked:

- The entry guards (`at_basic`, `at_newops`, `sane`): defensive refusals.
- The overflow skip of TS after CA (31,2314 and 31,2317) and after
  CA FIXLOC, AD SIX, COM (31,2324). A CA leaves no overflow, and FIXLOC is at
  most 0661, so these can't be reached.
- The UNIT and ABVAL alarm exits (lines 294, 345, 421, 503, 529). These need
  a zero or overflowing vector, which is degenerate for the V83/R31
  rendezvous display.
- The `cmi_switch` refusals on out-of-domain switch words.

The drafted certificate named `source_revision` 6e0a5a79, a commit of this
repository, not the pinned upstream revision. `tools/port_coverage.py`
requires 911e5c02 for a complete entry. `Comanche055/R31.agc` at 911e5c02
has the inventory's sha256 (94af1db0…), which is the text reviewed here, so
the flip records 911e5c02.
