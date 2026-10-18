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
image into `<m>.reu` at the rope base (`tools/flight_store_build.py`
`FLIGHT_ROPE_BASE`, 0x1e0000 for either mission: a flight store carries one
rope, `include/rope_image.h` under `RK_LM` or `RK_CM`) and pads the store to
2 MB. Builds that carry both ropes (`tools/rope_image_pack.py` `BASES`) keep
the LM's at 0x1c0000.

The 2 MB REU, by address:

| REU range | Use |
| --- | --- |
| 0x000000 up | The module store (header, index, resolver, modules); then the module-test harness's case blob, below 0x1df000 (`REU_LIMIT`) |
| 0x1df000-0x1dffff | Lane C Pinball trampoline's state save (`LANEC_PIN_REU_SAVE`, `src/lanec_pin_trampoline.c`) |
| 0x1e0000-0x1f1fff | The mission's rope image |
| 0x1f2000-0x1f2fff | The flight undo image (4 KB, one `apollo_overlay_state`) the five FLIGHT_INTERP_STOP modules stash before a run and fetch back on a refusal (`FLIGHT_UNDO_REU`, `include/flight_undo.h`; the build checks that the rope image ends at or below it and that it is unused) |
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
The LM trim gimbal module also carries PAXFILT +4 and CLOSEOUT (one-entry
hot tables of the P- and Q,R-axis files, `CO_LOCATE_HOT`): a powered-flight
DAP pass that runs ACDT+C12 or GTS goes on to them there, instead of
loading the axis file's module. With POWERED_FLIGHT_SUBROUTINES' DMP and
DMPSUB run in place (docs/INTERP-COMPILE.md `native_calls`), the P63 window
from 90000 cs went from 640 to 566 job-slot loads in 300 cs and the P66
window from 123000 cs from 937 to 855 (docs/LM-FLIGHT-LOAD-PROFILE.md).

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

The LM idle loop's own module (`lm_self_check_idle`, 0x010A) carries only
POSTJUMP, the one interbank routine the loop calls (CHECKNJ's TC
POSTJUMP): `lm_interbank_postjump_hot_routines` in `src/lm_interbank_dap.c`,
checked against `lm_interbank_routines` by `tests/test_lm_interbank_dap.c`.
The module drops from 10,287 B to 2,392 B. Every interrupt taken from idle
reloads it, so in VICE the LM DAP hold goes from 10.52x to 9.82x real time
and firing from 10.73x to 10.04x, with the same job loads (hold, firing, and
the P63 descent-thrust scenario: 1/21/6/16 per T3/T4/T5/background before
and after). The host flight run lists the hot table before
`lm_interbank_routines` (`CO_LOCATE_HOT`). Its swap model does not model
RIDE tables, so it now counts the DAP's IBNKCALL (which rides the excursion
on the C64) as a swap: `lm-p00-idle` reads 77.2 loads a second, up from
57.2, and the non-work scaled rows shift. The work rows and their 0
mismatches are unchanged; the C64 counts are the ones to go by.

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
each as large as fits: `pack_chunks` grows a chunk 1, 2, 4, ... routines
until one does not fit, then cuts the gap with a batch of parallel trial
links, so the chunks are the same on every build (`make test-flight-chunks`
checks it against a brute-force greedy packing). Files in `GREEDY_SPLIT`
(the CM entry DAP) find the same chunks by binary search on each chunk's
end: a pass runs that table front to back, so fewer and larger chunks mean
fewer loads per pass. A chunk holds only its own
file's sources and operand sites; a table co-located into another module is
never chunked. Chunks that fit are kept even when others do not; a single
routine still over its slot is reported as a failed entry
(`one routine over its slot`, with its `routine_key`), and a file more than
64 KB over its slot is `too big to split` unless `SPLIT_ANY` names it (Lane
C's Pinball routines, whose try-lists each fit). A stem in `CHUNK_ORDER`
packs its routines in its own order instead of by key, and each chunk is
then a set of entries (`FS_PICK`): Pinball's (`lanec_pin_order`) are grouped
by the modules their try-lists run, so a chunk links few of them (on LM this
took the store image from 2.11 to 1.85 MB). `LANEC_PIN_GROUPS` puts three
display chains first, each in a chunk of its own (`CHUNK_BREAKS`: no chunk
crosses a group's end): the per-row loop DSPDCPUT, DP1OUTSF, DP3OUTSF,
DSPDCEND, DSPDCEND +2; the monitor chain NVMONOPT through NVSUBEND and
VERBFAN; and ENTPAS0 with MIXNOUN. A monitor display walks these about 20
points in turn, so with them together the LM P66 window from 123000 cs makes
2.9% fewer job-slot loads (840 to 816 in 300 cs) and runs 1.5% faster
(docs/LM-FLIGHT-LOAD-PROFILE.md). The build prints, per
mission, the chunk count, their mean fill and the store image against the
rope image above it (`report.json`: `chunks`, `chunk_fill`, `image_bytes`,
`rope_base`). Before #211-era step-machine splits and
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
  The idle loop rides in it too (`lm_self_check_idle_hot_routines`, and
  POSTJUMP as `lm_interbank_postjump_t4_hot_routines`, the same entries as
  the idle module's), with the interrupt lead-ins
  (`lm_lead_in_hot_routines`, the resident wrapper $01E1's entries). The
  idle loop then runs on after a T4RUPT taken in it, and the next interrupt
  starts there, with no load. These are ride tables (`RIDE`): the resolver
  still sends their keys to their own modules. With them the pack is
  10,459 bytes.
- The *CM entry pack* (`ENTRYPACK`, from module $07F7) packs the 26
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
  to 465 K cycles. ATTRATES+66 (15,2753) joined it later (27 entries, Lane
  A2): from about 21000 cs every pass ran it from a full-table chunk between
  two pack routines, two job-slot swaps a pass. On c6bcf271 the 500 cs
  windows from 21000, 25000, 33000 and 41000 cs went 4.1-4.2% faster
  (16.06x to 15.40x from 33000); the window from 17000 cs, before that
  point is reached, is unchanged. SPSIN and SPCOS ride in it too
  (`cm_sp_hot_routines`, the resident's `cm_sp_routines` entries; `RIDE`):
  a pass calls them 7 times, and each call used to end the pack's module
  run, run the resident's copy as a step of its own and start the run again
  at the return. With them in the pack the run carries on through each
  call. On d39f68d6 the windows from 17000 and 33000 cs (500 cs) went from
  14.388x to 13.753x (−4.4%) and 14.675x to 14.053x (−4.2%). The pack is
  one 14,890-byte module, still charged 13 MCT a step; the resident is
  unchanged (gc-sections drops the unused copy).
- The *CM docked RCS DAP packs* (`RCSPACK`, `JETPACK`, `MANPACK`, Lane D1)
  pack the entries one docked RCS DAP pass (T5RUPT) reaches after an SPS
  burn: `cm_rcs_dap_hot_routines` (12), `cm_jet_select_hot_routines` (21)
  and `cm_auto_man_hot_routines` (6). SMALLMP and DPADD
  (`cm_mysubs_routines`) ride in the RCS pack, since the RCS pass calls
  them every pass. They are `ALONE` like the entry pack: one pack of all
  three files is far over the job slot. A one-table pack splits into
  consecutive chunks (`FS_CHUNK`), and a pack of several tables into
  consecutive runs of its entries in key order (`FS_PICK`). The RCS and jet-select packs split into two
  chunks each. A pack's keys resolve to the pack first, so those hot keys
  load the pack and the full tables' chunks keep the rest (`HOT_FULL`).
  Pack ids follow on: a pack starts after the ids the one before it split
  into, and the build fails if one passes 0x07FF (0x0800 on is lane G's). On 8d547e12 the cm-p40 window from 70660 cs
  (300 cs) went from 21.96x to 16.03x real time, with T5RUPT job loads 635
  to 364 and its mean cost 568,910 to 374,331 cycles per pass (90 passes).
  Moving SMALLMP into the RCS pack (on a8203cc4 with T6 running, #649)
  took that window from 18.83x to 14.60x, with T5RUPT job loads 364 to 176
  and its mean cost 365,047 to 283,208 cycles. Steps fell from 1,722 to
  1,015 since more of a pass runs inside one module. The T5 pass count
  also moved (90 to 86): the work clock charges a module's steps at that
  module's MCT, so the two runs are not step-for-step the same 300 cs.
  The jet-select pack then split inside the T6 program (T6START and the
  chain after it, 17,3617 to 17,3713), so each T6RUPT loaded its two
  chunks by turns. `SPLIT_BEFORE` starts a chunk at a given key: the jet
  pack's second chunk now begins at T6START and holds the whole T6 chain.
  On a090a136 (cm-p40 from 70660 cs with `--replay`, 308 cs) that took
  T6RUPT from 173 job loads to 29 (one a pass) and its mean cost from
  253,509 to 90,897 cycles, and the window from 11.79x to 10.22x real
  time; every other interrupt's counts are unchanged.

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
class 3 (`cm_self_check_idle_routines`), so those routines run without
loading a module, with POSTJUMP beside it (`cm_interbank_postjump_hot_routines`
in `src/cm_interbank_postjump.c`, checked against `cm_interbank_routines`
by `make test-cm-interbank-postjump`; `PIN_HOT`). The idle loop's CHECKNJ
calls POSTJUMP every pass, and before, each call loaded the interbank
module, which also evicted the module the interrupted job runs in. The
pinned copy runs only with the pinned module's operands (the idle file's
one POSTJUMP site), so another caller's POSTJUMP is refused there and falls
to the resolver, which loads `cm_interbank_routines` as before; the host
run (`tools/flight_host_run.c`) gives a point a pinned table refuses to the
next holder in the same way; on LM that loop is an ordinary job-slot module, as
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
   the LR antenna in position 1 as the core has it (`FLIGHT_CH33=036733`)
   and moving to position 2 5 s after LRPOS2 commands it at high gate
   (`FLIGHT_LR_POS2=500`), and `FLIGHT_ENGINE=15100`;
   the keys are those of `lm-p63-descent`. The LLGE flies the braking
   phase to P64 at about 1,118 s after the core (on yaAGC alone too). The
   radar reads end (the resident's radar cycle requests RADARUPT, as
   yaAGC does), with no data good, so no LR update is taken.
   `lm-landing-ascent-nassp`: `lm-p63-nassp` on to the surface and back to
   orbit. `FLIGHT_ENGINE_LAG=0.2`: the throttle command reaches the engine
   as pulses at 3,200 a second and the thrust lags them 0.2 s, and the
   PIPAs count at every native routine boundary (THROTTLE's FWEIGHT
   assumes all three; without them P66's throttle commands alternate and
   grow). At 1,225 s (about 140 m, 4.5 m/s down) the crew goes to attitude
   hold (channel 31 bit 13) and clicks the rate-of-descent switch (key M,
   channel 16 bit 6), so P66 starts and holds the sink rate; ten more clicks
   bring it to 0.8 m/s, and the crew tilts the LM to null the 34 m/s over
   the surface by 1,280 s (`FLIGHT_PILOT=8`: the thrust axis tilts against
   the LLGE's surface velocity, V plus DELVS, at most 15 degrees).
   Touchdown at 1,344 s (`FLIGHT_TOUCHDOWN`: contact stops the LM, and the
   PIPAs get the surface's reaction, GM/r^2 up along RN, so the navigation
   holds still). V37E68E: P68 stores RLS and flashes V06N43; PRO (channel
   32 bit 14): GOTOPOOH; V37E00E: P00 on the surface. V37E12E: P12, TIG
   102:53:00 on the AGC clock loaded on V06N33 (the flight's was 124:22),
   proceed on V06N76 and on V99; the ascent engine (`FLIGHT_ASCENT=4888`,
   15,569 N, no throttle) burns about 7 min 15 s to orbit insertion at
   1,679 m/s, about 18 km up. On the host every native routine matches
   yaAGC; the unregistered points are the 3 self-check ones and Pinball's
   RELDSP1 (02,2502, 1 arrival).
   `lm-p65-autoland-nassp`: the same descent with no crew takeover. P64's
   guidance reaches its low gate at 1,278 s and starts P65, which holds
   the LM vertical and descends at about 1 m/s; touchdown at 1,324.8 s,
   where the engine model's true height (`FLIGHT_LR_TRACE`) reaches the
   surface; P68 and P00 on the surface as above. Apollo 11 never flew
   P65 (the crew took P66 at about 500 ft).
   The landing radar runs here too (`FLIGHT_LR=60000`, no V57): R12's
   range reads go to low scale (no /5, SKALSKAL on the Doppler term)
   from 1,171 s and data good ends at 10 ft.
   `lm-p67-nassp`: `lm-landing-ascent-nassp` to P00 on the surface with
   the crew on manual throttle (channel 30 bit 5) from 1,240 s to
   1,244 s in P66: GUILDENSTERN starts P67, which issues no throttle
   commands (the thrust holds), and starts P66 over at the current sink
   rate when the discrete goes. Touchdown comes 10 s earlier, at 1,334 s.
   `lm-doi-nassp`: Apollo 11's descent orbit insertion from the same core.
   lm_doi.py propagates the core's LM (after DOI, on the 15 x 114 km
   descent orbit) back to that orbit's apolune and undoes the burn there
   (76.4 ft/s along the velocity), so the LM starts on the 114 x 117 km
   pre-DOI orbit with DOI at TIG 102:34:00 (lm-doi.load). V37E30E: P30 with
   TIG and -76.4 ft/s in N81 loaded; N42 shows 61.4 x 8.1 nmi. V37E40E:
   `FLIGHT_POKE` puts the CDUs and the DAP's desired CDUs at R60's N18
   angles (the crew's maneuver) and ENTER skips the auto maneuver; PRO on
   V99N40 at TIG - 5 s. The descent engine model stays at 10% (no
   throttle-up), so the burn is 75 s, not the flight's 30 s; cutoff leaves
   VG 1.1 ft/s and N85 1.0 ft/s. PRO through V16N40 and N85, V37E00E, then
   V82 (this vehicle): 61.4 x 8.8 nmi. `FLIGHT_DSKY=1` prints the DSKY at
   each change, which is how the key times were found. On the host every
   native routine matches yaAGC; the unregistered points are the 3
   self-check ones, Pinball's RELDSP1 and BANKCALL from `TCR BANKCALL`
   sites (#587).
   `FLIGHT_RCS=KG` (LM) is the RCS jets' translation for a KG kilogram
   vehicle: 444.8 N a jet along the axis the DAP's jet tables give it
   (channel 5 even bits +X, odd bits -X; channel 6 bits 5 and 8 +Y, 6
   and 7 -Y, 1 and 4 +Z, 2 and 3 -Z; rotation couples cancel), counted
   to the centisecond at each native routine boundary into PIPAX-Z
   through CDUX-Z. No attitude dynamics. The crew's translation hand
   controller is channel 31 bits 7-12 (+X, -X, +Y, -Y, +Z, -Z, active 0).
   `lm-p47-nassp`: P47 from the lm-doi-nassp start; 10 s of +X (two jets)
   reads 1.9 ft/s on N83. `lm-p41-nassp`: lm-doi-nassp's P30 with a
   5.0 ft/s retrograde trim, done by RCS in P41: V16N85 shows body VG
   4.6, 1.8, -0.8 ft/s at TIG and the crew's +X, +Y and -Z translations
   null it to 0.1, 0, 0. `lm-p42-nassp`: the DOI burn on the ascent
   engine after an abort stage at 1.5 s (`FLIGHT_STAGE`, `FLIGHT_ASCENT`)
   and V48 (DAPDATR1 11002: ascent stage, which sets APSFLAG; 10,776 lb
   on N47); the APS burns about 6.6 s, and with no attitude dynamics the
   steering does not turn the LM, so cutoff leaves N85 at 5.3, 14.3,
   -0.1 ft/s. All three end in P00 and match yaAGC at every native
   routine.
   `lm-abort-p70-nassp` and `lm-abort-p71-nassp`: `lm-p63-nassp` with the
   crew aborting in P64 at 1,150 s after the core (about 1,230 m up,
   28 m/s down), with `FLIGHT_ENGINE_LAG=0.2`. `FLIGHT_CHAN` clears one
   bit of the core's channel 30 (036351; the rest of it, PGNCS mode and
   the IMU, must stay as it is): bit 1, ABORT, for P70, or bit 4, ABORT
   STAGE, for P71. R10,R11 reads it on its next pass and starts the
   program, which restarts through ENEMA into the abort guidance. P70
   keeps the descent engine (THROTUP's BIT13 THRUST command takes it to
   full thrust, which the engine model gives in MODREG 70) and cuts off at
   about 1,408 s: 1,667 m/s, 18.3 km up, climbing 9 m/s. P71 stages
   (`FLIGHT_STAGE`: from the abort the engine is the ascent engine of
   `FLIGHT_ASCENT=4888`) and cuts off at about 1,579 s: 1,678 m/s, 18.3 km,
   climbing 7 m/s. The flight store resolves P70-P71's basic keys to
   lm_p70_p71_file, the file's whole-file certificate, before the older
   lm_p70_p71 tables (flight_store_build.py `rank`), since lm_p70_p71
   refuses THROTUP's WOR CHAN14. On the host every native routine matches
   yaAGC in both; the unregistered points are the 3 self-check ones and
   Pinball's VBPROC+3 to +5 (40,3445-3447, 2 arrivals) and RELDSP1.
   `lm-p63-overload` and `cm-entry-overload`: the Executive overload
   path, the 1201/1202 alarms of Apollo 11's landing. The flight's cause
   was the RR CDU counts: counter cycles the LGC lost, about 13% of it in
   P63. `FLIGHT_STEAL=T0-T1:N[,...]` models that. From the first clock
   period at or after T0 to the last before T1, each period (each
   work-clock T5 step) also spends N MCT that no routine runs in. The
   counted cells are left alone. `lm-p63-overload` is
   `lm-p63-descent-thrust` with V16N68 at 180 s and 420 MCT a centisecond
   stolen from 185 s to 240 s. This synthetic P63 runs lighter than the
   flight's (no landing radar reads), so it takes more than 13% to
   overload it. SERVICER falls behind: FINDVAC finds no VAC area (alarm
   1201, 196 s), then NOVAC no core set (1202, 210 s). Each is a BAILOUT
   that restarts through the phase tables. V06N63 comes back after each
   one, and the V16N68 monitor does not, as in the flight. Once the steal
   ends, N63 updates every 2 s again and P63 flies on. `cm-entry-overload`
   is `cm-entry` with 600 MCT a centisecond stolen from 210 s to 270 s,
   over P64 and P67: alarm 1201 three times, and the entry still reaches
   P67. `FLIGHT_WATCH=0375,0376,0377` prints FAILREG as it changes. On the
   host every native routine matches yaAGC in both.
   `cm-p52`: Apollo 11's first IMU realignment, in earth parking orbit at
   about 00:44 GET on stars 30 (Menkent) and 37 (Nunki), the pair the crew
   used. cm_p52.py writes cm-p52.load (the launch REFSMMAT, the CSM in a
   100 nmi orbit in the launch plane, held still with the sextant's shaft
   axis between the two stars, the platform drifted 0.012, -0.021 and
   0.009 degrees from REFSMMAT, TEPHEM for the 13:32 UTC liftoff, the sun
   and moon pads) and cm-p52-star1.load and cm-p52-star2.load, the CDUS
   and CDUT that put each star on the reticle as the drifted platform sees
   it. The optics switch starts at ZERO OPTICS and goes to CMC at 40 s;
   `FLIGHT_CHAN` turns it to MANUAL and back for each mark, and the key M
   is the MARK button (channel 16 bit 6 and KEYRUPT2). V37E52E, option 3,
   PICAPAR picks stars 30 and 37 itself, R52 computes the optics angles
   (its shaft angles agree with cm_p52.py's to 2 counts, its trunnion
   angles to 11, the drift), R53 takes one mark per star, N05 shows a
   star angle difference of 0.00 and N93 torquing angles of -0.012,
   +0.022 and -0.010 degrees, the drift taken out; ENTER on V50N25 00014
   and V37E00E end it in P00. Two yaAGC hardware models stop under the
   frozen clock for it (section 10): the optics drive, which zeroes CDUS
   and CDUT, and the gyro drive, which empties GYROCMD.
   `lm-p57`: the lunar-surface alignment at 103:40 GET, about 55 minutes
   after Apollo 11 landed at 0.674 N, 23.473 E, on Rigel (12) and Navi (3).
   There is no core this late, so lm_p57.py builds one: lm-p57.load (the
   landing REFSMMAT, the platform drifted 0.021, -0.014 and 0.017 degrees
   from it, the LM on the surface yawed 13 degrees left of the approach
   azimuth, SURFFLAG and LMOONFLG set, RLS, GSAV, TEPHEM and the AOT detent
   pads) and lm-p57-star1.load and lm-p57-star2.load, the CDUs at each
   mark, since the moon turns the LM under the platform. V37E57E, option 3,
   technique 2 (two stars), V21 star codes; R59 picks detents 6 and 4
   itself and its cursor and spiral angles for Rigel agree with
   lm_p57.py's to 0.002 degrees. Each mark is the key x (channel 16 bit 3,
   the AOT X MARK, and KEYRUPT2), then V21/V22 load the cursor and spiral
   lm_p57.py computed; SURFSTAR, then N05 shows 0.00 and N93 shows
   -0.021, +0.012 and -0.019 degrees, the drift taken out; V50N25 00014,
   V34 and V37E00E end it in P00. The keys y and j are the Y MARK
   (bit 4) and MARK REJECT (bit 5).
   `cm-p23`: cislunar midcourse navigation at 24:30 GET, one star 2 /
   earth near-horizon measurement. cm_p23.py writes cm-p23.load (the CSM
   180,000 km out on a two-body ellipse from TLI in the launch plane, the
   CMC's state vector 76 km off the true one, the launch REFSMMAT, the
   shaft axis held on the horizon toward the star, the sun and moon pads,
   WMIDPOS, WMIDVEL and HORIZALT) and two optics loads: cm-p23-cal.load,
   the star line 0.011 degrees from the landmark line for R57's
   calibration mark, and cm-p23-mark.load, the star on the horizon as the
   true trajectory sees it at the mark. V37E23E, star 02 and horizon code
   00110 on N70, ENTER on V50N25 (no auto maneuver), R57 (V59, MARK,
   TRUNBIAS 4 counts), R52 and one R53 mark, then POINTAXS integrates the
   state to the mark time, INCORP1 shows about 20 km on V06N49 (its
   measured-minus-estimated angle agrees with cm_p23.py's 0.0084 degrees
   to the trunnion's resolution, and the update moves the state toward the
   true one), and proceed incorporates it; P00.
   `cm-p40`: an SPS burn with LOI-1 (75:49:50 GET) as the reference
   case. cm_p40.py writes cm-p40.load: the docked CSM+LM (43,900 kg) on
   a hyperbola retrograde to the moon's orbit, 2,060 km out at the load,
   pericynthion on the far side half a burn after TIG (300 s); the
   external DV (V06N81) 889 m/s against the TIG velocity; the SPS pads
   S40.13 needs for a finite burn (WEIGHT/G, EK1VAL-EK3VAL, FANG, EMDOT;
   without them P40 treats the burn as impulsive and cuts off early); the
   TVC DAP data (N46, N47); a platform with X along the DV; and the CDUs
   at the ignition attitude S40.1 gives (the DV turned back by half the
   burn arc, about 18.5 degrees of inner gimbal with the bell biases), so
   R60's maneuver is nil. G&N AUTO (channel 31). V37E30E and proceed on
   N33, N81, N42 and N45; V37E40E, proceed and ENTER on V50N18, proceed
   on V50N25 00204 (the gimbal test), proceed on V99N40 at TIG-5. The
   harness's FLIGHT_SPS (tools/flight_host_run.c, CM) is the engine:
   from ENGONFLG it thrusts 91,188 N along the bell (body X turned by
   P40's PBIAS and YBIAS) and burns mass at 3,080 m/s exhaust velocity,
   counting the velocity into PIPAX-Z; with no attitude dynamics the
   CDUs follow the RCS DAP's CDUXD-ZD until ignition and hold from then
   on (the TVC DAP reuses those cells). The burn runs about 372 s to
   cutoff with VG 0.8 m/s; proceed on V16N40 and V16N85, then P00.
   The docked RCS DAP's attitude filter calls MYSUBS's SMALLMP (TC
   SMALLMP, 21,2042), which no earlier scenario reached:
   src/cm_mysubs_routines.c puts the certified SMALLMP and DPADD bodies
   (src/cm_mysubs.c) and their TC Q in the flight store as
   `cm_mysubs_routines`.
   `cm-p11`: Apollo 11's launch, 16 July 1969 13:32:00 UT, from the pad
   to earth orbit insertion. cm_p11.py writes cm-p11.load, the pad loads
   P01, P02 and P11 read: launch complex 39A (LATITUDE, PADLONG), the
   72.058 degree flight azimuth (LAUNCHAZ) and the vehicle's 90 degrees on
   the pad (AZIMUTH), TEPHEM so that the clock at liftoff is the launch
   time, and the attitude error polynomial (zero: the FDAI holds the
   liftoff attitude). PGNCSALT, UNITW and AZO are illustrative; no listing
   gives the flight's values. Two harness models (tools/flight_host_run.c,
   CM) stand in for the launch vehicle: `FLIGHT_IMU=1`, the gimbals (zero
   while channel 12 bit 5 is on; the coarse align drives the CDUs to
   THETAD), and `FLIGHT_SATURN=CS`, liftoff at that time (channel 30 bit 5
   goes active) and the boost's specific force in PIPAX-Z: the S-IC (35.1
   MN, inboard cutoff at 135 s, cutoff at 162 s), the S-II to 548 s and
   the S-IVB to 823 s, along a pitch program in the launch plane from
   vertical at 12 s to 93 degrees. cm_p11.py integrates the same boost
   and prints the trajectory: insertion at 142 x 218 km and 7,786 m/s
   (the flight's was 183 x 186 km). V37E01E at 20 s: P01 turns the IMU
   on, zeros and coarse aligns it, then P02's gyrocompass loop; liftoff at
   100 s: P11 zeros the clock, sets up REFSMMAT from LAUNCHAZ and shows
   V06N62 (inertial speed, climb rate, altitude) every 2 s; at insertion
   25,555 ft/s and 102 nmi. V82 in P11 shows V16N44, apogee 115.8 and
   perigee 81.5 nmi; proceed, then V37E00E: P00. Times after liftoff,
   keys, the run's length and flight_scenario_vice.py's `--snap-cs` alike,
   count from the zeroed clock (a `--snap-cs` under 10000 is on the pad,
   in P02). On the
   host every native routine matches yaAGC; the unregistered point is
   Pinball's RELDSP1 (02,2516, 1 arrival, on V82).
   `cm-p20`: the CSM's side of the LM rendezvous, P20 tracking the LM with
   SXT marks and VHF ranging. cm_p20.py writes cm-p20.load from the orbits
   of lm_rendezvous.py (Lane D2's LM P20/P34 scenario): the CSM in a 60 nmi
   circular lunar orbit, the LM 15 nmi below it and 2.1 degrees behind,
   both retrograde about the reference Z axis as Apollo's were; REFSMMAT
   the LM's local vertical; the CMC's LM state 1.8 km too far along the
   line of sight; R22's pads (WRENDPOS, WRENDVEL, RVAR, RMAX, VMAX); the
   CSM at R61's preferred tracking attitude. `FLIGHT_RR`'s CM side
   (tools/flight_radar.h) holds the CDUs on the DAP's desired angles
   (CDUXD-ZD; Comanche keeps them two words apart), points the SXT at the
   LM, and puts the range in RNRAD when a VHF read starts (channel 13
   bit 4; the resident timer ends the cycle with RADARUPT). The keys run V37E20E, V87E (VHF ranging
   allowed), V57E and MARK, proceeding on V51 (TRKMKCNT 1: the mark goes
   into the LM state and the W matrix). R22's first VHF range, at about
   125 s, shows V06N49, the 1.8 km. Proceed takes it (VHFCNT 1). The
   next, at about 185 s, shows N49 again for a 4 ft/s velocity change,
   and proceed takes that too (VHFCNT 2). R22 waits on N49 in a loop
   (RENDISP7), so the AGC never idles. `FLIGHT_BUSY_SAMPLE=100` gives the
   keys their sample points: a key every second, so each CHARIN job runs
   before the next key comes. On a busy stretch with all four keys at
   once, V87 arrived as V77 and cleared PRFTRKAT.
   On base 1a0ba510 the native run (25,000 cs, scaled x1) takes 651,427
   routines, none that yaAGC did not match. The one refused point is
   V87's interpretive SET (EXTENDED_VERBS.agc SETVHFLG +1, 43,3024),
   which cm_extended_verbs_file_interpretive does not hold.
   `lm-rendezvous`: the LM's side, P20 with rendezvous radar tracking and
   then P34 TPI targeting. lm_rendezvous.py writes lm-rendezvous.load:
   the same two orbits, the AGC's state vectors on them, R22's pads and
   P34's TTPI and ELEV. R22's variances (RANGEVAR, RATEVAR, RVARMIN,
   VVARMIN) are loaded: left zero, the gain blows up and every mark asks
   V06N49. WRENDPOS is integer metres (SR 5 into W's B-19). P34's N37
   shows TTPI, not TIG. The orbits have to be retrograde: for the moon
   INITVEL takes Lambert's direction from the sign of the orbit normal's
   Z, so a posigrade pair solves TPI the long way round (3,215 m/s).
   `FLIGHT_RR`'s LM side points the RR at the CSM and answers range and
   range-rate reads. The keys run V37E20E, proceed and ENTER on R61's
   V50N18, and proceed on R21's V50N72 lock-on. R22 then marks every 61 s
   (6 marks by 340 s, no N49). V37E34E comes at 290 s: proceed on N37,
   N55 and N45. P34 solves TPI for 26.6 degrees elevation (N58: 7.4 m/s;
   an independent Lambert solve gives 7.5). Proceed on N37, N58 and N81,
   ending on N59. `FLIGHT_BUSY_SAMPLE=50` gives keys sample points while a
   job busy-waits. On base 03e28d4a the native run (40,000 cs, scaled x1)
   takes 1,273,194 routines, none that yaAGC did not match.
   On the C64 (`flight_scenario_vice.py --keys`), one gap shows only
   there. CONIC_SUBROUTINES' CHECKCTR (12,2644) is KEPLOOP's and LAMBERT's
   RTB target. On the host it runs only inside lm_conic_compiled's routines
   (a native range), so no host run reaches it as a basic point. On the C64
   the interpreter module runs those orders, and P34's TPI recycle stopped
   there. `lm_conic_checkctr_routines` is its basic table: the same
   lm_conic_checkctr body, leaving Z at DANZIG for the interpreter's pending
   BHIZ. With it, on base 3384c02b, five windows between P20's start and N59
   run 4,000 cs in all at 20.2x real time with no stop, ending on DELVTPI
   7.38 m/s as on the host.
   The rest of the LM's rendezvous programs. lm_coelliptic.py writes the
   loads for the targeting before TPI, shaped like Apollo 11's coelliptic
   sequence: the CSM in a 60 nmi circular orbit 9.5 degrees ahead, the LM
   in a 9 x 45 nmi orbit ten minutes before apolune (CSI there), TTPI 97
   minutes after apolune. `BURN=` and `CSMBURN=` load a vehicle's state as
   if a burn had already been made, so P33 and P73 start after their CSI.
   `lm-csi` runs P32 (CSI) and then P72 (the CSM-active CSI): DH 16.5 nmi,
   CSI 48.4 ft/s forward; for P72, 49.9 ft/s back. `lm-cdh` runs P33 (CDH)
   after P32's burn, and `lm-cdh-csm` runs P73 after P72's. The crew
   loads N55's NN as 1, because P32 zeroes it and with 0 finds no CDH
   (alarm 604). The CSM-active programs need ELEV 206.6 (180 + 26.6: the
   CSM looks down at the LM). With 26.6 P73 finds no TPI (alarm 611).
   `lm-rendezvous-tpm` is lm-rendezvous with V85 (RR line of sight) and
   V83 (R31: range, range rate, theta) during P20. After P34 come P35
   (TPM), P74 and P75 (CSM-active TPI and TPM), P76 (target delta V: the
   CSM's state takes a 10 ft/s burn), then V56 (end tracking).
   `lm-stable-orbit` runs P78 (CSM-active stable orbit rendezvous, SOI,
   CENTANG 130) and then P79 (stable orbit midcourse), from
   lm-rendezvous's P20. `lm-stable-orbit-lm` runs the same with P38 and
   P39. P78's and P38's Lambert solutions take about 200 s of AGC time
   after N45.
   The CSM's side: Comanche's P32-P35 and P38/P39 have the CSM active.
   P72-P79 have the LM active, with the CMC solving for it. P17 and P77
   are the TPI search. `lm_coelliptic.py CM` writes the same moments
   for the CMC. `cm-csi`, `cm-cdh` and `cm-cdh-csm` are keyed as the LM
   ones with the program numbers swapped, and give the LM side's solutions
   (CM P72 is LM P32; CM P32 is LM P72). The other four start from
   cm-p20's P20:
   - `cm-rendezvous-tpm`: V83, V85, P34, P35, P74, P75, P76, V56. P74,
     with ELEV 26.6, solves the LM's TPI as the LM's own P34 does.
   - `cm-stable-orbit`: P38/P39.
   - `cm-stable-orbit-lm`: P78/P79.
   - `cm-tpi-search`: P17 and P77.

   The CMC leaves P38's CENTANG at 0, so N55 is loaded with V23.
   `cm-tli`: Apollo 11's translunar injection at 02:44:16 GET, the S-IVB's
   second burn, as the CMC monitored it in P47. cm_tli.py writes
   cm-tli.load, using the same geometry as cm_p52.py: the CSM in the 100
   nmi parking orbit, the launch REFSMMAT with the platform on it, TEPHEM,
   the sun and moon, and the CDUs with body X along the burn. It also
   prints the burn's direction in stable-member axes for `FLIGHT_SIVB=CS,
   UX,UY,UZ` (tools/flight_host_run.c, CM): from CS, 0.9 MN on 140 t less
   215 kg/s for 347 s, inertially fixed along the orbit's velocity half a
   burn after TIG, into PIPAX-Z. Its own integration of the burn gives
   cutoff at 300 km and 10,861 m/s, C3 -1.4 km2/s2 (the flight's was
   10,834 m/s). V37E47E: P47 starts average-G and counts the velocity on
   V16N83 (body X passes 9999.9 ft/s and the display wraps: 3,166 m/s in
   all); proceed (P47 ends in GOTOPOOH, which flashes V37), V37E00E; V82
   in P00 shows V16N44 with apogee 9999.9 nmi (the display's limit) and
   perigee 101.6 nmi. P47BODY's interpretive block runs in the NOVAC job
   TIGON starts, so the cm-p40-p47 spec admits FIXLOC 0110 there
   (`novac`). On the host every native routine matches yaAGC; the
   unregistered point is Pinball's RELDSP1.
   `cm-p22`: lunar-orbit navigation on a known landmark, as Collins
   tracked landmark 130 before the landing (synthetic, about 98 h GET).
   cm_p22.py writes cm-p22.load: the CSM in a 60 nmi retrograde orbit
   passing over the landmark (1.20 N, 23.70 E, 1,500 m below the CMC's
   lunar radius) in the CMC's own moon frame (MOONMX with the rope's
   constants), the CMC's state 1.4 km off the true orbit, REFSMMAT and
   platform local vertical, the sextant's shaft axis on the landmark at
   mid-pass, and the W matrix pads. The mark loads put CDUS and CDUT where
   the true orbit sees the landmark at each MARK. N89 holds half the
   longitude (LLASRDA doubles it into LONG). V37E22E, proceed on V06N45;
   V22E10000E on V05N70 (landmark known, so N89), proceed; proceed on
   V06N89; R52 drives the optics on V06N92; the optics switch to MANUAL
   (FLIGHT_CHAN), MARK at 132 s and 152 s, proceed on V51; proceed on
   V05N71 and V06N89; V06N49 shows 0.6 nmi, proceed; the landmark
   solution on V06N89 (1.205, 11.847, -0.74 nmi), proceed; V05N70, then
   V37E00E. On the host every native routine matches yaAGC; the
   unregistered points are Pinball's VBPROC +3 to +5.
   `cm-tei`: Apollo 11's transearth injection at 135:23:42 GET, the SPS
   burn on the far side that left lunar orbit. cm_tei.py sets it up as
   cm_p40.py does LOI-1 (the same TVC and SPS pads, `FLIGHT_SPS`): the
   CSM alone (16,400 kg) in a circular 60 nmi lunar orbit, the platform
   with X along the DV, the CDUs at the ignition attitude R60 asks for,
   RLS for the altitude displays, and TEPHEM so the CMC clock's 300 s is
   TIG. The TEI-1 pad is keyed: V37E30E, V25 on V06N33 (TIG 300 s on the
   CMC clock), V25 on V06N81 (+3283.6 ft/s), proceed on N81, N42 (9999.9
   x 60.0 nmi) and V16N45; V37E40E, proceed and ENTER on V50N18, proceed
   on V50N25 00204 and on V99N40 at TIG-5. The SPS burns about 154 s to
   cutoff (N40 VG 6.1 ft/s); proceed on V16N40 and N85, V37E00E; V82 shows
   V16N44 9999.9 x 60.0 nmi, then V34E. On the host every native routine
   matches yaAGC; the unregistered points are Pinball's VBPROC +3 to +5 and
   RELDSP1.
   `cm-p37`: return to earth targeting on the transearth coast, about
   150 h GET, as the crew ran P37 for midcourse and abort pads. cm_p37.py
   loads the CSM inbound 240,000 km from the earth on an ellipse from the
   moon's distance with a natural -6.5 degree entry (outside the moon's
   sphere, so CMOONFLG clear), the platform in local vertical axes.
   V37E37E: V25 on V06N33 (TIG 1 h on the CMC clock), V22 and V23 on
   V06N60 (100 ft/s, -6.50 degrees; with zeros the conic iteration stops
   on alarm 00605), proceed. The conic solution shows V06N61 (20.52 N,
   36.79 W), N39 (31:07:43), N60 (36,054 ft/s, -6.50) and N81; the
   precision solution (about 170 s) shows 20.57 N, 37.51 W, 31:11:05 and
   36,060 ft/s, then V04N06 (SPS), the biased V06N33 and V16N45. V37E00E;
   V82 shows V16N44 9999.9 x 22.4 nmi, then V34E. On the host every native
   routine matches yaAGC.
   tools/flight_scenario_vice.py runs a flight scenario on the C64: the
   host run's native state at `--snap-cs` (FLIGHT_SNAPSHOT) loaded into
   the scenario mission's flight resident on its whole-program store in VICE
   (platform/c64/dap_pass_check.c, as tools/dap_pass_vice.py does), run
   for `--cs` centiseconds. The host takes the snapshot at the first point
   at or after `--snap-cs` with no interrupt pending, so inside a long
   interpretive job it can start late (a CM window asked at 9000 cs began
   at 9952). The run prints where the snapshot was taken, and says `slid`
   when that is more than 100 cs late (`snap_at` and `snap_slid` in its
   JSON); `--strict-start` fails such a window instead. The run records
   cycles, steps and module loads per AGC interrupt; a refused or untranslated point stops it. With `--keys` it
   presses the scenario's DSKY keys that fall in the window, at the
   centiseconds the host run presses them (channel 15 and KEYRUPT1, or a
   mark's channel 16 bits and KEYRUPT2; on the CM also rk_device's
   main_keys and navigation_keys, which each store module on a CM record
   writes back to those channels). Without it the window presses none.
   Other scheduled inputs (FLIGHT_CHAN, FLIGHT_POKE and FLIGHT_LOAD, the
   radar model) are not replayed. `--watch 1011,...` prints up to five
   erasable words as the window ends (1011 is Comanche's MODREG).
   Every run then checks its REU writes (tools/reu_layout.py): each byte
   that differs between the image the run started from and the one VICE
   writes back must lie in a region declared writable. The regions come
   from their sources: the store (0 to its image bytes) and the rope
   (`rope_reu_base`, 2 bytes a word) are read only; the Pinball save
   (src/lanec_pin_trampoline.c LANEC_PIN_REU_SAVE, 4 KB), the undo area
   (include/flight_undo.h) and the scratch (the last 32 KB) are writable;
   with `--replay` the rows are read only and the states after them
   writable. A write outside fails the run (`reu_bounds` in its JSON), and
   the store build fails when two regions overlap. A write of the bytes
   already there is not seen by that diff; the run's DMA trace covers it,
   and every read. A VICE monitor checkpoint on the command register
   ($DF01, `trace`, so it logs without stopping) computes each DMA's REU
   range from the controller's registers and logs it when it is not inside
   one span of declared regions; any logged DMA fails the run
   (`dma_outside`). VICE evaluates the condition itself, so an in-bounds
   run costs nothing. `--reu-check-drop REGION` leaves a region out, as a
   test of the check: with the scratch left out, an LM P63 window logs 48
   DMAs (the step records) and fails; with every region, it logs none.
   `--replay` replays everything instead and checks the result: the host
   run (`tools/flight_host_run.c` FLIGHT_REPLAY) goes on after the
   snapshot through the window on the C64's work clock, charging each
   table the weight the C64 store charges it (host.inc's, #650), and writes every input its models and schedules give the
   AGC there (the engine, SPS and aero models' PIPA counts, keys,
   FLIGHT_CHAN, pokes, the radar model) as 8-byte rows. They go at the
   top of the zeros between the store and the Pinball trampoline save
   (0x1df000), outside the store and the resident (before this they sat
   just below the rope, and the C64's states ran into the save, which
   every Pinball routine stashes 4 KB to). The C64 applies each row at the first step at or after its
   time where the host took it (a sample point, or any step boundary for
   the engine lag and radar reads). At the first sample point `--cs` in
   (and with `--replay-every N` at the first one at or after every N cs
   before), both sides write their erasable words and output channels (5,
   6, 11-14, 34, 35, the superbank; on the CM 11-14 from rk_device, which
   the base tables run on);
   the run fails on any difference, printing the first one, skipping what
   the host differential skips (the central registers and RUPTREG1-4;
   unlike it, the timers are compared). So a window that ran with no stops also has to match
   the host. Until the CM idle pass is charged as idle on the C64 (it is
   module work there when a job ends into it, idle on the host), CM windows
   skip what that tick moves: DSPCNT, DSRUPTSW, DSPTAB's sent bit and
   the clock of checkpoints a centisecond apart, and the whole of such a
   checkpoint when other words differ too, unless it ends the window
   (`SCHEDULE_SKIP`; the run prints
   `skipped:` when it uses it; `--replay-exact` turns it off). The timers
   TIME1-TIME6 are compared too. Until the work charges agree (Interpreter
   #650, RK #663), both missions skip a TIME5 one count apart when TIME1
   agrees: the two sides sampled in different thirds of the centisecond
   (`TIME5_PHASE`, printed when used). LM windows skip nothing else.

   **What "matches host" means** (whole-program acceptance, bar item 3).
   It is a chain of two checks on the same inputs:
   - *Host = yaAGC.* The scenario's host run (native C on the PC,
     `tools/flight_host_differential.py`) and yaAGC run the same rope,
     load, keys, channels and device models. At every sample point the
     erasable words are compared, all 2048 but three groups. The central
     registers and the interrupt save registers ARUPT-BRUPT (0-017) hold
     the interrupted step's working values. The
     clock counters (024-031) are left out because the two clocks are
     driven differently (FLIGHT_TIMERS compares them). RUPTREG1-4
     (070-073) hold only what the last interrupt left. A scoreboard row
     is "host clean" when this finds 0 mismatches over the whole scenario.
   - *C64 = host (`--replay`).* The C64 window starts from the host's
     snapshot and gets the host's inputs as rows. At the window's end, and
     every `--replay-every` cs, it is compared with the host on a wider
     scope than the first check: the same erasable set, plus the timers
     TIME1-TIME6 (both sides run one work clock here), plus output channels
     5, 6, 11-14, 34 and 35 and the superbank. A PIPA's -0 and +0 count as
     equal. A PIPA count the host gave at a sample point the C64 was still
     busy at goes in at the C64's next step, as the hardware counts whether
     or not the AGC is idle, and late rows go in before a state is written.
     Anything else is a difference, apart from the temporary skips
     above, which are named in every result that uses them. Output channel
     10 (the DSKY relay words) is not compared: it is a strobe the AGC
     writes once per relay word, not a latch, so the display is checked
     through DSPTAB in erasable instead.
   So the chain gives C64 = yaAGC only on the first check's scope. The
   timers and the output channels are checked C64 = host only.
   A window "matches host" when its `--replay` log has no FAIL line. It
   "matches exactly" when the log also has no `skipped` line.

   **Seams (`--chain K`, bar item 4).** One replay window is limited to
   16,383 cs, so a whole phase runs as K windows back to back. The host
   run goes on through all K windows. A window ends at the first sample
   point `--cs` or more in that has no interrupt pending. At that point
   the host writes the next window's snapshot (`-snap.h-1`, `-snap.h-2`,
   ...), taken after that point's inputs, which it also writes as `.seam`
   rows. It then starts the next window's work clock the way a C64 window
   starts its own. Each window runs on the C64 as above. Before window n
   runs, the C64's end state of window n-1, plus the seam rows, is
   compared with window n's snapshot on the `--replay` scope. So the run
   fails unless every window matches the host and every seam is
   continuous. A key or an interrupt request at a seam point fails the
   host run (exit 4); move the seam by changing `--cs`.
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
   `lm-p52-realign`: a P52 option 3 (REFSMMAT) fine align from the same
   core, on the pair its own last P52 sighted, stars 18 (Regulus) and 20
   (Gienah) in the forward AOT detent (BESTI 108, BESTJ 120, STARCODE
   0224). The keys run V37E52E, option 3 (V22E3E) and proceed on V04N06,
   ENTER on V50N25 00015 (no star pick: R52 offers BESTI, then BESTJ),
   proceed on V01N70, ENTER on V50N18 (no auto maneuver: with no rotation
   model KALCMANU would never finish), proceed on V01N71, a Y and an X
   mark and proceed on V54N71, for each star; then proceed on the star
   angle difference (V06N05) and the torquing angles (V06N93), and ENTER
   on V50N25 00014. The AOT marks are the keys `y` and `x`: channel 16
   bits 4 and 3 and a KEYRUPT2 request (as `M` and `J` are the CM's). For each
   sighting `FLIGHT_POKE` sets CDUX-CDUZ to the attitude that puts the
   star on the detent's optic axis (tools/flight_scenarios/lm_p52.py: the
   smallest rotation from the core's attitude, which is already star 20's
   to a count). So V06N05 shows 0.00 and V06N93 a few thousandths of a
   degree, and R55 torques the gyros (GYROCMD keeps its count on both
   sides: the frozen clock stops yaAGC's gyro drive, and the C64 has
   none). On base 30af9747 the native run (10,000 cs, scaled
   x1) takes 528,365 routines, none that yaAGC did not match, and ends
   in V37 after P52; the only points left unregistered are the
   self-check's SMODECHK and CHECKNJ, which the core's mid-idle-loop start
   reaches. On the C64 (flight_scenario_vice.py windows from host
   snapshots) the R52, R60, OPTAXIS, R54 and R55 work runs at 9.8 to 11.5
   times real time, and the AOT marks run; the windows from the marks on
   stop at PINBALL's NVMONOPT (unmapped in the store until #425).
   tools/flight_scenarios/sighting.py holds the conversions, catalog and
   gimbal matrices lm_p52.py uses.
   Off-nominal inputs: four scenarios repeat a nominal one with one
   input gone wrong. `FLIGHT_RR_DROP=T0-T1[,...]` holds the radar's data
   good high (not good) over those centiseconds whatever the tracker
   does (tools/flight_radar.h: channel 33 bit 4 on the LM, bit 2 on the
   CM); the reads still answer. `lm-rendezvous-rr-dropout` drops it from
   195 s to 210 s, over R22's third mark: the read fails (alarm 521,
   RADMODES data fail), R22 drops the lock and R21 flashes V50N72 again;
   proceed at 215 s, the RR locks on and the marks go on, one fewer, and
   P34 ends on N59 as before. `cm-p20-vhf-dropout` drops VHF ranging
   from 180 s to 190 s, over R22's second range: VHFREAD takes BADEND,
   so no N49 and no mark (VHFCNT 3 by 320 s, not 4), and the next ranges
   incorporate on their own. `FLIGHT_LR=CS[:DH]` is the LM landing radar (tools/flight_lr.h).
   From CS, once HCALC is set, it takes the LM's state from RN and VN
   (through REFSMMAT) and flies its own trajectory: lunar gravity plus
   the engine model's thrust, so truth and the LGC's navigation part as
   the navigation errs. The surface is the landing site's radius
   (/LAND/) plus DH metres. Data good (channel 33 bits 5 and 8) holds
   below 50,000 ft of slant range and above 10 ft, until touchdown and
   outside `FLIGHT_LR_DROP` windows; bit 9 gives the range scale (high
   above 2,481.7 ft). On each LR read (channel 13 bit 4 with bit 3) it
   writes RNRAD: the altitude beam's slant range along HBEAMNB (with the
   Doppler term POSUPDAT removes), or one velocity beam's component in
   12288 + counts. The beams follow the antenna's position (LRALPHA,
   LRBETA and the second pair, `FLIGHT_LR_POS2`). `FLIGHT_LR_TRACE=1`
   prints each read. `lm-p63-lr-dropout` is lm-p63-nassp with it from
   600 s and `FLIGHT_LR_DROP` from 880 s to 950 s: R12's range reads fail
   (RADMODES LR range fail) until the slant range comes inside
   50,000 ft at 816 s (about 38,000 ft up), pass, fail in the window and
   pass again from 952 s. `lm-p63-lr-update` is lm-p63-nassp with
   `FLIGHT_LR=60000:150` (the surface 150 m above the landing site) and
   V57E at 660 s: from 816 s R12's altitude updates take HCALC from about
   186 m above the true height to about 77 m by 955 s (with no bias it
   runs 40-60 m above, a sample's lag). It ends at 960 s, before the
   velocity reads: on the host SERVICER reaches READV 6 cs after
   READACCS, so the velocity read starts while the altitude read is
   still running, HMEAS 0 is stored and the update moves HCALC about
   1,200 m. That is a timing question for the interpretive charge, not
   the radar (replay-mismatches.md).
   `FLIGHT_UPLINK=CS:KEYS[;CS:KEYS...]` is the ground's uplink: each
   key, 20 cs apart from CS, reaches INLINK as UPRUPT's triple-redundant
   word with an UPRUPT request; `#OCTAL` sends a raw word, and `R` is
   ERROR RESET. `lm-uplink-reject` in P00: V71 (P27 flashes V21N01), 25
   components (octal, over the limit of 20 OHWELL1 checks, so V21N01
   again), V34 (P27 ends); a garbled word (#00001) at 50 s locks the
   uplink out (FLAGWRD7 bit 4), so V37E at 55 s is ignored; ERROR RESET
   at 65 s unlocks it and V37E00E takes P00. On the host every native
   routine of the four matches yaAGC.
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
   yaAGC's cycle counter, and with them the optics and gyro drives, which
   no C64 resident models (yaAGC's optics drive takes the sextant's CDU
   counters CDUS and CDUT, 036 and 035, for the drive counters CDUSCMD and
   CDUTCMD and zeroes them; its gyro drive empties GYROCMD at once); unset, the variant is the unchanged engine. The
   AGC clock then runs only as the flight resident runs it: a modeled C64
   wall clock advances by each span's AGC machine cycles times K, plus
   20 ms for each module swap (the VICE load cost; `FLIGHT_SWAP_MCT`
   sets it in AGC machine cycles, 1,707 by default), each optionally
   jittered from 1/4 to 4 times by SEED. A CIA1 interrupt lands every
   D × 10 ms of it, and at each native boundary at most LAG of them become
   `rk_timer_tick` periods, the rest dropped, as `rk_rupts` does under the
   CIA wall clock (`FLIGHT_SCALED=W` runs the work clock's
   `rk_timer_phase` steps instead, each after any request that can be
   taken); bridged code takes its periods between instructions. Lockstep then compares the
   clock counters too and adopts nothing from yaAGC, so every routine must
   match yaAGC run from the same state under the same periods, and the
   run as a whole is one yaAGC run under that tick schedule. No
   free-running reference runs, and no samples are compared.
   `FLIGHT_SCALED=W` replays the work clock instead (docs/RESIDENT-FLIGHT.md,
   The work clock): each native routine advances the clock by the
   weight the C64 store charges it (host.inc, `c64_mct` in
   tools/flight_store_build.py once the modules link): its module's
   FS_MCT from docs/flight-routine-mct.json, or for a table left to the
   compact interpreter RK_INTERP_MCT, yaAGC's mean per interpretive word
   (docs/flight-interp-mct.json). A host-only build has no modules and
   charges each table its own weight. A self-check idle routine moves it to
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
