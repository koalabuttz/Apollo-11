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

Today the order bodies are the image-level ones the compiled routines call
(`src/lm_interp_steps.c`, `src/lm_interp_orders.c`), which are verified against
yaAGC body by body. They are about 54 KB on the C64; the compact rewrite for one
16 KB slot is the lane's next milestone.

## Tests

- `make interp-runtime-differential`: for every compiled-routine milestone in
  `tools/interp_specs.py`, the same cases, oracle and stops as
  `tools/interp_routine_differential.py`, with the file's interpretive points
  run by the interpreter and its basic routines and the spec's native RTB
  routines registered as basic routines. A case passes when Z, the 2048 words
  and the interrupt-enable state equal yaAGC's at the stop.
- `make interp-runtime-size`: each mission's interpreter linked as one ABI 2
  job-slot module, id `$0A01` (lane I), with the flight store's flags.
