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

The interpreter extension (`INTERP_EXTENSION`, `--ext`) is 4 KB of RAM under
I/O at $D000-$DFFF (steward ruling, 2026-10-13). It is its own ABI 2
module, `$0A02`, of slot class 2 (`APOLLO_OV2_SLOT_FIXED`), which the
resident's boot loads once and verifies (`rk_load_fixed`):

- It holds the cold math bodies (`XT`: SQRT, UNIT, VSQ, SIN, COS,
  ASIN, ACOS with POLY, DDV, BDDV, DOT, VPROJ, VXSC, MXV, VXM, V/SC), linked
  by `platform/c64/interp_ext.ld`.
- None of these bodies reads the rope itself, since the REU registers are
  I/O. Their one rope read, an indexed fixed constant (`fixed_indexed`, in
  `$0A01`), banks I/O in around the read.
- They are reached only from the address and unary order tables. Every call
  through a table banks I/O out ($01 = $34) and back in. The resident's IRQ
  entry saves $01 and banks I/O in.
- `$0A01` and `$0A02` are one link. `interp_runtime_size.py` cuts the
  extension's section from the ELF as `interp-ext.prg` (24-byte header, an
  RTS entry nothing calls, no BSS) and checks both PRGs with the store
  packer's `parse_module`.
- With it, the module fits: LM 16,029 and CM 16,063 of 16,384 bytes, and
  `$0A02` takes 4,038 and 4,029 of 4,096 bytes with its header.
- Without it, the module is 3.7 KB over.
- The flight store packs both (`tools/flight_store_build.py`
  `INTERP_POINTS`): the resolver sends each entry point below to `$0A01`,
  except SWRETURN (BANKCALL's return, which a basic table holds).

Every entry point is a place where a resident step can hand a job to the
module, so each one enters from the AGC's own state there, as yaAGC has it:

- NEWOPS, DANZIG, NOIBNKSW, GOTO and INTPRET need nothing beyond the
  interpretive registers.
- DODLOAD, DOVLOAD, DODLOAD* and DOVLOAD* set CYR themselves.
- DIRADRES takes the INDJUMP index from CYR (20000 + index, as DODLOAD,
  DOVLOAD or ADDRESS left it), and 15BITADR takes the MISCJUMP index and
  the code's fourth bit from CYR (the code cycled right three places, or
  STOTC's CALLCODE cycled once). Any other CYR is refused.
- A NOVAC job's FIXLOC is FAKEPRET (MPAC -36D, so QPRET is MPAC +6); it is
  accepted beside the five VAC areas.
- GOTO through a cycle of erasable CADRs (GOTOERS, "arbitrary indirectness")
  never reaches fixed memory; the AGC loops there until the night watchman
  restarts it. After 64 hops the module hands the job back at GOTO with the
  current POLISH, so the next step goes on looping and interrupts still run.
- An INDEX'd fixed constant (the shift orders' BIT15/BIT14 tables) is the
  rope word at the indexed address, wherever the index lands (#224).

`make vice-interp-core` runs the flight build on a PAL C64 in VICE
(`tools/interp_core_vice_test.py`, `tools/interp_core_vice.c`):

- The build has the resident's k_ ops and the extension. The test puts
  module `$0A02` in the REU, and the harness loads and verifies it at $D000
  by the resident's boot DMA (registers set with I/O in, then a write to
  $FF00 with $01 = $34). With the module misplaced in the REU the run
  crashes, so the code at $D000 is the DMA's.
- The rope is in the REU at its flight address.
- The CIA timer IRQ runs throughout, through a RAM vector that saves $01.
- It runs 180 fuzz slot records per mission and checks each 6502 result
  against the host's verification interpreter.

`make interp-flight-vice` runs flight-scenario interpretive code through
the flight resident in VICE (`tools/interp_flight_test.py`,
`platform/c64/interp_flight_check.c`). It needs `make flight-store`.

- `tools/interp_flight_capture.c` (host, links yaAGC) runs LM and CM P00
  from Fresh Start for 30 s with V37E00E, as `tools/flight_host_run.c`
  runs its reference. Each first arrival at an entry point the store sends
  to `$0A01` (distinct entry, LOC and BANKSET) is a case: the erasable image
  and the superbank. These are the fresh-start NOVAC jobs and P00's state
  vector integration.
- The C64 program boots like the flight resident on the store, with the
  cases streamed into the REU above it. For each case it builds the image
  at $7000 and runs `rk_resident_step` while Z stays at such an entry, with
  interrupts off. The store's resolver, loader and trampoline all take part.
- Each case's exit Z, FB and a digest of the image go back to the host,
  where yaAGC runs from the case (interrupts held off) and must reach that
  Z under that FB with that digest.
- On the merged base (a376a718) both missions pass: 240 cases each (of
  504 LM and 509 CM, evenly spread), every entry but DODLOAD* and DOVLOAD*
  reached, every exit state reached by yaAGC. The rows are in
  `evidence/interpreter/interp-flight-vice.json`.
- `tools/interp_flight_replay.c` replays one case slot by slot on the host
  with the same core, each slot checked against yaAGC: it found the
  DIRADRES, 15BITADR and NOVAC entries above.

`make interp-flight-flowed-vice` runs the same check on flowed states.
These are not mission flight. They cover where the flight store stops a
FLIGHT_INTERP_STOP module (`tools/flight_store_build.py`: AGS_INITIALIZATION,
BURN_BABY_BURN, THE_LUNAR_LANDING, SERVICER, IMU_PERFORMANCE_TESTS) because
its code reaches interpretive code and the interpreter takes the job.

- `tools/interp_flight_flowed.c` (host, links yaAGC and the milestone's
  native sources) runs yaAGC from the first 128 states of each routine of
  the milestone's cases, as its `tools/<stem>_reference.c` does, until the
  run leaves the file. Each first arrival at an entry point from code
  outside the interpreter's banks (00, 01, 03), at an order inside the file
  (at INTPRET, Q), is a case. A yaAGC look-ahead prefers a state whose run
  keeps a real job's frame (FIXLOC, the push-down pointer, MODE, superbank
  0 to 0100).
- `tools/interp_flight_verify.c` runs the verification interpreter from
  each case. A case it refuses, or whose result yaAGC does not reach, runs
  outside the contract above and is left out (counted in the report).
- The rest run in VICE, as many per boot as the REU stream holds (a case
  that shares few words with the one before goes whole).
- With superbank 1-3 accepted (below): 415 cases captured, 12 refused, 7
  unreached, 396 run in 7 boots (477 resident steps), every exit state
  reached by yaAGC. SERVICER keeps 147 of its 171 points,
  IMU_PERFORMANCE_TESTS 133 of 133, THE_LUNAR_LANDING 74 of 74,
  AGS_INITIALIZATION 26 of 26 and BURN_BABY_BURN 16 of 16; the rest have no
  state inside the contract. The rows are in
  `evidence/interpreter/interp-flight-flowed-vice.json`.

`make interp-flight-p63-vice` runs the same check on mission flight:
Apollo 11 about ten minutes before PDI, from Virtual AGC's NASSP core dump
(`tools/scenarios/README.md`).

- `tools/interp_flight_capture.c` loads the core and keys V37E63E. P63's
  ignition-time iteration ends at V06N61. It then keys PRO (V33E) and ENTR
  through the V50N25 and V50N18 checklist to V06N62 and the countdown, with
  Average G and SERVICER starting at TIG-35 s. That is 70,000 cs and 59.7
  million instructions, with 4,215 distinct first arrivals at entry points.
  1,378 of them run under superbank 3 (channel 7 060), the value flight
  keeps in SUPERBNK.
- 1000 of those arrivals, evenly spread, run in 5 boots (2,204 resident
  steps). Every exit state is reached by yaAGC. They take every entry
  point but DOVLOADX. The rows are in
  `evidence/interpreter/interp-flight-p63-vice.json`.
- The run found two defects, both now fixed:
  - Superbank 1-3: the core, the verification interpreter and the
    INTERPRETER subroutines' rope reads refused superbank 020-060 for
    banks 030-037. The hardware and yaAGC move only bit 0100 (to banks
    040-043), so every P63 stop in banks 32, 33 and 37 was refused. Values
    0 to 060 now read banks 030-037; 0100 reads 040-043; 0120 and above are
    still refused. Issue #321 is the same defect in compiled routines'
    guards.
  - The DV oracle: the stock yaAGC engine's fast DV reads an L of -0 as
    -2^14 (`docs/DIVIDE.md`). UNIT meets that in flight, when a vector
    component's DCS of +0 leaves L at -0. The capture now links the
    hardware-DV engine (`tools/prepare_hardware_dv.py`), as the other
    oracles do.

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

### Contract: erasable addresses stay inside erasable

Both interpreters keep every erasable operand inside the image:
- `ai_ea` masks an address to 01777. So an operand that an index or the push-down pointer runs past 01777 wraps into low erasable.
- The original reads fixed memory there, under FB, and its writes there do nothing.
- A wild indexed STORE address wraps the same way, through the 10-bit erasable field (`src/agc_image_ops.c` `ai_ea`, `src/lm_interp_steps.c` `ea`). The AGC writes elsewhere.
- STORE,1, STORE,2 or STADR can index the address negative (#422). STORE's CCS then takes the work-area branch, and FIXLOC plus that address lies above 01777. There, `INDEX ADDRWD` turns the DXCH into another instruction, so the AGC stores nothing. The compact core wraps that address like any other; adding a check would cost about 180 bytes of a slot with about 300 left. The runtime interpreter refuses it (`store_outside`). `tools/interp_runtime_target.c` refuses such a STORE at NEWOPS for both, before the routine runs, and the differential counts those cases without comparing them.

The core's contract is that indexed and push-down erasable addresses stay inside erasable. Every flight path keeps them there. INCOR1 and INCOR2 set X1 and X2 by AXT (at most 162D, stepping down to the S1 or S2 step), so every W+k,X operand stays inside W. FAZB1's WIXA is 6 to 54.

`make interp-core-differential` (`evidence/interpreter/interp-core-differential.log`) runs 86,472 cases over every compiled milestone, and none differ. PR CI runs it (`ci-extra-host`) when the interpreter's own files change (`ci-watch-interp-core-differential`).
- A case that enters a cycle of erasable CADRs is not compared (one lm-p40-p47 case at 27,3042 did, on earlier case sets): the oracle loops in GOTOERS until its step limit, as the AGC does until the night watchman restarts it. The core hands the job back at GOTO after 64 hops (#224). The tool counts such cases without comparing them, as the routine differential does when both sides loop.
- Since #308, a CM program with no agc_hardware runs as a copy that carries one (`tests/interp_case.c` `irc_cm_begin`). `tools/interp_runtime_target.c` therefore gives its program one: its stops find the mission by the program they are handed, and its routines read the case's channels there.
- measurement-inc reads 0 of 2,464 since D2's flight-range states (#393). Before that, 14 FAZB1 cases started from WIXA and X1 values far outside flight and indexed past erasable.

The other 113 differences that measurement-inc showed before #366 came from the tool. It called `flow_seed` without the spec, so the spec's `flow_from` entries were ignored.
