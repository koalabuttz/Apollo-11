# C64 module-test harness

`tools/module_test.py` runs a suite's cases on the C64, in VICE, through
flight modules that the flight resident loads from the REU store
(`make flight-store`, docs/FLIGHT-STORE.md). Each complete result is compared
with the host's result for the same case.

It exists for code that a stand-alone C64 diagnostic cannot hold. A
`tools/lanec/c64_diag.c` program links the suite's case generator, the
module and its frozen rows into one 38 KB program. The harness keeps only
the flight resident and a small runner in memory. The module sits in its
16 KB slot, as in flight, and the cases come from the REU, so the
generators are never linked.

## What a run does

The host side (`tools/module_test.py`) works in four steps:

1. It reads three files of rows for one suite:
   - `--states`: each case's entry state (`n LM|CM`, the image's 2048 words,
     channels 0-077, superbank and IRQ enable). For Lane C suites,
     `tools/lanec/native.c --states SUITE` writes this.
   - `--oracle` and `--native`: the host results in the oracle's row format.
2. It keeps the cases of the requested mission (or the `--cases` list). The
   expected result is the oracle row. For a case the native code refused
   (status 1), it is the native row, which is the unchanged state. This
   follows `tools/lanec/freeze.py`.
3. It writes the case records into the store image's unused gap below the
   rope. The gap holds about 195 LM or 100 CM cases, so more cases run in
   batches, one VICE run each.
4. It links `tools/module_test.c` against the flight resident, like the
   flight-store load check.

The C64 side (`tools/module_test.c`) boots the flight resident on the store
(`rk_boot`). Then, for each case:

1. It fetches the record into the image and a device record of the modules'
   io kind (`lm_dntm_io` or `cm_dntm_io` so far).
2. It runs routines one at a time from the entry point, through the table
   lists of the loaded modules under test.
   - At a point no loaded module holds, it asks the store's resolver which
     module holds it. A module under test is loaded with an ABI 2 run from
     a point no table holds, then the point is tried again. Any other
     module ends the case.
   - The case also ends at its stop point or at its routine limit
     (`--limit`).
3. An entry point no module under test holds, or a first routine that
   refuses, gives status 1, and the state must be unchanged. A routine
   that refuses after others have run gives status 2, which matches no
   host row.
4. It computes the CRC-32 of the image, channels, superbank, IRQ and status,
   and compares it with the host's.

The mailbox at `$c000` holds the counts, the first failing step and up to
32 failing case numbers. Results go to `build/module-test/<name>-<m>.json`.

## Using it

A lane hooks a target into `module-test` (`tools/mk/module-test.mk`). Lane
C's `module-test-lanec-pin-keys` (`tools/mk/lanec-pin-keys.mk`) runs every
case of the `pin-keys` and `pin-key-labels` suites, in both missions,
through the five keypress modules (docs/LANEC-PIN-KEYS.md), one routine per
case. Every case matches the host: 1408 per suite and mission, 5632 in
all. The keypress routines run through the Pinball trampoline, which takes a
TC it ends at in a register or return cell; `--tc-cells` names those cells
so the host row is advanced the same way.

A suite whose modules take another io kind needs that device record added
to `tools/module_test.c`, mapping the case's channels in and out.
