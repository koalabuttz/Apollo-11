# Runtime interpreter (INTERPRETER.agc)

Interpreter lane. Decision 2026-10-13 (steward proposal
`interpreter-proposal.md`): the C64 flight image runs the AGC's interpretive
programs with a native port of INTERPRETER.agc, reading the order words from
the rope image in the REU as the AGC read them from its rope. Basic code in
every file stays native C. Compiled interpretive routines
(`tools/interp_compile.py`) stay: they are the host oracle, the C64 fallback for
profiled hot spots, and the code of the planned fully compiled x86_64 build.

## Hook

A program registers compiled routines as before. The interpreter takes every
interpretive point no compiled routine is registered for:

- `lm_interp_runtime` / `cm_interp_runtime` (`include/interp_runtime.h`) is the
  routine for an interpretive point: Z at NEWOPS with LOC under BANKSET, or a
  pending DANZIG.
- `lm_interp_runtime_basic` / `cm_interp_runtime_basic` hold the interpreter's
  own basic entries: SWRETURN, INTPRET and DANZIG.

One call runs one NEWOPS slot (one order pair, or one store code and its load)
and returns with Z at the next NEWOPS, or at the first point outside the
interpreter: EXIT's basic code, an RTB routine, CHANG2, an alarm. RTB and CALL
into basic code leave Z (and FB) at the target; the resolver finds the basic
routine there, and its return through SWRETURN or TCF DANZIG comes back to the
interpreter. Returning after every slot keeps interrupt latency at one order
pair.

## How it decodes

As INTERPRETER.agc does. A positive word at LOC is a store code (DOSTORE). A
negative word is an order pair: CCS gives the left code in bits 1-7 and the
right code, plus one, in EDOP. For a code c:

| c | Class | Table index |
|---|---|---|
| 0 | EXIT | |
| bit 1 set | address order (INDJUMP); bit 2 set: indexed | c >> 2 |
| bit 2 set | index, branch and miscellaneous (MISCJUMP) | c >> 3; bit 3 selects X1/X2 or the second order of the entry |
| bit 3 set | short shift (SHORTT/SHORTV by MODE) | |
| otherwise | unary (UNAJUMP) | (c >> 3) - 1 |

The jump-table body PCs and the interpreter's addresses for each mission are
generated from the listings into `include/interp_runtime_tables.h`
(`tools/interp_runtime_generate.py`). Operand words come from the rope at LOC; a
fixed operand (ADDRWD in 02000-03777 after address formation) is read from the
rope at FB/ADDRWD, so indexed fixed tables need no per-file data.

The verification interpreter's order bodies are the image-level ones the
compiled routines call (`src/lm_interp_steps.c`, `src/lm_interp_orders.c`),
verified against yaAGC body by body. They are about 54 KB on the C64.

## Compact interpreter (flight build)

`lm_interp_core` / `cm_interp_core` (`src/lm_interp_core.c`; the CM unit
includes it with `INTERP_COMANCHE`) are the same interpreter in one
translation unit for the C64 job slot:

- One image: with `AGC_FIXED_IMAGE` on llvm-mos the bodies address the
  canonical image at $7000 directly, and the entry refuses any other.
- Its own instruction helpers (`ai_*`), with one-byte entries for constant
  register addresses and small constants, chosen at compile time. The
  flight module builds with `INTERP_K_OPS`: the ops the resident kernel
  exports as ABI 2 services (`k_ca` ... `k_bzmf`, docs/FLIGHT-STORE.md)
  come from the resident, and only the rest are linked in.
- Table dispatch for the address, miscellaneous and unary orders.
- Its own module entry, `platform/c64/interp_module.c`. It runs slots until Z
  leaves the interpreter, so it needs no transfer-ABI table wrapper.

It must equal the verification interpreter word for word:

- `make interp-core-fuzz` runs random interpretive programs in a bank of the
  rope against both interpreters and compares every NEWOPS slot: Z, all 2048
  words, the status and the io record. Inputs no program produces are left
  out and counted (an EBANK window over the registers, an index register or
  store run wild, MODE outside -1..+1).
- The fuzz runs a second build with `INTERP_K_OPS`, in which
  `tools/interp_core_kops.c` stands in for the resident's ops
  (agc_image_ops on the bound image).
- `make interp-core-differential` runs the yaAGC differential above with the
  compact interpreter.

`make interp-runtime-size` links it as module `$0A01` (16 KB job slot target);
`--runtime` links the verification interpreter instead.

`make interp-core-cycles` measures 6502 cycles per order in llvm-mos's
simulator (`tools/interp_core_cycles.c`, flight flags, image at $7000):

- It takes one order per slot from the fuzz.
- It checks every slot's 6502 result against the host's.
- It adds the measured REU rope read cost.
- Results go to `build/interp-cycles/<m>.json`.

`tools/interp_order_rates.py` counts the orders yaAGC runs in the flight
scenarios (`tools/interp_order_trace.c`). It counts one per pass through the
INDJUMP, MISCJUMP and UNAJUMP tables. It also prices a set of orders kept in
a second module at 30K cycles a load. From a fresh start these scenarios run
almost no interpretive code (0-8 orders a second).

## Tests

- `make interp-runtime-differential`: for every compiled-routine milestone in
  `tools/interp_specs.py`, the same cases, oracle and stops as
  `tools/interp_routine_differential.py`, with the file's interpretive points
  run by the interpreter and its basic routines and the spec's native RTB
  routines registered as basic routines. A case passes when Z, the 2048 words
  and the interrupt-enable state equal yaAGC's at the stop.
- `make interp-runtime-size`: each mission's interpreter linked as one ABI 2
  job-slot module, id `$0A01` (lane I), with the flight store's flags.
