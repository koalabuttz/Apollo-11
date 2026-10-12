# Compiling a source file into native routines (`tools/interp_compile.py`)

Used by Lane D1b milestones (`inflight-align`, `geometry`, ...). Any lane can use it for files mixing interpretive and basic code. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## What it emits

`python3 tools/interp_compile.py <spec>` reads the yaYUL listings (`tools/interp_listing.py`) and a spec in `tools/interp_specs.py`. For each mission it writes `src/<base>.c` and `include/<base>.h`. Each entry, branch target, CALL/STCALL return, EXIT, TC INTPRET point and return after an outside TC becomes a routine of the control-transfer ABI:

- **Interpretive** routines are keyed by the physical LOC at NEWOPS. Each order pair compiles to the calls the interpreter itself makes, in order, with the pair's literal words:
  - NEWOPS or the store code;
  - the address or misc fetch;
  - the order body (`src/lm_interp_orders.c`, or the `cmi_` build for Comanche);
  - DANZIG.
- A fixed operand is a `K` array holding the rope words at its address, so nothing reads the rope at run time.
- **Basic** routines are keyed by the physical Z. Each instruction is one `agc_image_ops` call, decoded from the instruction word itself, with EXTEND and INDEX prefixes. Control flow inside the file is C `goto`. A TC into the file sets Q and continues at the target; its return point is a routine of its own. `TC Q` runs the word in Q as the hardware does: a TC to its address, leaving Q at 3. A transfer out of the file sets Z and ends the routine. A TS overflow skip skips one word, even when that word is an EXTEND.
- RTB to a routine with a native implementation (`rtb` in the spec, such as `rtb_angles`) calls it. BOVB's branch to basic code ends the routine at that address.
- Every routine first checks that the image is one the original could be in. Anything else is `EXEC_OUT_OF_SCOPE` with the image unchanged.
- `--check` fails when the generated files differ from what the listings give.
- `<SYM>_KEEP<n>` masks keep only some routines. The C64 diagnostics use them.

## How it is checked

`python3 tools/interp_routine_differential.py <spec> --cases 512 [--freeze]` runs the unmodified ropes in yaAGC against the native routines. The oracle (`tools/interp_routine_reference.c`) uses the pinned engine with DV always on its hardware model (`tools/prepare_hardware_dv.py`). Each routine runs from 512 states to its stop, and the runs must agree in Z, all 2,048 erasable words and the interrupt-enable state. A stop is one of:

- NEWOPS with LOC outside the file;
- an exit of the interpreter bodies (POODOO/ALARM);
- a transfer from the file to an outside basic routine;
- the case's own caller return.

The states are random words, small magnitudes and unit vectors. FIXLOC is a random VAC area, and PUSHLOC, MODE and OVFIND vary. Caller CADRs and Q returns point outside the file.

`--freeze` writes:

- `tests/<id>_vectors.h`: every Nth case with its oracle digest, for the host and sanitizer tests.
- `tests/<id>_c64_vectors.h` and `tools/mk/<id>.groups`: groups of routines that fit below $c000 with the shared interpreter bodies (found by linking each candidate), and the cases whose whole run stays in a group.
- `build/<id>-proof.json`.
- `build/<id>-source-map.json`: each word of the file with the routine that compiles it. A word no routine compiles fails the freeze, unless the spec's `partial` entry names why.
