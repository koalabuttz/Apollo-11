# Flight store and whole-program differential (phase 1)

Flight image integration, phase 1, Resident kernel lane. Two tools put every
lane's native routines together and run them as one program:

- `make flight-store` (`tools/flight_store_build.py`): every lane's routine
  tables in ABI 2 modules, packed into the REU store with one canonical
  resolver, plus a fit report (`build/flight-store/report.json`).
- `make flight-host-differential` (`tools/flight_host_differential.py`):
  both missions from Fresh Start on the host, against yaAGC, with erasable
  memory compared every centisecond. Logs, a `summary.json` and the
  unregistered points by file and lane go to `build/flight-store/differential/`.

Neither target is part of `test`. They measure the whole program while lanes
are still filling it in, and phase 1 adds no certificate.

## Store builder

`tools/flight_store_build.py` compiles every `src/*.c` on the host, reads
the symbols with `nm`, prints every routine table's keys, assigns tables to a
mission and builds one module per source file (the generic wrapper
`platform/c64/flight_table_module.c` plus the file's link closure, `-Oz`,
LTO, gc-sections). Lane and slot come from `docs/overlay-allocation.json`.
The resident's own tables (thin kernel, SPSIN/SPCOS, interrupt bodies) and
the certified Executive tables the thin kernel replaces are not packed. A
key two packed tables both hold is a collision: the resolver sends it to the
table of the file `docs/port-coverage.json` lists first, and the report
lists every collision for its lanes. `--host-only` writes only the host
table index (`build/flight-store/<m>-host.inc`) and the whole-program site
table (`build/flight-store/<m>-sites.inc`) for the differential.

The host index models the store's module layout for the host flight run's
module-load count (`tools/flight_host_run.c`). The pinned sets (slots 3 and
4) come right after the resident tables, never load and never take a slot.
The job-slot packs (`PACKS`: the excursion and T4RUPT) go ahead of their
files' tables: a hot subset of a resident table goes ahead of the resident.
A ride table (`RIDE`) holds its keys only while its pack is in the job slot
(slot 5). With a class-4 set in, task-slot modules count in the job slot. A
full build writes the index from the report's `pinned` and `packs`; a
`--host-only` build reads the last `report.json` when its sets still match
`PIN` and `PACKS`, and otherwise uses those as configured. On 84d550f5 this
cut the counted loads for cm-entry from 873,831 to 11,901 (the CM idle
loop's POSTJUMP and self-check run from the pinned class-3 set) and for
lm-p63-nassp from 195,318 to 123,454.

Each module carries its own operand table (`FS_SITES`): the
`agc_site_operand` arrays its sources compile to for the mission (read from
`cc -E` of each source), so no whole-program site table is needed. The
builder fails unless every `src/*sites.inc` has exactly one includer. The
host table index also writes `build/flight-store/<m>-sites.inc` (one array
per source) for the harness. A module's `fixed_data` is the resident's
(boundary words in the REU) unless `FIXED_DATA` names another:
`<m>_self_check` reads the rope image through its own hook while ROPECHK
runs, and `lm_trim_gimbal` (GTS reads an indexed CA through `fixed_data`)
reads every rope word from the mission rope image the store carries
(`FS_ROPE_DATA`, `platform/c64/rope_image_reu.c`). The builder writes that
image into `<m>.reu` at the rope base (`tools/rope_image_pack.py` `BASES`:
LM 0x1c0000, CM 0x1e0000) and pads the store to 2 MB.

The 2 MB REU, by address:

| REU range | Use |
| --- | --- |
| 0x000000 up | The module store (header, index, resolver, modules); then the module-test harness's case blob, below 0x1c0000 |
| 0x1c0000-0x1d1fff | LM rope image (LM store only) |
| 0x1df000-0x1dffff | Lane C Pinball trampoline's state save (`LANEC_PIN_REU_SAVE`, `src/lanec_pin_trampoline.c`) |
| 0x1e0000-0x1f1fff | CM rope image (CM store only) |
| 0x1f2000-0x1f2fff | The flight undo image (4 KB, one `apollo_overlay_state`) the five FLIGHT_INTERP_STOP modules stash before a run and fetch back on a refusal (`FLIGHT_UNDO_REU`, `include/flight_undo.h`) |
| 0x1f8000-0x1fffff | The load check's scratch (`SCRATCH_BYTES`; the build checks it is unused) |

The compact interpreter (Interpreter lane, `platform/c64/interp_module.c`)
goes into the store as two modules linked by `tools/interp_runtime_size.py`
`link_core`: $0A01 in the job slot (LM 15,847, CM 15,856 bytes) and its cold
bodies $0A02 at $D000 (slot class 2, loaded at boot; LM 4,038, CM 4,029).
`INTERP_POINTS` gives the resolver a record to $0A01 at each of the
interpreter's fixed entry points (logical Z, canonical bank 04000 or 06000)
unless a table record or a resident routine already holds it: SWRETURN is
a table record (the interbank tables), and INTPRET always goes to the
interpreter (#335), so 11 of 12 are the interpreter's on each mission. DANZIG is one of them: the resident's DANZIG marker is not a
routine, and the flight resident's own program has no marker table, so
without the record a job left pending at DANZIG would not resolve. CHANG2 is left out, since that is where
the interpreter hands back to the Executive. The module returns to the step
when the CIA1 count at $6FBC moves, so an interrupt waits at most one
interpretive slot. The load check resolves the records; it does not run the
interpreter (`make vice-interp-core` does).

Every C64 flight module compiles with `-DAGC_FLIGHT_INTERPRETER`. In the
files listed in `FLIGHT_INTERP_STOP`, that flag ends a chain at NEWOPS
([FLIGHT-INTERP-STOP.md](FLIGHT-INTERP-STOP.md)). The store leaves those
files' interpretive tables, and the `INTERP_ONLY` tables (LM
`lm_ephemeris_routines`, `lm_s3435_routines` and `lm_s3435b_routines`:
compiled interpretive code entered and left at NEWOPS; and on both
missions the compiled interpreter's `*_interp_core_basic` and
`*_interp_runtime_basic`, whose SWRETURN, INTPRET and DANZIG keys the
interbank tables, the interpreter's records and the DANZIG marker hold),
to the interpreter. Their
keys need no record: the step resolves by Z, which is NEWOPS there, and
NEWOPS has the interpreter's record. The report counts these keys as
`interpreter_keys`, separately from `unplaced_keys`. The host index and
`<m>-host.inc` keep the compiled tables.

`FLIGHT_PARTS` links one file as several modules, one per define set.
Each part's keys are what its own build's tables hold. The builder reads
them from a host object of the part, with the tables renamed, so a part
can key hand-off words the whole file does not. LM `lm_servicer` is linked
this way, as `-DLMSV_FLIGHT_PART=1` (85 keys, 14,581 bytes) and `=2`
(80 keys, 14,302 bytes)
([LM-SERVICER-FLIGHT-PARTS.md](LM-SERVICER-FLIGHT-PARTS.md)). Parts are
never chunked. With both lists in place, the five LM files fit their
slots. The LM load check then runs 1,269 checks (889 before) on
67 modules, with 0 failures.

`CO_LOCATE` packs a table into a second module as well, when two modules
would otherwise swap all the time: the interbank routines (POSTJUMP,
BANKCALL, ...) ride in `<m>_self_check`, whose idle loop calls them, which
cut module loads from about 4,200 a second to the rates below. It only
fits once self-check and the interbank routines shrink (fit, above).

A DOWNRUPT runs in one resident step. It used to take three: the lead-in
wrapper (vector 04040 to DODOWNTM), the down-telemetry module (DODOWNTM,
DNTMGOTO, the phase, DNTMEXIT) and the RESUME wrapper, each paying the
step's dispatch (resolver, trampoline and slot misses, about 23,000
cycles). `<m>_dntm_rupt_routines` (`src/lm_dntm_rupt.c`,
`src/cm_dntm_rupt.c`) copy the DOWNRUPT vector and RESUME onto the DNTM io
records and ride with `<m>_downtelemetry` (`CO_LOCATE`); the vector's
wrapper record goes to that module (`DOWNRUPT_HOST`), while RESUME's keys
stay with their wrappers, since only that module's run reaches its copy.
The run still stops at the work-clock budget, since TIME1-TIME6 count by
cycle steal inside an inhibited interrupt too; the next step resolves the
point it stopped at back into the same module, which is still loaded. Its
fixed points have records already, and the erasable DNTMGOTO gets one
under FB 05. `tests/test_dntm_rupt.c` runs every down-telemetry case's
DOWNRUPTs from the vector both ways (37,000) and fuzzes each copy against
its original. In VICE (A2's DOWNRUPT pass harness, mean cycles a pass): LM
P00 140,000 to 84,000, CM P00 110,000 to 72,000, CM entry 240,000 to
128,000; `flight_downlink_check.py --vice` matches yaAGC on all 300
DOWNRUPTs, one resident step each.

The device record `include/flight_io.h` covers every lane's io kind. Two
new kinds, `FLIGHT_IO_LM_CHANNEL` and `FLIGHT_IO_CM_CHANNEL`, carry the
mission record followed by `agc_input_channels` (channels 3, 4, 30-33);
the builder picks them for any file that uses `<m>_channel_io` (CM
`FINETIME` in IMU_CALIBRATION needs it).

On the C64 the resident hands every module its base record (`rk_io`:
`p66_bias_io` or `cm_native_io`). A module whose tables take another kind
(`module_io`; `FS_IO_KIND` in `platform/c64/flight_table_module.c`) builds
that record with `flight_io_enter` from `rk_hardware`, `rk_flight` and
`rk_device` before its run and writes it back with `flight_io_leave` after;
compiled routines' channels are already in `rk_hardware` (built with
`-DAGC_CHANNELS_HW=rk_hardware`), so nothing is copied. Its tables are marked `RK_TABLES_NOT_DIRECT`, so `rk_direct`
leaves them to the module main. A module may mix the base kind only with a
kind whose record starts with the base record (LM_*, CM_*), and the builder
refuses a kind with no `flight_io_enter`/`flight_io_leave` case. The
conversion adds about 330 bytes to the module main, against about 750 for
a base-kind module. The LM DAP tables take the base record (their channels
are `rk_hardware`'s), so `rk_direct` calls them with no module main; the
pinned LM DAP set measures 37,500 bytes.

Every module on the base record, pinned or not, is direct-only
(`FS_DIRECT_ONLY`). It links no transfer engine, and its main only builds
the table list and returns `EXEC_UNMAPPED` (nothing ran). After such a run
`rk_resident_step` calls `rk_direct` in the same step, so the point runs
through the new tables without an extra step. This saves about 490 bytes
per module. Against the build before it on the same tables, the CM image
fell from 1,417,728 to 1,368,576 bytes, because chunked CM DAP files need
fewer chunks. Direct-only needs the base record, so a module or pinned set
whose tables take another kind links the engine and conversion until the
kind folds into `agc_hardware`;
`report.json` gives both sizes (`bytes`, `direct_bytes`). The LM DAP set
measures 39,341 bytes today and 38,534 direct-only.

The VICE load check also fills the free soft stack below the timings with a
pattern and reports the deepest byte touched (`soft_stack_peak_bytes` in
`vice-load.json`): 302 bytes LM and 174 bytes CM, over every module's main
and one resolver record per module run through `rk_direct`. The timings (10 bytes
per module from $C400, at most 256 modules) leave the stack 512 bytes below
$D000, and the check fails (step 13) if the stack reaches them.

### Fit on 6b6ed2dc

| | Tables | Modules | Linked | Failed | Resolver records | Colliding keys | Store |
|---|---|---|---|---|---|---|---|
| LM | 113 | 63 | 16 | 47 | 213 | 650 (25 table pairs) | 297,472 bytes |
| CM | 89 | 52 | 16 | 36 | 202 | 405 (12 table pairs) | 294,912 bytes |

Every failure is a module bigger than its 16 KB slot (median overflow about
34 KB, largest 136 KB LM and 170 KB CM). LM `service_transfer` misses by 65
bytes since it carries its own 171 operand sites (684 bytes), and LM
`lm_fresh_start` by 359. `<m>_self_check` links since services 0x0202
(`apollo_reu_transfer`) but misses by about 1.1 KB on its own (its step
machine `body` is 10 KB) and by about 9 KB with the interbank tables
co-located, so the co-location below is the target layout rather than one
that fits yet. The store, with the resident's own modules (five LM, four
CM), is far below the 2 MB REU. The load path works in VICE (below); the
whole-program scenarios in VICE wait on the modules fitting.
Interpretive modules wait on the interpreter decision
(`/mnt/project-files/steward/interpreter-proposal.md`).

A basic-only module over its slot is split into chunks of consecutive
routines (`platform/c64/flight_table_module.c` `FS_CHUNK`: entries copied
field by field by constant index, so LTO links only the chunk's routines),
halving until each chunk fits. Files in `GREEDY_SPLIT`
(`tools/flight_store_build.py`) split greedily instead: each chunk takes
as many consecutive routines as fit, found by binary search on its end.
The CM entry DAP is one: a pass runs its table front to back, so fewer and
larger chunks mean fewer loads per pass. A chunk holds only its own file's sources
and operand sites; a table co-located into another module is never
chunked. Chunks that fit are kept even when others do not; a single
routine still over its slot is reported as a failed entry
(`one routine over its slot`, with its `routine_key`), and a file of more
than 64 KB is `too big to split`. Before #211-era step-machine splits and
Lane F's fix to the entry copy (a whole-struct copy kept every routine
linked), chunking gained nothing; now it does.

A key whose every holder failed has no module in the store: the C64 stops
there, while the host run bridges it through yaAGC. `report.json`
`unplaced` counts these keys per table, and the build fails when a table
has more than `docs/flight-store-unplaced.json` allows (a table not listed
is allowed none). On 80342904 that is LM P20-P25 (450 keys, the file is too
big to split), the Pinball remainder (LM 268, CM 267) and CM IMU calibration
(16 single routines over a slot). A fix that lowers a count lowers the file
with it; the build prints the new count.

Shadowed entries are left out. Several files are compiled by more than one
lane (CM EXTENDED_VERBS as Lane C's `lanec_ev_cm_*` tables and as
`cm_extended_verbs_file_basic`, for example). A key resolves to its first
holder that is not the interpreter's, so the other tables' entries for it
never run. A module or chunk with such entries links only its live ones,
in key order (`FS_PICK`, X(table, index)). `--gc-sections` then drops the
shadowed routines, and a module with no live entries is left out. The
report lists `shadowed_entries` per table and
`modules_left_out_shadowed`. Flight parts keep their own key slices.
Erasable keys are always kept.
A chunk or pick module gets its entries' keys as data (`FS_KEYS`, read
by the build from the same tables): its main copies them in one loop and
stores only the calls, where it stored both per entry (about 25 bytes of
code each). On LM that is 11,776 bytes of the store; DAP pass cycles in
VICE do not change.

With every table fitting, both stores ran past the rope image of a
2 MB REU (LM 2,146,816 bytes against 1,835,008; CM 2,552,576 against
1,966,080). After pruning (573 LM and 665 CM entries, 10 and 8 modules
left out) and CM EXTENDED_VERBS as two flight parts
(`docs/FLIGHT-PARTS.md`), they are LM 1,485,568 and CM 1,434,880 bytes.
Unplaced keys did not change (LM 268, CM 267: Lane C's Pinball routines).

### Fit with chunking (this branch, after #209)

| | Tables | Modules | Linked | Failed | Store image |
|---|---|---|---|---|---|
| LM | 103 | 146 | 89 | 57 (25 one routine over its slot, 32 whole files: interpretive, every routine over, or too big to split) | 1,067,264 bytes |
| CM | 90 | 207 | 146 | 61 (34 one routine over its slot, 27 whole files) | 1,473,536 bytes |

The over-slot routines and files, by lane, are in
`/mnt/project-files/resident-kernel/over-slot-2026-10-13.md`.

### Runtime sharing

A module no longer links its own copy of the transfer ABI and the common
image ops. The resident exports them as ABI 2 services (table 0x0203,
docs/overlay-v2-services.json): `agc_transfer_run`, `agc_transfer_point`,
`agc_fixed_physical`, `memcpy` and the kernel's `k_<op>` for 17 image ops
(CA, CAF, CS, CSF, AD, ADF, MASKF, TS, XCH, LXCH, ADS, DXCH, CCS, TC, DCAF,
BZF, BZMF). `tools/flight_store_build.py` links each module with
`-Wl,--wrap=ai_<op>` and `platform/c64/flight_image_op_services.c`, whose
`__wrap_ai_<op>` calls `k_<op>` (the frame's image is always the canonical
one at `APOLLO_SHARED_ADDRESS`). Ops whose export would cost the resident
too much (DAS, INCR, QXCH, MASK, DCSF, `ai_put`, `ai_word`;
docs/RESIDENT-FLIGHT.md, Size) and ops the kernel does not wrap stay in
each module. On def640e3d, against the same tree without sharing:

| | Modules (linked, failed) | Resolver records | Mean module | Store image |
|---|---|---|---|---|
| LM before | 129 (72, 57) | 596 | 11,040 bytes | 936,192 bytes |
| LM after | 113 (66, 47) | 606 | 9,022 bytes | 721,920 bytes |
| CM before | 248 (183, 65) | 791 | 8,181 bytes | 1,636,608 bytes |
| CM after | 183 (133, 50) | 806 | 7,353 bytes | 1,106,944 bytes |

Fewer modules link because more files fit whole and need no chunks; the
resolver records (keys the store can run) go up on both missions.

`PIN` names the tables pinned in a region, each set linked as one module
and reported (`report.json` `pinned`). A slot-3 set goes to slot class 3
($C000-$CBFF, 3,072 bytes; docs/RESIDENT-FLIGHT.md): when it fits
direct-only the builder packs it into the store, the resident loads it at
boot, and its routines run from there through `rk_direct` while the resolver
still names their packed modules. A slot-4 set goes to slot class 4, the
task slot's $E000-$FFF9 (8,186 bytes), and is packed only beside a packed
class-3 set, whose table list names its tables (the class-4 module runs no
main). With a class-4 module in the store every task-slot module, the
resident's interrupt wrappers included, links for the job slot
(`task_pinned_out` in the module list), and so does the null module 0x01E0. This is the steward's DAP rule
in two regions, with the LM RCS DAP hold cycle split by what one pass
reaches (the hold bench's per-pass keys, 21.9 KB after the compaction in
#356):

- Class 4 holds TJETLAW (`lm_tjet_law_routines`) and the RESUME family
  (`lm_t6rupt_hot_routines`: RESUME, NOQRSM, NOQBRSM, NOQBRSM +1), and is
  the *runtime home*: every global function the class-3 module links (the
  fast ops, the image-op wrappers, the DAP files' shared helpers in
  `include/lm_dap_shared.h`) is linked once, here. Every interrupt ends at
  RESUME, which otherwise loads the T6RUPT wrapper ($01E4, 7,991 bytes);
  `rk_direct` finds the pinned entries first, and the wrapper's resolver
  records stay as the fallback.
- Class 3 holds DAPIDLER's per-pass entries (`lm_dapidler_hot_routines`, a
  file's `*_hot_routines` subset: PIN only, its file's modules still pack
  the full table) and the table list naming both sets.
- The *excursion* (`EXCURSION`, module $07F5) packs P-axis's and Q,R-axis's
  per-pass entries (`lm_p_axis_hot_routines`, `lm_q_r_axis_hot_routines`)
  and RATELOOP (`lm_kalman_basic`, which runs in the same pass) in one
  job-slot module, so a pass that leaves the pinned sets loads one module,
  not two (P-axis 11,603 bytes and Q,R-axis 11,971). RATELOOP moved here
  from class 4 to make room for RESUME. It also carries the jet writes
  (`lm_t6rupt_jets_hot_routines`: WRITEP, WRITEU, WRITEV and their -1
  entries), which a firing pass makes three times; before, each loaded the
  T6RUPT wrapper ($01E4) and then reloaded the excursion. They are in
  `RIDE`: run through `rk_direct` while the excursion is loaded, while
  their resolver records keep the wrapper for every other caller. IBNKCALL
  and ISWRETRN ride the same way (`lm_interbank_dap_hot_routines`, a
  compact copy of `lm_interbank_routines`' two entries, checked against
  them by `make test-lm-interbank-dap`): the passes call across banks
  through them, and each call loaded the interbank module and then
  reloaded the excursion. Their CADR operands are the callers' words from
  `lm_interbank_routines`' site list (`RIDE_SITES`), the ones in the banks
  the excursion's own entries are in. On LM it is 16,295 bytes.
- The *T4RUPT pack* (`T4PACK`, module $07F6) packs T4RUPT's core
  (`lm_t4rupt_core_hot_routines`, the same entries as the resident wrapper
  $01E2's `lm_t4rupt_core_routines`) and the radar step
  (`lm_t4rupt_rr_routines`), 6,773 bytes, so a radar T4RUPT loads one
  module instead of two. The core's wrapper resolver records point at the
  pack. RCSMON (`lm_rcsmon_basic`) stays in its own module: it takes
  `lm_channel_io`, and a pack holding it would run every step through the
  transfer engine instead of `rk_direct` (measured: radar T4RUPT steps cost
  31 K cycles instead of 16 K).
- The *CM entry pack* (`ENTRYPACK`, modules $07F7 and $07F8) packs the 26
  entries a CM entry DAP pass (T5RUPT) reaches (`cm_entry_dap_hot_routines`,
  Lane G; `HOT_FULL` names its full table, `cm_entry_dap_basic`). Those
  entries were spread over all of the full table's chunks, so a pass loaded
  about 5 of them. CM pins no class-4 home, so this pack is `ALONE`: it links
  its own runtime. Its routines share the file's step bodies, so the 26
  are over one job slot by 2 KB; a stand-alone pack over its slot splits
  into consecutive chunks that each take as many entries as fit
  (`split_pack`, a binary search on each chunk's end), here two. On
  4214bb6a the CM entry window from 17000 cs (172 cs) went from 21.24x to
  20.05x real time, with T5RUPT job loads 85 to 69 and its mean cost 577 K
  to 465 K cycles.

The excursion and the T4RUPT pack (`PACKS`) are linked only beside a packed
class-4 home, the `ALONE` packs with or without one. Each links once on its own before `pin_pair`; the home then
keeps global (`--undefined`) those of a pack's functions it links anyway,
and the pack takes every function the home holds as a global
(`--defsym`). A function only a pack uses stays in the pack. The resolver
sends a pack's keys to it before their files' modules.
Measurements: `/mnt/project-files/steward/cpu-budget.md` (D2, 10-14).

The two link separately (`pin_pair`). Class 4 links first with class 3's
files as well, keeping only what class 3 asks for (`-Wl,--undefined=` for
each name in class 3's map; LTO and `--gc-sections` drop the rest). Class 3
then links only its own files, taking those addresses from class 4's ELF
(`-Wl,--defsym=`). The pair relinks until class 3 resolves and links no new
name. The class-4 module is compiled with `FS_PIN_HOME`: its main is a stub,
as the resident never runs it, and it only keeps its tables linked. Both
link with `-Wl,-mllvm,-inline-threshold=-50` (`PIN_LTO`), so the LTO step
also stops inlining. On LM, class 4 is 8,121
of 8,186 bytes and class 3 is 2,910 of 3,072. CM pins the background self-check idle loop in
class 3 (`cm_self_check_idle_routines`, 1,920 bytes), so those routines run
without loading a module; on LM that loop is an ordinary job-slot module, as
class 3 is kept for the DAP. The load check runs one of the class-3 set's
points through the module that packs the same table and through `rk_direct`
with only the pinned module present, compares them, checks the class-4
header at $E000 when there is one, and then drops the pinned module (its
mailbox and timings use $C000-$CDFF).

### Direct calls into a loaded module

Header word 22 (reserved before) is `TABLES`: the address, inside the
slot, of the module's table list (`rk_module_tables`,
`platform/c64/resident_flight.h`), or 0. `overlay_v2_payload.ld` fills it
from a module's `.ov2_tables` word, so only flight modules carry one. A
module builds the list on its first run after a load (the load clears its
BSS, so a fresh copy is never trusted). `tools/overlay_v2_pack.py` checks
the word lies inside the slot; the loader does not read it.

After the trampoline and before the resolver, the resident's step calls
`rk_direct`: for each loaded slot with a ready list, it runs
`agc_transfer_run` on the resident's program with the module's tables, at
most 256 steps. `EXEC_UNMAPPED` with no step means the point is not that
module's; the resolver and an ABI 2 run follow only when no loaded module
holds the point. A module's main does the same through its own list, so
both paths run the same tables. The list also carries the high bytes of
the lowest key its basic tables hold and how far above it the highest
key's is (`FS_KEY_LO`, `FS_KEY_SPAN`, from the builder), and `rk_direct`
skips a module whose range misses the high byte of the step's fixed point
(0xff when Z is erasable; a module holding an erasable key, or with
interpretive tables, spans to 0xff), so most slots that do not hold the
point cost a byte compare instead of a run. Measured on the LM DAP hold
(work clock, VICE): 11.92x to 11.19x real time, for 43 resident bytes.

A module never crosses a 512 KB REU bank (`tools/overlay_v2_pack.py`
starts it on the next bank instead): one DMA loads a module, and the REU's
address counter wraps inside the bank, so a crossing module loaded the
store's first bytes into the slot's tail.

The resident and every module are compiled with
`-DAGC_TRANSFER_DANZIG_MARKER` (`tools/resident_flight_build.py` COMMON):
the pending-DANZIG marker is opt-in on the C64 since #184, and the resident
resolves pending RTB right-hand orders through it.

## VICE load path

`make flight-store-vice` (`tools/flight_vice_load.py`) boots the flight
resident on the whole-program store in VICE. The store builder also packs
the resident's own modules into the store (the interrupt-body wrappers with
their records, and the null module), ends it with the boundary-word table
built for the store's resolver records (`build/flight-store/<m>-boundary.bin`,
docs/RESIDENT-FLIGHT.md), so `build/flight-store/<m>.reu` is a bootable flight image. It
also links `build/flight-store/<m>-load-check.prg`: the resident with
`platform/c64/flight_load_check.c` as main. After `rk_boot`, the check
reads every resolver record from the store, resolves its key through the
resident's resolver and checks the module it names. The record count and a
Fletcher checksum over each record's bank, loc and module in key order must
match the builder's list (`FL_RECORDS`, `FL_RECORD_SUM`); the list itself
is not linked, as LM's 2,480 records would overrun the boot-only region. It then runs each
flight module twice, with interrupts off, from a fixed point no table holds:
once loading it from the REU into its slot, and once already loaded. Each run
must come back with `EXEC_UNMAPPED`. CIA2's cascaded timers count the
cycles. The timings go to `build/flight-store/vice-load.json`.

On 6b6ed2dc both missions pass (LM 278 checks, CM 259): every record
resolves, and all 16 fitting modules per mission load and return. Load cost
over a cached call, fitted over the 32 modules (5.7 to 16.2 KB, largest
residual 880 cycles):

    load = 7,459 + 1.103 cycles per byte   (PAL: 985,248 cycles a second)

With chunking (this branch) both missions pass with every linked module:
LM 740 checks (89 modules, load and call 18,367-48,231 cycles), CM 1,067
(146 modules, 18,187-60,798). Over 235 modules the fit is 10,645 + 1.227
cycles per byte with a largest residual of 22,476: some modules' first run
does more work than their cached run, so the 32-module fit above is the
cleaner transfer cost. For bisecting a crash, `flight_load_check.c` takes
`-DFL_FIRST`/`-DFL_LIMIT` (a module range) and `-DFL_NO_PROFILE`.

The check then runs one resolver record per module both ways: through the
module's main (an ABI 2 run) and through `rk_direct`, from the same image,
slot contents and channels (snapshotted in the REU's last 32 KB), with only
that module loaded. Status, image and channels must match. A direct-only
module's main must return `EXEC_UNMAPPED` and leave image and channels as
they were (step 12). Its point then runs through `rk_direct` alone, and the
report gives those cycles (`direct_only`; LM 25 modules at 3,344-27,187
cycles, CM 45 at 4,155-22,076). The pinned check's reference run goes
through a direct-only twin's tables with the pinned module set aside. On def640e3d
plus this change both missions pass (LM 903 checks, 44 modules compared;
CM 1,394, 99), and a direct call saves 1,433 to 2,536 cycles over the
module main: the ABI 2 run and the copy of the program record. Most of
what is left is the step itself: the transfer point and the table search.

The step now computes the transfer point once (it was computed again to
pick the table set), which cut it from about 1,100 to 880 cycles, and the
search skips a table whose key range cannot hold the point.
`flight_store_build.py` refuses a table whose keys do not strictly ascend,
since both the binary search and the range skip rely on it. With this,
cached calls are 6,081 to 8,158 cycles (from 7,287 to 11,407), and the
check passes on both missions (LM 907 checks, CM 1,398). The resident
resolve is still 4,200 to 4,900 cycles of each step.

A cached call (no load) costs 6,100 to 8,200 cycles on its own: the ABI 2
run, the module copying its program record and tables, and one resolve. So a
swap to a 12 KB module costs about 21,000 cycles plus the call, about 30,000
in all (3% of a second), and a full 16 KB slot about 35 ms. At the LM's 40
module loads a second (DAP chain, results below), swaps alone would take
more than the CPU (about 1.2 million cycles a second), and even 5 a second
costs 15%. The task-slot DAP chain needs its modules merged or kept
resident, not just made to fit.

## Host differential method

`tools/flight_host_run.c` (GPL, host only, links yaAGC; never part of the
port) runs one mission:

1. **Cold start.** Native and yaAGC both start at GOJAM with yaAGC's
   `agc_engine_init` state (channel 30 = 037777, channels 31-33 = 077777,
   no DOWNRUPT). Both missions take PTBAD (FAILREG 01107) and then
   DOFSTART, natively and in yaAGC alike.
2. **Native run.** The thin kernel and every packed table, resolved by the
   canonical key, run through `rk_rupt_take` and the trampoline. Each call
   sees the operands and `fixed_data` its module would hold on the C64: its
   own site table, and the boundary words (the rope word at every table key,
   the interpreter fetch point and DANZIG) or self-check's rope hook.
   Interrupts take the word at Z, and at an interpretive routine Z is the
   fetch point or DANZIG rather than the key, so without those two words an
   interrupt there stays pending (LM V37E00E: 500 of 3000 samples equal).
   The resident's boundary table needs the fetch point too once it
   resolves interpretive orders.
3. **Lockstep timing.** A native routine has no clock, so each call is
   checked by running yaAGC from the same before-state until it reaches the
   routine's continuation with equal erasable memory (024-031 and Z
   excepted; A, L and Q at 16 bits; a RESUME boundary with
   SubstituteInstruction set counts). On a
   match the native side takes yaAGC's timers 024-031, channels 3/4, channel
   13 bit 15 and the interrupt requests raised up to the match. On a
   mismatch the routine is reported and the timing comes from yaAGC's first
   arrival at the continuation.
4. **Bridges.** A point no table registers, or a routine that refuses (its
   effects undone), runs in yaAGC with its own clock and interrupts until a
   registered point or idle, and is reported with its arrivals and
   instruction count.
5. **Samples.** Each centisecond, at the first pass through ADVAN (LM
   03214, CM 03231, checked by physical address), idle, nothing pending, in
   the same SCALER1 phase, erasable memory is compared with an independent
   yaAGC run (0-017, 024-031 and RUPTREG1-4 excepted).
6. **Keys.** DSKY keys are pressed at a sample point. The yaAGC run enters
   KEYRUPT1 at the same instruction as `rk_rupt_take` does (ZRUPT =
   ADVAN+1, BRUPT = the rope word at ADVAN under the current FB and
   superbank).
7. **Module loads.** One module per slot (job, task); a routine whose
   module is not in its slot loads it. Each scenario reports loads per
   second and the points that cause them.
8. **Set-up knobs** (environment, both sides alike): `FLIGHT_CH30` and
   `FLIGHT_CH31` set input channels 30 and 31 at start (octal; e.g.
   `036377` and `057777` for IMU operate, PGNCS control and AUTO, so
   DAPIDLER starts the DAP); `FLIGHT_POKE=CS:ADDR=VALUE,...` sets erasable
   words at the first sample point at or after CS centiseconds (e.g.
   `5000:035=02000` moves CDUY); `FLIGHT_LOAD=CS:FILE,...` sets the
   erasable words of a load file (octal `ADDR VALUE` lines, `#` comments;
   tools/flight_scenarios/) the same way; `FLIGHT_CH32` and `FLIGHT_CH33`
   set those input channels as `FLIGHT_CH30` does. In the keys argument,
   `@CS` holds the next key until CS centiseconds (keys otherwise go every
   half second from 2 s). The reference keeps up to 32,767 centiseconds
   (scaled time, which has none, any number). `FLIGHT_CORE=FILE` (scaled
   time only) starts from a yaAGC core dump (agc_engine_init's format,
   e.g. a NASSP save) in place of GOJAM: yaAGC loads it and finishes any
   EXTEND, INDEX or interrupt sequence it stopped inside and runs on to
   the idle loop (a dump is usually inside a routine), the native side
   takes that state with any `FLIGHT_CH30`-`33` given overriding the
   core's, and AGC time (keys, pokes, the centisecond count) runs from the
   core's TIME1/TIME2. It does not combine with `FLIGHT_DOWNRUPT` (yaAGC's
   steps to the idle loop would enter the downlink reference stream).
   `FLIGHT_MISS_LOG` prints each routine yaAGC did
   not match with its centisecond, Z, FB and Q (a diagnostic); set to a
   directory, it also names the three routines before each and writes the
   erasable state the first eight started in there.
   `FLIGHT_ENGINE=KG` (LM) adds a minimal descent engine, the same on both
   sides at each sample point: from ENGONFLG it thrusts at 10% (373
   throttle bits of 12.53 N), follows each throttle command (PSEUDO55 when
   TTHROT changes) up to FMAXPOS, burns KG kilograms down at DPSVEX, and
   adds its velocity to PIPAX-Z (1 cm/s a count) along body X from CDUY
   and CDUZ. No attitude dynamics: while it burns, CDUX-Z are set to the
   DAP's desired angles CDUXD-ZD. `FLIGHT_AERO=B` (CM) adds drag and lift
   for a capsule of ballistic coefficient B kg/m^2 and L/D 0.3: while
   SERVICER runs and the CM is below 140 km, each sample point takes the
   trajectory from the AGC's own RN and VN (exact PIPAs make its
   navigation the truth), puts drag (an exponential atmosphere, 6.7 km
   scale height, against the earth-relative velocity) and lift turned by
   the commanded roll ROLLC into PIPAX-Z (5.85 cm/s a count, through
   REFSMMAT). No attitude dynamics: while P62 waits for the maneuver to
   entry attitude (P63FLAG +1), CDUX-Z are set to the desired angles
   THETAD.

9. **Flight scenarios** (tools/flight_scenarios/, `FLIGHT` in
   tools/flight_host_differential.py). `lm-p63-descent`: lm_p63.py writes
   lm-p63.load, a synthetic but self-consistent P63 start loaded at 20 s
   (state vector in a 15 x 110 km descent orbit, REFSMMAT, Tranquility
   RLS, braking and approach targets, ignition criteria, mass). With the
   IMU operating, PGNCS, AUTO, auto throttle and the LR antenna in position
   1, the keys run V37E63E, proceed at the TIG display, ENTER on fine align
   (R51), proceed and then ENTER on the attitude maneuver (R60), and
   proceed on V99N62: ignition at 146 s, SERVICER, LUNLAND and QUADGUID
   every 2 s from 172 s. Without an engine DVMON flashes V97 (thrust fail).
   `lm-p63-descent-thrust` adds `FLIGHT_ENGINE=15100`: DVMON passes and the
   LLGE throttles and steers the braking phase (yaAGC alone reaches P64 at
   787 s and 4,000 ft at 900 s). Both run under scaled time x1 with
   `FLIGHT_SWAP_MCT=0` (the DAP pinned): unscaled, yaAGC's TIME3 can tick
   between WAITLIST's CS TIME3 and XCH TIME3, which a native routine
   cannot match (LST1 one count apart from 0.63 s on with the IMU
   operating); with 20 ms swaps the DAP's ~40 loads a second leave the CPU
   no idle time, and no key is taken.
   On base df172441: `lm-p63-descent` runs 1,127,577 native routines in
   250 s (154,791 bridged instructions) and `lm-p63-descent-thrust`
   1,276,118 in 300 s (252,398 bridged), both in P63 at the end. Each has
   8 routines yaAGC did not match, all compiled interpretive ones
   (INTEGRATION_INITIALIZATION, GIMBAL_LOCK, KALCMANU_STEERING) leaving Q
   without the sign extension yaAGC gives it (040000 for 0140000), and 75
   unregistered or refused points, 47 of them in INTERPRETER.agc.
   `cm-entry`: cm_entry.py writes cm-entry.load, a synthetic lunar-return
   entry loaded at 20 s (CSM state vector 150 s before an entry interface
   at 400,000 ft, 11.03 km/s and -6.5 degrees; REFSMMAT with Y along the
   orbit normal, as S61.1 checks; splash point 2,400 km downrange; EMS
   altitude, trim angle and L/D pads). With the IMU operating and
   `FLIGHT_AERO=355`, the keys run V37E61E and proceed through P61 (N61,
   N60, N63) and P62 (separation request V50N25 00041, then N61); P63
   senses 0.05 g and P64 and P67 fly the roll. yaAGC alone: P63 at 99 s,
   P64 at 204 s, P67 at 306 s, peak 6.7 g, and at drogue conditions
   (470 s, 24 km, 650 m/s) N66 shows about 1 nm crossrange and 2 nm
   downrange error. On base 4f0e3d4a the native run (32,000 cs, scaled x1)
   takes 1,432,112 routines (11,557 bridged instructions) and is in P67
   at the end; each is checked in lockstep against yaAGC from the same
   state, and 1 did not match (INTEGRATION_INITIALIZATION's compiled
   interpretive Q, as above) and 16 unregistered or refused
   points (POLY, SLOAD2, NEWMODE, TPAGREE, TFFXTEST and P61-P67 returns).
   `lm-p63-nassp`: Apollo 11's own P63 state, the NASSP core
   tools/scenarios/luminary099-p63-ignition.core (102:25 GET), with AUTO,
   the LR antenna in position 2 (`FLIGHT_CH33=036673`; position 1, as the
   core has it, gives alarm 523 at high gate) and `FLIGHT_ENGINE=15100`;
   the keys are those of `lm-p63-descent`. The LLGE flies the braking
   phase to P64 at about 1,118 s after the core (on yaAGC alone too); with
   no landing radar model, alarms 511 and 522 recur every 2 s.
   tools/flight_scenario_vice.py runs a flight scenario on the C64: the
   host run's native state at `--snap-cs` (FLIGHT_SNAPSHOT) loaded into
   the scenario mission's flight resident on its whole-program store in VICE
   (platform/c64/dap_pass_check.c, as tools/dap_pass_vice.py does), run
   for `--cs` centiseconds with cycles, steps and module loads per AGC
   interrupt; a refused or untranslated point stops it.
   On base 72b40a63 `lm-p63-nassp` runs 5,031,476 native routines in
   1,150 s (240,234 bridged instructions, 41 unregistered points) and is
   in P64 (MODREG 100) at the end. Under
   scaled time there is no sample-by-sample reference (yaAGC's clock is
   frozen); each native routine is instead checked in lockstep against
   yaAGC from the same state: 8 of the 5,031,476 did not match (5 points),
   all the compiled interpretive Q of INTEGRATION_INITIALIZATION (as
   above). The core was dumped inside
   DOWNRUPT, so yaAGC runs it to the idle loop before the native side
   starts: entering DOWNTELEMETRY's continuation with the dump's Q (00336)
   gives no state a flight reaches.
   `flight_scenario_vice.py` on `lm-p63-descent-thrust` from 175 s (the
   LLGE running), on base 72b40a63: 1,564,617 cycles for 66 AGC
   centiseconds (2.4 times real time), then a point the store does not map
   (40,3534; 46 of the 124 LM table modules do not link into the
   flight store yet). With every table but Pinball's linked (6f894e00) it
   stops at 40,3534 after 65 centiseconds, and `cm-entry` from 175 s at
   40,3441 after 1,596 (9.0 times real time, 857 steps): DSPMMJB, the
   Pinball job SETUPDSP starts through NOVAC (FB 30 with superbank bit
   0100, so not NB2CDUSP+1 in bank 30). Pinball is not in the store yet.
   `FSV_STEPS=1` links DP_STEPS and prints the last step records as
   physical bank,Z (superbank applied).

10. **Scaled time** (`FLIGHT_SCALED=K,D,LAG[,SEED]`, the replay proof of
   docs/RESIDENT-FLIGHT.md, Scaled time). yaAGC's own clock is frozen:
   `tools/flight_host_differential.py` builds a local engine variant whose
   `AgcFrozenClock` stops the scaler loop (TIME1-TIME6, the scalers and the
   cycles their counts take) and the IMU CDU drive, which bursts on
   yaAGC's cycle counter; unset, the variant is the unchanged engine. The
   AGC clock then runs only as the flight resident runs it: a modeled C64
   wall clock advances by each span's AGC machine cycles times K, plus
   20 ms for each module swap (the VICE load cost; `FLIGHT_SWAP_MCT`
   sets it in AGC machine cycles, 1,707 by default), each optionally
   jittered from 1/4 to 4 times by SEED. A CIA1 interrupt lands every
   D × 10 ms of it, and at each native boundary at most LAG of them become
   `rk_timer_tick` periods, the rest dropped, as `rk_rupts` does; bridged
   code takes its periods between instructions. Lockstep then compares the
   clock counters too and adopts nothing from yaAGC, so every routine must
   match yaAGC run from the same state under the same periods, and the
   run as a whole is one yaAGC run under that tick schedule. No
   free-running reference runs, and no samples are compared.
   `FLIGHT_SCALED=W` replays the work clock instead (docs/RESIDENT-FLIGHT.md,
   The work clock): each native routine advances the clock by its
   module's MCT weight (as the flight build computes it from
   docs/flight-routine-mct.json), a self-check idle routine moves it to
   the next tick, module swaps cost nothing, and no tick is dropped.

`tools/flight_holes.py` names the file, label and owning lane of every
unregistered or refused point from the listing and the allocation.

## Results on 8564ca79

| Scenario | Samples equal | Major mode native / yaAGC | Routines yaAGC did not match | Module loads per second |
|---|---|---|---|---|
| LM P00 idle, 4000 cs | 4000 / 4000 | -0 / -0 | 0 | 40.3 |
| CM P00 idle, 4000 cs | 3998 / 4000 | -0 / -0 | 0 | 0.3 |
| LM V37E00E, 3000 cs | 3000 / 3000 | 00 / 00 | 0 | 43.2 |
| CM V37E00E, 3000 cs | 2998 / 3000 | 00 / 00 | 0 | 2.2 |
| LM V37E12E, 3000 cs | 3000 / 3000 | 14 / 14 | 0 | 43.1 |
| CM V37E06E, 3000 cs | 2998 / 3000 | 06 / 06 | 1 | 3.1 |
| LM V37E63E, 3000 cs | 3000 / 3000 | 77 / 77 | 0 | 43.1 |

On 0ce769c6 with this branch's store, the CM rows are unchanged; LM P00
idle stays 4000/4000, and the three LM V37 scenarios are 2900/3000 with
52.8-56.3 module loads a second. The 100 samples (4.01 to 5.00 s) differ only
in LOC (0164, 20002 native, 20042 yaAGC) after the DSKY key at 4.00 s: yaAGC
passes interpretive point 33490 after Pinball's 32831, natively the
`lanec_pin_lm` routine runs past it. The same tables give the same result
with the runner from 0ce769c6, so this is drift in the base's tables since
8564ca79, not the store builder.

Findings:

- **LM module loads.** About 40 a second, all in the task slot: the DAP
  chain `lm_dapidler` (MOREIDLE+2, SHUTDOWN), `lm_q_r_axis` (QERRCALC) and
  `lm_p_axis` (CALCPERR) each load once per DAP pass. They fit together only
  once Lane F splits the step machines.

- **CM samples at 1 and 2 cs.** Native interrupt latency: the CM fresh-start
  routine at 05608 runs about a centisecond as one native call, so a T3RUPT
  due at 2.20 cs waits for it while yaAGC takes it at once (words 01361 and
  01371 differ). Memory converges by 3 cs.
- **CM `cm_imu_calibration_basic` at 02,2554 (FINETIME).** L 37373 native,
  37374 yaAGC: the `agc_input_channels` ABI latches the scaler for the
  routine's whole call while yaAGC's scaler advances between the reads.
- **CM P11.** V37E11E is rejected (operator error) by yaAGC and natively
  alike, so P11 is not reachable by V37 from Fresh Start. CM V37E06E reaches
  P06 instead.
- **No lost BRUPT.** An interrupt never landed at a RESUME point whose
  BRUPT differs from the rope word.

### Unregistered and refused points, by lane

Every point below ran in yaAGC. Arrivals are from the 4000 cs idle runs
where the point is periodic.

| Lane | Mission | Points |
|---|---|---|
| A | LM | T4RUPT_PROGRAM: GPMATRIX 06,3132 (84), IMUMON 06,2172 (84), RRAUTCHK 06,3006 (83). SERVICE_ROUTINES: GENTRAN+1 02,3545 refused by `lm_service_routines`, because `tools/agc_sites_generate.py` takes `TC GENTRAN` sites but not the two `TC GENTRAN +1` sites in AOSTASK_AND_AOSJOB (20,3574 and 20,3615; no other `TC <callee> +n` site exists in either mission). INTERPRETER: INTPRET 03,2037 refused by `lanec_ev_lm_intpret`, Q+1 03,2737 |
| A | CM | T4RUPT_PROGRAM: IMUMON 06,2174 (84), OPTMON 06,2765 (83), OPTDRIVE 10,2000 (83). ALARM_AND_ABORT: ALARM 02,3650 (native code, no routine table). INTERPRETER: INTPRET 03,2006, Q+1 03,2706, and in V37E00E NOIBNKSW, SWSTORE, VLOAD+3, NEG4+1, TSSL+5, OPJUMP+2, VSHR2+11, VPDVL+3, NEWOPS+1, DSQSUB+15 |
| B | LM | INTEGRATION_INITIALIZATION: INTSTALL 13,3414/3415 and GOBAC+1 13,3464 refused by `lm_integration_init_*` |
| B | CM | INTEGRATION_INITIALIZATION (V37E00E): STATINT1, INTEGRV, MOVEPCSM, MOVEACSM, ATOPCSM+6, MOVEPLEM, ATOPLEM+6, MOVEALEM, ANDOUT+5/+7, GRP2PC+4, INTWAKE+11, WAKE1+4, INTSTALL, SETIFLGS, ENDINT, ENDINT+4/+11 |
| C | both | PINBALL_GAME_BUTTONS_AND_LIGHTS: CHARIN, LODSAMPT, ENTER, MMCHANG, MMCHANG+15, REQMM+10, 2BLANK, RELDSP, DSPMMJB, NVMONOPT, GOVNUPDT, UPDAT1+2, FLASHON, FLASHOFF, BLANKSUB, ENDIDLE; CM also DSPA, DSPOCTIN, OCTBACK, DSLV+1 |
| D1 | CM | KALCMANU_STEERING: STOPRATE 02,2526 refused by `cm_kalcmanu_steering_basic` |
| D2 | LM | AOSTASK_AND_AOSJOB: 1/ACCSET 20,2447, 1/ACCS+2, 1/ACCS+3, 1/ACCJOB+2, GETAOSUV+6, DOACCFUN+4. P20-P25: CLRADMOD 03,2011 |
| F | LM | RCS_FAILURE_MONITOR: RCSMON 06,3156 (83). P40-P47: ENGINOF1, ENGINOF3 (+4 to +21), ENGINOF4 refused by `lm_p40_p47_basic` |
| unallocated | CM | RTB_OP_CODES: LOADTIME 22,3510, SGNAGREE 22,3544 |

## Next (phase 2)

- Fit the store (steward decision above), then the same scenarios in VICE
  from the REU store.
- Give the flight resident the interpreter fetch point's boundary word when
  it resolves interpretive orders.
- Cut the cost of a cached module call (about 8,000 cycles) and plan the
  swap budget around the measured load cost.
- Rerun after each lane fills a point in; the target prints the remaining
  list.
