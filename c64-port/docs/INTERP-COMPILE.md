# Compiling a source file into native routines (`tools/interp_compile.py`)

Used by Lane D1b milestones (`inflight-align`, `geometry`, `kalcmanu-steering`, `r60-r62`, `attitude-maneuver`, ...), Lane D1 (`sband-antenna`) and Lane F (`lm-p40-p47`). Any lane can use it for files mixing interpretive and basic code. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## What it emits

`python3 tools/interp_compile.py <spec>` reads the yaYUL listings (`tools/interp_listing.py`) and a spec in `tools/interp_specs.py`. For each mission it writes `src/<base>.c` and `include/<base>.h`. Each entry, branch target, CALL/STCALL return, EXIT, TC INTPRET point and return after an outside TC becomes a routine of the control-transfer ABI:

- **Interpretive** routines are keyed by the physical LOC at NEWOPS. Each order pair compiles to the calls the interpreter itself makes, in order, with the pair's literal words:
  - NEWOPS or the store code;
  - the address or misc fetch;
  - the order body (`src/lm_interp_orders.c`, or the `cmi_` build for Comanche);
  - DANZIG.
- A fixed operand is a `K` array holding the rope words at its address, so nothing reads the rope at run time.
- **Basic** routines are keyed by the physical Z. Each instruction is one `agc_image_ops` call, decoded from the instruction word itself, with EXTEND and INDEX prefixes. Control flow inside the file is C `goto`. A TC into the file sets Q and continues at the target; its return point is a routine of its own. `TC Q` runs the word in Q as the hardware does: a TC to its address, leaving Q at 3. A transfer out of the file sets Z and ends the routine. A TS overflow skip skips one word, even when that word is an EXTEND.
- INDEX adds C(K) to the whole next instruction word, with the ones'-complement end carry, as yaAGC does. If that changes the instruction's order, the routine refuses.
- A `TC Q` whose Q is a return point of an in-file TC continues in the routine. When the file has INCR Q (TIMECHK), the two words after each such return are return points too.
- Fixed words that basic instructions read (`CAF`, `AD`, `MP`, `DCA` on constants) are recorded in the source map as constants.
- An outside call listed in the spec's `noreturn` owns its inline words (POSTJUMP's CADR) but has no return point.
- An outside call's inline words can be counted from the listing (`'auto'` in `externals`): the data words after the TC.
- Words nothing in the file reaches, such as the returns of a display or restart (GOPERF +1/+2), become routines of their own with `resume_orphans`. Words after an EXTEND or INDEX prefix are not.
- I/O channel instructions (extended opcode 0: READ, WRITE, RAND, WAND, ROR, WOR, RXOR) refuse with `EXEC_OUT_OF_SCOPE` until the ABI has a channel model. The source map records those words as untranslated, with the spec's `partial` reason.
- A CALL or STCALL followed by basic code has no return point. Its target is a saved return (`STCALL DELLT4` with `QTEMP1`).
- `CALRB` is CALL.
- A TC to an erasable address (TC MPAC +2, TC TBASE2, TC QSAVED) runs the word there with Q set to its return, as the hardware does. Switched erasable (01400-01777) is read through EB. The differential seeds the word with a caller return unless the spec pins it.
- Lane E keys (`r30`, `cm-r31`, `cm-tpi-search`, `cm-p30-p37`, `cm-p32-p33`, `cm-p34-p35`, `cm-p37-p70`, `lm-p32-p35`, `lm-p34-p35`):
  - `tc_tables`: an INDEX into a TC table (`DOTICK +7`), with the table's length. It becomes a C `switch` on the indexed word.
  - `fixed_index`: an indexed fixed operand (`DLOAD* 1/RTMUE,2`, `STODL* MUTABLE +2,1`), with the offsets the index register can take. Each reachable address is a `K` array, and any other effective address is refused. Banks 30-37 also refuse when the superbank bits are set.
  - `returns`: an outside call that returns to more than one word after its CADR (GOFLASH 3, GOFLASHR 4). Each of those words becomes a return point.
  - SLOAD of a fixed constant (DECTWO) runs the interpreter's inline load.
  - VPROJ is an address order.
  - A CALL or TC whose return lies past the end of the file, or on interpretive or data words, has no return point.
  - A spec may name one mission only (`'missions': ('cm',)`).
  - A shift's operand word is its count, never a fixed operand. SINE and COSINE compile as SIN and COS.
  - `fixed_index` also covers `SLOAD*` of a fixed table (ALMXIT's `ALARM/TB -1,1`).
  - `self_loops`: a branch back to the routine's own start loops inside the routine, up to 4,096 passes, instead of going through the run loop's 200-step limit.
  - `outside`: `{label: words}` leaves code the compiler can't model (VNPOOH's own FBANK switch) out of the routines; the source map marks it untranslated.
  - A CADR maps to superbank 40+ only when the routine using it is in bank 40 or above, so a file mixing bank 35 and bank 42 code keeps each bank's CADRs.
  - A basic instruction that reads Z (`CA Z`) sees the address after it.
  - The oracle also stops up to four words past the case's return (outside the file), so a skip return (`INCR Q`, `RETURN`) or an indexed return ends like a plain return.
  - `index_returns`: `INDEX K; TCF 0 +n` with K a saved Q (P37's SPRTEX, RTENCKEX) computes the TCF and lets the run loop go on from its address.
  - `max_steps`: raises the run loop's 200-routine limit for loops that span routines (P37's GAMDV and V2T iterations); the host runner takes it as `IRC_MAX_STEPS`, and the milestone's test and C64 diagnostic pass the same number to `irc_run`.
- An INDEX-modified write that could reach a bank or Z register checks the address at run time and refuses if it would.
- MXV and VXM read their 18-word matrix operand as a `K` array. LXC is an indexed misc order.
- VPROJ is an address order. BHIZ runs RTB/BHIZ's `CCS CYR` before its own test, as the original does.
- A TC into the file followed by data (`TC SBANDANT +2`, which never returns) has no return point.
- Files in banks 040-043 run as FB 030-033 under superbank 4 (channel 7 bit 0100). Their routines are keyed by that FB and refuse unless the superbank bit is set; a file in banks 030-037 refuses with it set. Fixed operands and CADR targets decode through the superbank. Files below bank 030 are not affected.
- `resume_keys` (off by default) is for files that run as jobs. The word after every DANZIG is a routine of its own. A DANZIG that finds NEWJOB positive stops at CHANG2, where the Executive suspends the job; it resumes at NEWOPS with LOC +1, which is that routine's key. Without it, every routine requires NEWJOB +0.
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

The states are random words, small magnitudes and unit vectors. A spec can pin EB (`ebank`, the file's `EBANK=`) and set erasable values at a routine (`states`, keyed by `LABEL` or `LABEL+n`) or at every routine (`all_states`, per mission if it is a dict). State keys are `vac+n`, an octal address or an erasable symbol from the listing (`TM+1`). `joint_states` gives sets of values that only mean something together (TM a little past TIME2/TIME1): one set per case, at a routine or, under `'*'`, in half of every routine's cases. With `counters`, TIME2, TIME1 (across its range, at 0 and just below its 037777 wrap) and CDUX-Z vary; otherwise `irc_build` leaves 010-061 at 0. With `edge_values`, a quarter of the cases put edge values into MPAC, the top of the pushdown list and every operand: ±0, one LSB, 1/2, 1/4, just below 1, half-unit axis vectors (a matrix's rows as a signed permutation), unit vectors in a coordinate plane and tiny vectors, with the CDUs at 0, 45, 90, 180 or 270°. These keys are all off unless a spec sets them, so other milestones' cases are unchanged. At a basic routine, every erasable word a TC runs holds the case's return. A routine in banks 040-043 runs with the superbank bit set. With `resume_keys`, NEWJOB is positive in a quarter of the cases, and CHANG2 is a stop. FIXLOC is a random VAC area, and PUSHLOC, MODE and OVFIND vary. Caller CADRs and Q returns point outside the file.

With `resume_flow` (and `resume_keys`), a resume LOC's cases start from the state the original program is in when it gets there, not random words: each case of the routine before it runs on the oracle (`IRC_SNAP`) to NEWOPS with LOC at the resume LOC, along a run of resume LOCs in turn, and NEWJOB is then set in a quarter of them. A resume LOC random states would reach with a MODE, index register or PUSHLOC the program never leaves there needs no pins this way. The differential prints how many resume LOCs it seeded; one no case reaches keeps random states.

`--freeze` writes:

- `tests/<id>_vectors.h`: every Nth case with its oracle digest, for the host and sanitizer tests. With `cover`, also the fewest cases (greedy) that reach every edge any case reaches: the native runner is rebuilt with `clang -fsanitize-coverage=trace-pc-guard` over every linked source but the test harness (`tools/irc_edges.c`), so each branch outcome is an edge, and each case reports the edges it reached.
- `tests/<id>_c64_vectors.h` and `tools/mk/<id>.groups`: with `cover`, the cases a greedy cover picks to reach every edge, among those whose run fits one diagnostic, plus the fewest-visit case of any routine it leaves out, packed case by case into groups of routines that fit below $c000 with the shared interpreter bodies (found by linking each candidate), and the cases whose whole run stays in a group.
- `build/<id>-proof.json`. Cases that stop at a channel instruction are listed as `waiting_for_io_channels` and not compared. Routines whose runs reach too much of the file to fit one diagnostic are listed as `c64_host_only`. With `cover`, `outcome_cover` gives the edges all cases, the host vectors and the C64 cases reach, and the source lines of any edge no C64 case reaches.
- `build/<id>-c64-cover.json` (with `'c64_rows': 'cover'`): the `c64_branch_outcomes` record a certificate cites. The C64 rows are the fewest cases that take every gcov branch arc the traced host cases take over every linked source; `cover_stride` traces every Nth case (a divisor of `freeze_stride`, so every frozen host vector is traced) to keep large files fast.
- `build/<id>-source-map.json`: each word of the file with the routine that compiles it. A word no routine compiles fails the freeze, unless the spec's `partial` entry names why.

## Additions from the orbital lane (ORBITAL_INTEGRATION, INTEGRATION_INITIALIZATION)

Spec keys:

- `cgoto`: the CADR list a CGOTO reads, with its length. The list's words are `cadr_list` rows in the source map.
- `indexed_fixed`: a starred operand into fixed memory, with the range its index register can take. It emits a `KX` table and checks the index at run time, refusing anything outside the table.
- `rtb` also serves BOVB to a basic label (BOVB TCDANZIG).
- `return_words`: erasable words that hold a saved interpretive return (TBASE2), seeded like QPRET with a return outside the file.
- `native_ranges`: basic routines implemented by hand behind RTB (the MOVE* state-vector moves), mapped in the source map to that function.
- `native_entries`: those hand-written routines as registered transfer routines too, for files that reach them directly (MEASUREMENT_INCORPORATION's RTB MOVEACSM). Each runs its function and then its TC DANZIG as a pair's end; a left-hand RTB (EDOP holding the pair's right order) is refused.
- `max_steps`: the routines one case may run (default 200). With `resume_keys` every pair is a routine, and one DIFEQ step runs several hundred.
- `c64_flags`: extra flags for the C64 probe and the milestone's PRGs. The large integration files use `-fno-inline -fstatic-stack -mlto-zp=110`, with the image under KERNAL at $e000.

`all_states` sets the legal values of index registers, PBODY, DIFEQCNT, COLREG and PUSHLOC for every case.

A target of `SSP QPRET <label>` (PHEXIT, AMOVED) is a return point and must be listed in `entries`.

`rope_bin` reads rope words from the `MAIN.agc.bin` beside the listing, so bank checksums and fillers that the listing omits are read correctly.

Both files run with `resume_keys`. `states` restores each mid-sequence invariant a CHANG2 resume point would otherwise lose: between ACCOMP's `XCHX,2 DIFEQCNT` pairs, X2 holds the DIFEQ stage and DIFEQCNT holds PBODY; S1 holds PBODY inside GAMCOMP; COLREG indexes TIMESTEP's W stores after a VLOAD.

A spec with `resume_keys` keeps the case's chosen NEWJOB after the operands are randomized. Before, an operand block next to NEWJOB could leave it negative.
