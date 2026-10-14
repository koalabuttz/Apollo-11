# Compiling a source file into native routines (`tools/interp_compile.py`)

Used by Lane D1b milestones (`inflight-align`, `geometry`, `kalcmanu-steering`, `r60-r62`, `attitude-maneuver`, ...), Lane D1 (`sband-antenna`) and Lane F (`lm-p40-p47`, `lm-r63`, `lm-p12`, `lm-p70-p71`). Any lane can use it for files mixing interpretive and basic code. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

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
- Words nothing in the file reaches, such as the returns of a display or restart (GOPERF +1/+2), become routines of their own with `resume_orphans`. Words after an EXTEND or INDEX prefix are not. With `resume_orphans: 'all'`, so is an order word nothing reaches (the EXIT after LITTLSUB's GOTO QPLAC), entered at NEWOPS.
- I/O channel instructions (extended opcode 0: READ, WRITE, RAND, WAND, ROR, WOR, RXOR) refuse with `EXEC_OUT_OF_SCOPE` until the ABI has a channel model. The source map records those words as untranslated, with the spec's `partial` reason.
- With `channel_inputs`, READ, RAND, ROR and RXOR of input channels 3, 4 (the scalers) and 030-033 compile. They read the inputs the caller latches for the run through `AGC_CHANNEL(n)` (`include/agc_input_channels.h`: the mission's io record, then the inputs), as yaAGC does: A overflow-corrected, combined, sign-extended. A scaler does not move during a run. Channel 016 (NAVKEYIN, the mark buttons) is latched the same way. Channels 1 and 2 are L and Q: READ, WRITE, RAND, WAND, ROR, WOR and RXOR of them compile as 16-bit register operations, as in yaAGC. Writes to other channels still refuse. This is interim: `AGC_CHANNEL` moves to `agc_hardware` when the Kernel lane adds the channel fields there.
- `idle` names spin points that wait for hardware to move an input (CHECKG1, FINETIME +1). Reaching one ends the run there with `EXEC_IDLE`, and the point is a routine that runs the wait once more.
- `erasable_calls` names erasable addresses the file transfers to (TC LAT(SPL), the erasable program of the IMU performance test). The run ends there.
- `index_guard` refuses an indexed erasable operand that leaves erasable memory. A starred operand with a fixed base reads the rope words its index can reach (a `KX` window). One with an erasable base that indexes into fixed memory is refused.
- With `'channels': 'all'` on a Luminary file (it also needs `channel_inputs`), every channel instruction on a modelled channel compiles, in interpretive files and `'basic': 'full'` ones alike, as a call to `agc_ch_op` (`include/agc_channel_ops.h`, the one channel model): output channels 5, 6, 011, 012, 013 and 014 are the program's `agc_hardware` latches (the Kernel lane's field names); 3, 4, 016 and 030-033 are the run's latched inputs; channel 7 is the run's superbank. Channel 013's bits 12-14 arm the HANDRUPT traps, as yaAGC's: they stay set in `hardware->channel13` and read back 0. The routine refuses at run time, changing nothing, on a write that would change the superbank or store to 3, 4 or 016, and when the program has no `hardware`. Channels 010 and 015 are not modelled yet and still refuse. The wrappers set `agc_channels_hw` from `program->hardware`, so the target gives its program one (`static agc_hardware hw;` ... `&hw`). Such a spec compares channels by default (`io_channels` `'auto'`, below). Comanche `'basic': 'full'` files keep their own `'channels'` (the CM io record).
- A SLOAD of a fixed word, a TC to switched erasable, and an indexed TC into a table of up to eight TCs (INDEX NDXCTR, TC +1) compile. A channel instruction reached without its EXTEND (a TS overflow skip) is refused.
- A spec can name one mission (`missions: ('cm',)`).
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
  - Offsets count from the operand as written (`XUNIT -6,2` with X2 -6, -4 or -2 is `{'XUNIT': [2, 4, 6]}`). An `SLOAD*` into fixed memory without an entry is a compile error: its indexed path reads erasable.
  - `any_fixloc`: a basic routine that never names FIXLOC and never enters the interpreter checks the image without the VAC-area test on FIXLOC (`sane_nv`), and half of its differential cases put FIXLOC anywhere in erasable. For routines that tasks and NOVAC jobs call (STOPRATE, the RTB bodies).
  - `any_newjob` (with `any_fixloc`): such a routine that also never names NEWJOB takes NEWJOB -0 as well, which DUMMYJOB leaves while the computer idles, and a quarter of its cases set it. For routines a T4RUPT or T3RUPT task runs whatever the job state (RCSMON).
  - `self_loops`: a branch back to the routine's own start loops inside the routine, up to 4,096 passes, instead of going through the run loop's 200-step limit.
  - `outside`: `{label: words}` leaves code the compiler can't model (VNPOOH's own FBANK switch) out of the routines; the source map marks it untranslated.
  - A CADR maps to superbank 40+ only when the routine using it is in bank 40 or above, so a file mixing bank 35 and bank 42 code keeps each bank's CADRs.
  - A basic instruction that reads Z (`CA Z`) sees the address after it.
  - The oracle also stops up to four words past the case's return (outside the file), so a skip return (`INCR Q`, `RETURN`) or an indexed return ends like a plain return.
  - `index_carry` (`{'bank,addr': word}`): an indexed instruction whose index carries it into another quarter code compiles as the instruction it becomes (the opcode and quarter code given), its address taken from the sum; any other sum refuses. AOTMARK's `INDEX MARKSTAT; TS 0 -1` assembles as DXCH 1777 and runs as TS VACnUSE.
  - With `'basic': 'full'`, `channel_inputs` reads the latched input channels (3, 4, 016, 030-033) as above, and an indexed MSU runs on the summed address when it stays an ordinary erasable word (SXTMARK's DIFCHK).
  - `index_returns`: `INDEX K; TCF 0 +n` with K a saved Q (P37's SPRTEX, RTENCKEX) computes the TCF and lets the run loop go on from its address.
  - `max_steps`: raises the run loop's 200-routine limit for loops that span routines (P37's GAMDV and V2T iterations); the host runner takes it as `IRC_MAX_STEPS`, and the milestone's test and C64 diagnostic pass the same number to `irc_run`.
- An INDEX-modified write that could reach a bank or Z register checks the address at run time and refuses if it would.
- MXV and VXM read their 18-word matrix operand as a `K` array. LXC is an indexed misc order.
- VPROJ is an address order. BHIZ runs RTB/BHIZ's `CCS CYR` before its own test, as the original does.
- A TC into the file followed by data (`TC SBANDANT +2`, which never returns) has no return point.
- Files in banks 040-043 run as FB 030-033 under superbank 4 (channel 7 bit 0100). Their routines are keyed by that FB and refuse unless the superbank bit is set; a file in banks 030-037 refuses with it set. Fixed operands and CADR targets decode through the superbank. Files below bank 030 are not affected.
- `image_w` (off by default): the file takes `W` from `include/agc_image_w.h`, so flight builds (`RK_FIXED_IMAGE`) address the fixed $7000 image directly, about a fifth smaller on the C64. Host and diagnostic builds are unchanged. Turn it on at the spec's next refreeze; existing files stay byte-identical until then, so their pinned evidence does too.
- `fast_window` (off by default; `'basic': 'full'` in one function, so not with `basic_split` or `basic_parts`): the file's orders go through `include/agc_fast_redirect.h` with `AFR_EBN` set to the spec's `ebank`. A constant operand in the EBANK 6 window (01400-01776) passes a one-byte offset to a noinline `afw_*` helper (`include/agc_fast_window.h`, Lane F's #278 spill pattern), any other plain cell its address to `afo_*`, and anything else goes through the `ai_*` helper; `W` is `AFR_S(s)->words` and each ABI wrapper calls `AFR_BIND(s)`. Link `src/agc_fast_ops.c` and `src/agc_fast_window.c` (`native_sources` and the milestone's mk); `-DAFR_GENERIC` builds the plain `agc_image_ops` form. It replaces `image_w` (`AFR_S` is the fixed image in flight builds). Files whose operands mostly sit in an EB6 window gain most, above all the DAP's per-cycle set: LM KALMAN_FILTER (RATELOOP) went from 2,807 to 2,208 bytes on the flight image. Set it at the spec's next refreeze.
- `trusted_image` (off by default): the entry checks (`sane`, `sane_nv`, `basic_sane`) skip their scan of the whole erasable image for words over 15 bits in flight builds (`#if !RK_FIXED_IMAGE`). That scan costs about 121,000 6502 cycles a call at -Oz (mos-sim), more than a 100 ms PAL DAP pass. In flight the image holds only words the native code wrote, and a refusal there would only stall. Host tests and C64 diagnostics keep the scan, so the evidence is unchanged; the flight store is the only `RK_FIXED_IMAGE` build of generated files. LM KALMAN_FILTER (RATELOOP, run every DAP pass) sets it.
- `short_index` (off by default; `'basic': 'full'` files): an INDEX and its indexed order form the address in one noinline `ixe(word, base, limit)` (-1 outside the range), and the basic code's `ix`/`e` locals are `int`, not `long`. Every value they hold fits 16 bits (a 12-bit address plus a 15-bit signed index, or a 15-bit register word). The 6502 code shrinks by about a third per indexed order: LM KALMAN_FILTER's RATELOOP flight module went from 3,597 to 3,097 bytes. The generated code changes, so set it at the spec's next refreeze.
- `superbank_bit` (off by default): an indexed read of a fixed operand in FB 30-37 refuses only when the superbank bit (channel 7 bit 0100, which selects banks 40-43) is set, and refuses once at the routine's entry, before any order has written state. Without it the read tests `superbank&0160` after the earlier orders have run, so the legal values 020, 040 and 060 refuse with state already changed (issue #321). Only specs with such a read change: today lm-p32-p35, cm-p30-p37, cm-p32-p33 and cm-p34-p35. Set it at the next refreeze.
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

Some cases loop on both sides (#224). An example is an erasable-CADR cycle in the interpreter's GOTO, which the AGC runs until the night watchman restarts it. In those cases the oracle reaches its step limit (kind 9) and the native `lmi_goto_any` calls `exit(3)`. The host runners link `-Wl,--wrap=exit` with `-DIRC_WRAP_EXIT`, so `tools/interp_routine_native.c` ends that one case as a kind-9 row and the run goes on. The differential counts these cases ("both sides loop") and leaves them out of the frozen sets. A case where only one side loops is a mismatch. `IRC_SALT` (environment) appends to the case seed, for audits with other case sets than the frozen one.

A native stop must be one of those stops (#325). Suppose a run ends because no routine covers Z (`EXEC_UNMAPPED`, which `irc_run` reports as a stop), and Z is a word of the file itself, placed in its bank by the FB the run ends under. That is not the program's stop, because yaAGC runs on from there. The differential fails on it, except at the case's own stop and at the spec's `idle` points, where both sides stop by design. `--stops-only` runs just the native side and prints these stops by routine; the native runner's `fb` mode prints the final FB.

One kind of case is refused rather than compared. The interpreter's GOTO can land on an operand word of the file (nav1 at 25,3671 lands on 24,3715). The native run then stops at NEWOPS with LOC on a word that is no order start, so no routine covers it, while yaAGC decodes the operand as orders. The differential leaves such cases out of the run and the frozen sets and prints their count ("refused (GOTO to an operand word of the file)").

The states are random words, small magnitudes and unit vectors. A spec can pin EB (`ebank`, the file's `EBANK=`) and set erasable values at a routine (`states`, keyed by `LABEL` or `LABEL+n`) or at every routine (`all_states`, per mission if it is a dict). `states`, `all_states` and `joint_states` may instead be split by mission (`{'lm': {...}, 'cm': {...}}`), for labels that exist in both programs at different code. State keys are `vac+n`, an octal address or an erasable symbol from the listing (`TM+1`). With `index_states`, each routine whose next indexed operand (in listing order, following CALL and GOTO into the file, before X1 or X2 is set) names erasable gets X1 or X2 as base minus a target across the switched bank, so the operand lands in erasable as the program's AXC/AXT settings do; explicit `states` win. `joint_states` gives sets of values that only mean something together (TM a little past TIME2/TIME1): one set per case, at a routine or, under `'*'`, in half of every routine's cases. With `counters`, TIME2, TIME1 (across its range, at 0 and just below its 037777 wrap) and CDUX-Z vary; otherwise `irc_build` leaves 010-061 at 0. With `edge_values`, a quarter of the cases put edge values into MPAC, the top of the pushdown list and every operand: ±0, one LSB, 1/2, 1/4, just below 1, half-unit axis vectors (a matrix's rows as a signed permutation), unit vectors in a coordinate plane and tiny vectors, with the CDUs at 0, 45, 90, 180 or 270°. These keys are all off unless a spec sets them, so other milestones' cases are unchanged. At a basic routine, every erasable word a TC runs holds the case's return. A routine in banks 040-043 runs with the superbank bit set. With `resume_keys`, NEWJOB is positive in a quarter of the cases, and CHANG2 is a stop. FIXLOC is a random VAC area, and PUSHLOC, MODE and OVFIND vary. Caller CADRs and Q returns point outside the file. `range_states` sets values at every routine from one label up to another (a loop's index registers, say). A state value `'outside'` is a caller return outside the file. With `channel_inputs`, each case latches input channels drawn from its seed (`irc_inputs`), LOSCALAR 37777 in a quarter of them, and the oracle sets yaAGC's input channels to the same values. With `io_channels` (`'auto'`: every compared channel the file's orders name), each case also sets those channels (`IRC_CHANNEL` edits) on both sides, and its digest covers channels 5, 6, 011-014 and 030-033 and the trap flags after the run; on Luminary the outputs are the program's `agc_hardware` (`irc_lm_io`), channel 013 seeded without trap bits and the traps read from its bits 12-14.

With `resume_flow` (and `resume_keys`), a resume LOC's cases start from the state the original program is in when it gets there, not random words: each case of the routine before it runs on the oracle (`IRC_SNAP`) to NEWOPS with LOC at the resume LOC, along a run of resume LOCs in turn, and NEWJOB is then set in a quarter of them. A resume LOC random states would reach with a MODE, index register or PUSHLOC the program never leaves there needs no pins this way. The differential prints how many resume LOCs it seeded; one no case reaches keeps random states. `'resume_states'` {`'b,aaaa'`: {octal address: [values or `'flow'`]}} then sets words of a resume LOC's flowed state, one choice per address per case (`'flow'` keeps what the run reached). This widens the domain to reach outcomes the flow seldom gives: cm-orbital-integration zeroes MPAC's high word in half its 11,2736 (BDDV) resumes, which gives divide-overflow states flight never reaches.

`--freeze` writes:

- `tests/<id>_vectors.h`: every Nth case with its oracle digest, for the host and sanitizer tests. With `cover`, also the fewest cases (greedy) that reach every edge any case reaches: the native runner is rebuilt with `clang -fsanitize-coverage=trace-pc-guard` over every linked source but the test harness (`tools/irc_edges.c`), so each branch outcome is an edge, and each case reports the edges it reached.
- `tests/<id>_c64_vectors.h` and `tools/mk/<id>.groups`: with `cover`, the cases a greedy cover picks to reach every edge, among those whose run fits one diagnostic, plus the fewest-visit case of any routine it leaves out, packed case by case into groups of routines that fit below $c000 with the shared interpreter bodies (found by linking each candidate), and the cases whose whole run stays in a group.
- With `c64_segments` (and `cover`), an edge that only runs too long for one diagnostic reach, in a routine just before a CHANG2 resume LOC, gets a segment row. The row runs its case from that routine to the resume LOC. The next routine is left out of the group's KEEP mask, so the run stops there, at the same NEWOPS where the oracle stops (`IRC_SNAPROW`: `IRC_SNAP`'s stop LOC, with the usual digest line). Each segment first runs on the host with the same KEEP mask, and the freeze fails unless it equals the oracle. Its edges come from a KEEP-mask build of the edge runner, matched to the full build's by source line and column. The proof lists the rows as `c64_segments`, and each C64 group's `case_ids` names the differential case of each row.
- `build/<id>-proof.json`. Cases that stop at a channel instruction are listed as `waiting_for_io_channels` and not compared. Routines whose runs reach too much of the file to fit one diagnostic are listed as `c64_host_only`. With `cover`, `outcome_cover` gives the edges all cases, the host vectors and the C64 cases reach, and the source lines of any edge no C64 case reaches.
- `build/<id>-c64-cover.json` (with `'c64_rows': 'cover'`): the `c64_branch_outcomes` record a certificate cites. The C64 rows are the fewest cases that take every gcov branch arc the traced host cases take over every linked source; `cover_stride` traces every Nth case (a divisor of `freeze_stride`, so every frozen host vector is traced) to keep large files fast.
- `cover` and `'c64_rows': 'cover'` together: the C64 rows are the union of both picks, so they reach every edge and take every gcov arc. (gcc's arcs miss unsplit critical edges such as a switch's break, and clang's edges fold a constant `?:` into a select.) The proof then carries both `outcome_cover` and `c64_gcov_branches`.
- `'site_edges': True` (with `cover`): an edge of the shared code (interpreter bodies, image ops, native RTBs) or of the file's own helpers also counts once per call site, under the routine block (`.text.r<key>` section) that last ran before it. The runner reads the code ranges from the link map (`build/<id>-edges/native.sites`), and the trampoline ends a site. The C64 rows then take each site's own outcomes of the shared helpers, not just their union over the file.
- `'c64_cuts': True` (with `'c64_rows': 'cover'`): for arcs that only runs too long for one C64 diagnostic take, the freeze adds truncated rows. A truncated row is one such case run to a cut, the first NEWOPS with LOC at an interpretive routine the run enters. The cut is the earliest one whose prefix takes the arc, and the prefix must fit one diagnostic. The yaAGC oracle runs the same case to the same cut and prints kind 8 when it stops anywhere else first. Such a cut is refused, as is one where the native state differs. The row's digest is the oracle's full state at the cut. The cover record lists each truncated row (`truncated_rows`: row, source case, cut LOC, routines run). The spec links `tests/interp_cut.c` (`irc_run_cut`) and builds its runners with `-DIRC_CUTS`. A group with truncated rows defines `IRC_C64_CUTS` and `irc_c64_cut[]` in its vectors header, and the diagnostic calls `irc_run_cut` under `#ifdef IRC_C64_CUTS`. Specs without `c64_cuts` build and freeze exactly as before. When even the shortest prefix does not fit, the row resumes from the oracle's snapshot (`IRC_SNAP`) at the last routine its run enters before the arc, and the record keeps that snapshot (`resumed_at`, `snapshot`, `snapshot_sha256`). `--cover-check` rebuilds every truncated row from its source case (`cut_row`): a plain row is verified when the oracle stops at its cut with the native state, and a resumed row is verified as `digest` when its snapshot matches its sha256 and the native run matches the recorded digest. With `cover_stride`, arcs the traced sample takes only in runs too long for one diagnostic are first looked for among the short untraced cases.
  Whole-file basic specs (`'basic': 'full'`, which choose C64 rows by edge covers) take `c64_cuts` too (`cover_cuts`). After the groups are chosen, it traces the C64 rows, up to 8 cases of each routine left to the host (fewest edits first), and the cases `wider_arcs` finds. It cuts rows for the gcov arcs the C64 rows miss and packs them into groups after the others. It also writes the cover record, so `--cover-check` rebuilds the rows.
- `build/<id>-source-map.json`: each word of the file with the routine that compiles it. A word no routine compiles fails the freeze, unless the spec's `partial` entry names why.

### Fitting the C64 groups (whole-file basic specs)

Specs with `'basic': 'full'` choose their C64 cases by edge coverage (each case's clang trace-pc-guard/trace-cmp edges over every linked source) rather than a fixed count per routine. Some runs are too big to fit one diagnostic on their own, because they reach too many routines and banks. Such a case stays host-only and is listed in the proof's `c64_too_big_cases`. So are the other cases of its routine whose runs reach at least the same routines with at least as many edits, so one pass drops them all. The proof's `c64_gcov_branches` and `host_branches_missing_on_c64` show the C64 cases still take every branch outcome the host cases take.

Group selection reruns after each drop. Every size probe (one llvm-mos link) is memoized on its group, case text and flags, so a rerun links only the groups that changed. P40-P47's 49 groups fit in minutes instead of hours.

`'basic_split': True` emits one `basic_run_<bank>` per bank. Each group defines `-D<SYM>_BANKS` with the banks its kept routines and its cases' basic labels reach, so a diagnostic links only those banks. A transfer to a bank left out returns `EXEC_OUT_OF_SCOPE`.

## Additions from the orbital lane (ORBITAL_INTEGRATION, INTEGRATION_INITIALIZATION)

Spec keys:

- `cgoto`: the CADR list a CGOTO reads, with its length. The list's words are `cadr_list` rows in the source map.
- `indexed_fixed`: a starred operand into fixed memory, with the range its index register can take. It emits a `KX` table and checks the index at run time, refusing anything outside the table.
- `rtb` also serves BOVB to a basic label (BOVB TCDANZIG).
- `return_words`: erasable words that hold a saved interpretive return (TBASE2), seeded like QPRET with a return outside the file.
- `native_ranges`: basic routines implemented by hand behind RTB (the MOVE* state-vector moves), mapped in the source map to that function.
- `native_calls`: a basic `TC` to a label outside the file whose body is a hand-written C function (CSM_GEOMETRY's SHORTMP: `geometry_shortmp`). The TC compiles to that call, refusing the case when the function does, and the routine continues at the next word. The label is not an external, so the oracle runs through the same instructions.
- `native_entries`: those hand-written routines as registered transfer routines too, for files that reach them directly (MEASUREMENT_INCORPORATION's RTB MOVEACSM). Each runs its function and then its TC DANZIG as a pair's end; a left-hand RTB (EDOP holding the pair's right order) is refused.
- `max_steps`: the routines one case may run (default 200). With `resume_keys` every pair is a routine, and one DIFEQ step runs several hundred.
- `c64_flags`: extra flags for the C64 probe and the milestone's PRGs. The large integration files use `-fno-inline -fstatic-stack -mlto-zp=110`, with the image under KERNAL at $e000.

`all_states` sets the legal values of index registers, PBODY, DIFEQCNT, COLREG and PUSHLOC for every case.

A target of `SSP QPRET <label>` (PHEXIT, AMOVED) is a return point and must be listed in `entries`.

`rope_bin` reads rope words from the `MAIN.agc.bin` beside the listing, so bank checksums and fillers that the listing omits are read correctly.

Both files run with `resume_keys`. `states` restores each mid-sequence invariant a CHANG2 resume point would otherwise lose: between ACCOMP's `XCHX,2 DIFEQCNT` pairs, X2 holds the DIFEQ stage and DIFEQCNT holds PBODY; S1 holds PBODY inside GAMCOMP; COLREG indexes TIMESTEP's W stores after a VLOAD.

A spec with `resume_keys` keeps the case's chosen NEWJOB after the operands are randomized. Before, an operand block next to NEWJOB could leave it negative.

## Additions from CONIC_SUBROUTINES

- `flow_states`: before the cases are drawn, the oracle runs each entry's cases with `IRC_FLOW=1`, and the reference prints PUSHLOC, MODE, X1, X2, S1 and S2 at every NEWOPS. Each other routine then draws those registers from the values the file's own runs reach it with. `states` still wins, and a routine no entry reaches keeps random ones. A PUSHLOC off the pushdown list (an entry case's own random one) is not collected. Without this, a resume inside PARAM or GETX starts with a pushdown pointer or loop count the file never has there.
- A starred store tail (`STOVL* COGA / MUTABLE,1`) that reads a fixed table now goes through the same `KX` check as a starred load (`indexed_fixed`). Before, the load half of the tail read zeros.

## Additions from LUNAR_LANDING_GUIDANCE_EQUATIONS (Lane F, `lm-llge`)

- `index_tcf`: `INDEX K; TCF T` and `INDEX K; BZMF T` into a table of the file (`INDEX WCHPHASE; TCF WHATGUID`), with the offsets C(K) can hold. Each offset is a `case` of a switch; any other value refuses.
- `basic_fixed_index` also covers `EXTEND; INDEX K; INDEX T` with T a fixed table (CGCALC's `INDEX WCHPHASE; INDEX TARGTDEX; DCA TCGFBRAK`). The second INDEX adds the rope word T + C(K) for the listed offsets.
- `index_erasable`: an `INDEX` whose sum leaves erasable memory refuses (`INDEX MPAC +3; CA 5` reads a power table).
- `intpret_via`: subroutines the file `TC`s to that end in `TCF INTPRET` (INTPRETX). The interpretive code starts at the caller's Q, as after `TC INTPRET`.
- `in_file_returns`: returns a callee makes past its caller's `TC` (ROOTPSRS's `INDEX RETROOT; TCF 2`). Each is a key of its own.
- `returns` 0: a callee that never returns (`BANKCALL DISPEXIT +3` ends the job). Only its inline CADR belongs to the call.
- `rtb` with `(call, 'stops')`: the call returns `RTB_OK` to go on and 2 when the run ends outside the file (`lm_normunit` at the 1302 alarm).
- `TC A` (FASTCHNG's return) runs the word in A as an erasable program. `INCR BBANK` (TDISPSET's EBANK 5 to 7) is EB + 1, and past EB 7 it refuses.
- `c64_cover_partial`: C64 rows take every arc they can. The arcs only runs too big for one diagnostic take are listed in the proof (`c64_missed_arcs`) instead of failing the freeze. Only for files that stay partial.
- The freeze's C64 size probes run in parallel (one compile per core).
- LLGE uses `index_carry16` (below) for `INDEX WCHPHASE` with WCHPHASE -1.

## Additions from FINDCDUW (lane A2)

- `basic_fixed_index` also covers `INDEX K; MP T`.
- BOVB to the file's own basic code (FINDCDUW's `BOVB UNIT NOATTCNT`) works without `'basic': 'full'`.
- `refuse_unextended`: a CCS arm on the word after an EXTEND is refused instead of compiled unextended.
- `basic_edges` (differential): the erasable words interp_compile's own basic code names get the same ±0, small and saturating values as `'basic': 'full'` files' words.
- `index_carry16`: the generated `ixw` (INDEX's ones'-complement add) takes the end-around carry without a 17-bit sum. Without it, a C64 build (16-bit `unsigned`) drops the carry of an INDEX by a negative value (A = -1 on SPARCSIN's `INDEX A; CS LIMITS`) and refuses; other specs' generated files keep the old form.

## INDEX 16-bit carry on the other specs (steward)

The steward did not regenerate every spec with `index_carry16`: 69 sources' sha256 are pinned in milestone evidence, and regenerating would stale about 36 lanes. Instead:

- `tools/cert_precheck.py` WARNs (never FAILs), once per pending or complete certificate, when the certificate's interp spec lacks `index_carry16`. The spec is found from the `evidence/<id>/` records the certificate cites (spec keys are milestone ids), or from the spec's `file`. Files without an interp spec are skipped. The fix hint tells the lane to set the key at its next refreeze. The keys are listed in `REFREEZE_SPEC_KEYS`; `image_w` (steward PR #260) joins it when it lands.
- An audit of every spec's frozen C64 rows (the cases in `tests/<id>_c64_vectors.h`, through a host runner that logs each `ixw` call whose 16-bit and 17-bit results differ) found one spec without the key whose C64 rows are affected: `rtb-op-codes`, 18 of 144 rows (`INDEX A; CAF LIMITS` with A = -1, Luminary099/RTB_OP_CODES.agc:116-117 and 142-143, Comanche055/RT8_OP_CODES.agc:140-141 and 185-186). Those rows read LIMITS -2 for LIMITS -1, and both words are 37777, so their digests still match. `lm-p70-p71` has host-only cases (INDEX by -0, P70-P71.agc:138-139). `findcduw` and `lm-llge` exercise the carry but already have the key.
- `msu()` (MSU K, k < 3) also sums `ui+uj+1` in an `unsigned`, but it only keeps bits 0-15 of the sum, so it is the same with a 16-bit `unsigned` (checked under mos-sim). It is unchanged and does not depend on `index_carry16`.

## Additions from the CM flight lane (Comanche conics, integration, P38-P39)

Spec keys (all opt-in, so other specs compile byte-identically):

- `seed_counters`: whole-file basic specs seed the counter cells and input channels a case reads (`counters` on other specs means TIME1/TIME2 and the CDU states). The milestone's mk files define `-DIRC_CHANNELS`, which turns on the channel and counter case code in `tests/interp_case.h`.
- `rope_fallback`: a word the listing doesn't show (bank fillers such as V/SC's 200CS read) comes from the `MAIN.agc.bin` beside the listing.
- `fixed_tables`: `{'LABEL' or 'bank addr': (lo, hi)}`, the index range of a basic indexed read from a fixed table (INTBITAB, WAKESTAL).
- `fixed_index_erasable`: a starred operand listed in `fixed_index` whose effective address falls below 02000 reads erasable with no `K` table. INTEGRATION_INITIALIZATION's `DLOAD* MUEARTH,2` reads either a constant or an erasable copy depending on X2.
- `interp_not_basic`: a CCS or skip whose landing word is an interpretive order (or a blank word) treats that word as data, so basic code is not decoded from it. Opcode names that are also basic instructions (INCR, AD, ...) still decode as basic.
- `noreturn` also applies to a TC whose target is inside the file (P38-P39's TC DSPLY81, which ends in a TCF ENDOFJOB).

The yaAGC reference now decides whether a case's first instruction is in the file before it runs, so a case that starts by running an erasable word (TC INTWAKUQ) still stops at the first fixed address outside the file.

A spec's `max_steps` must match the `irc_run` limit in its `tests/test_<sym>.c` and `platform/c64/<sym>_diagnostic.c`; a lower limit fails long cases as kind 2.

`basic_parts` (`{'budget': n}`) splits a whole-file basic step machine into part functions of about `n` bytes of C text each, so every routine's link closure fits its flight-store slot (CM RCS DAP, jet selection, entry DAP, TVC initialize, automatic maneuvers). A goto that crosses a part becomes a hand-off: Z is set to the target word and the routine returns, so the transfer runner calls the target's routine as the next step. Each target is a new routine key (a part key), and so is every computed-transfer target once there is more than one part. A cut never falls before a word reached under EXTEND. A part key in fixed-fixed memory runs under the FB its caller left. Part keys draw no differential cases of their own: their runs come from the routines that reach them, so a split leaves the case set and the host vectors byte-identical (`--cover-check` reports `cases_match`). Only the C64 groups change. A pass now takes more steps, so these specs raise `max_steps` to 2000.
