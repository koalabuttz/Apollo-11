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

Both missions and both basic layouts work: a `'basic_split'` file (one
function per bank) and a `'basic': 'full'` file (one `basic_run`) get the
same guards, part keys and entry. Luminary parts read channels through
`AGC_CH_HW_SET`, Comanche parts through the native I/O hooks.

Keys in fixed-fixed memory (banks 02 and 03) are allowed only in a spec
with `'fixed_fixed_any_fb'`, whose per-key routines already run there under
any FB and SUPERBNK: the flight entry does the same for those banks, and
`<sym>_leave` sets Z to the key's physical address, which for banks 02 and
03 is its fixed-fixed address.

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

## Luminary099 AOSTASK_AND_AOSJOB

`'basic': 'full'`, cut at bank 20 `03010` and `03434`:

| Part | Basic words | Bytes |
|---|---|---|
| 1 | bank 20 to `03007` | 11,264 |
| 2 | bank 20 `03010`-`03433` | 11,046 |
| 3 | bank 20 from `03434` | 11,955 |

As one module it did not fit a 16 KB job slot.

## Comanche055 EXTENDED_VERBS

`'basic': 'full'`, cut at bank 43 `02600`. Chunked by routine, the file
took 32 modules of about 15.8 KB, because each one linked the whole
`basic_run`. With the flight entry, its two parts replace them.

## Luminary099 and Comanche055 INTERPRETER

`'basic': 'full'` with `'fixed_fixed_any_fb'`, cut at fixed-fixed `07000`:
part 1 is bank 00 and `06000`-`06777`, part 2 is `07000` up. Chunked by
routine, `lm_interpreter_basic` took 13 modules (212 KB) on the LM store
with Lane C's chunk packing, each with its own `basic_run` of about 10.7 KB.
As parts it is two modules, 12,745 and 10,465 bytes on LM. The flight
parts check needs the rope image the spec's target loads (DMP, POLY and
the others read their callers' words from it); `tools/flight_parts_check.py`
loads it whenever the spec's own target does.

## Store classification

`tools/flight_store_build.py` uses `interpretive_table()` everywhere it
used to test for `interpret` in a table's name. So `lm_interpreter_basic`
and `cm_interpreter_basic` (INTERPRETER.agc basic code) are linked and
chunked like other basic tables, rather than left whole in one module as
interpretive. Every compiled interpretive table except the DANZIG markers
goes to the interpreter (`to_interp`). Lane C's evidence `INTPRET` tables
are in `INTERP_ONLY`.

With those changes, both missions' stores fit every table except Lane C's
Pinball routines (`lanec_pin_routines`), which Lane C's chunk packing fits.

## Evidence

`tools/flight_parts_check.py <spec>` (`make flight-parts-check`) first runs
the file's differential (`tools/interp_routine_differential.py`), which
checks the whole build against the original program. It then builds every
part for the host and one runner with all the parts' tables, the way the
store's modules work together. Every case must end exactly as it does in
the whole build: Z, the 2048 erasable words and the interrupt state.

For P40-P47, all 31,744 cases (64 per routine) end the same. 1,597 of
them cross from one part to another at least once. For AOSTASK, all 704
cases end the same over its 3 parts. For CM EXTENDED_VERBS, all 11,648
cases end the same over its 2 parts.
