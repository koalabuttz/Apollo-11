# Flight parts of a compiled-routine file

Pinball module split lane, for the steward's flight-store overflow list
(`/mnt/project-files/pin-split/store-overflow.md`).

Some files that `tools/interp_compile.py` generates have more basic code
than one 16 KB flight job slot holds. Comanche055 P40-P47 is one: with
basic tables only, its module was 61,830 bytes over the slot. 36 KB of that
was the 190 per-key entry routines (`r<addr>_abi`, about 200 bytes each on
the 6502); the rest was the six banks' basic code.

A spec's `'flight_parts'` lists cuts: physical addresses, ascending. With
`<SYM>_FLIGHT_PART=j`, the file builds part j of the C64 flight store:

- **The banks it holds.** `<SYM>_BANKS` keeps only the banks with words
  between the part's cuts, so the others' functions drop out.
- **A word of another part.** In a bank a cut divides, every word is
  guarded by `<SYM>_FP`. A run that reaches another part's word, or a
  bank the part lacks, calls `<sym>_leave`. When the word is a key of
  any part, that sets Z to it and returns `EXEC_OK`, so the transfer
  engine goes on in the module that holds it. Any other word is refused.
- **Its keys.** The part's keys are the file's basic routines between its
  cuts, plus part keys. A part key is a word a goto across a cut
  reaches, or any computed-transfer target. Part keys exist only in
  flight builds.
- **One entry routine.** All the part's keys share one entry,
  `<sym>_flight_entry`, which finds the key from Z, FB and SUPERBNK. It
  makes the same checks as the per-key routines, which are left out.

Without `<SYM>_FLIGHT_PART` the file compiles to the same tokens as before.
That holds for the host build, the coverage build and the C64 diagnostic
groups (KEEP and BANKS masks), so the certified tests and evidence don't
change. The interpretive table is unchanged too: under the
native-interpreter decision, the C64 flight store leaves interpretive code
to the interpreter.

## Comanche055 P40-P47

The cuts are at bank 20, bank 21, and bank 24 `02300`, `03000` and `03300`.
The parts' flight links, measured with `tools/flight_store_build.py`'s
`link_module`:

| Part | Basic words | Bytes |
|---|---|---|
| 1 | bank 16 | 7,911 |
| 2 | bank 20 | 12,033 |
| 3 | bank 21, bank 24 to `02277` | 11,297 |
| 4 | bank 24 `02300`-`02777` | 15,319 |
| 5 | bank 24 `03000`-`03277` | 12,510 |
| 6 | bank 24 from `03300`, banks 27 and 42 | 15,205 |

The flight store has to link the file once per part with
`-DCM_P40_P47_FLIGHT_PART=j`. Each link exports `cm_p40_p47_basic` with
that part's keys, which don't overlap with any other part's.

## Evidence

`tools/flight_parts_check.py <spec>` (`make flight-parts-check`) first runs
the file's differential (`tools/interp_routine_differential.py`), which
checks the whole build against the original program. It then builds every
part for the host and one runner with all the parts' tables, the way the
store's modules work together. Every case must end exactly as it does in
the whole build: Z, the 2048 erasable words and the interrupt state.

For P40-P47, all 31,744 cases (64 per routine) end the same. 1,597 of
them cross from one part to another at least once.
