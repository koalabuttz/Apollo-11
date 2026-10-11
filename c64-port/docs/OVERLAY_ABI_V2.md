# Overlay ABI 2: memory, module store and continuations

Lane H owns this contract. Every lane that emits native modules for the
integrated LM and CM applications builds them against it. Changes go through a
PR that Lane H reviews. ABI 1 ([OVERLAYS.md](OVERLAYS.md)) stays in place for
the retained single-slot proofs; nothing in this document changes them.

Status: the runtime, REU store, packer, allocation plan and service-table
generator pass host checks, strict and under ASan/UBSan
(`make -f tools/overlay_v2.mk overlay-v2-test`). The linker scripts, the
service jump table, the REU hooks and the RAM-vector runtime configuration also
pass a target proof on stock PAL ROMs in VICE 3.7.1 with an emulated REU
(`make -f tools/overlay_v2.mk overlay-v2-target`, see "Verification"). The
CM service overlay proof has been rebuilt as ABI 2 modules and replays to the
same final image as under ABI 1 (`make -f tools/overlay_v2.mk overlay-v2-cm`,
see "Reference port"); lanes copy that build as their template.

## Why ABI 1 is not enough for the whole program

ABI 1 proved the mechanism: one 16 KB slot at `$8000`, modules streamed from
disk through KERNAL, a canonical erasable image at `$7000`, and no C frame
surviving a swap. Four measurements show where it stops scaling.

| Question | Measurement | Source |
|---|---|---|
| How much AGC code is there? | About 35,000 fixed-memory words per program (Luminary099 35,026, Comanche055 34,210; the hardware holds 36,864) | `tools/overlay_allocation.py` line classifier |
| How much native code per AGC word? | 16 bytes (PHASCHNG, ~131 words to 2,169 bytes) up to about 40 bytes (Executive, ~366 words to ~15 KB of `exec_*` text) | `evidence/cm-clock-display-overlay-2.map`, `evidence/p66-resident-outlined.map` |
| What does that make per program? | 0.5 MB low, 0.8 MB mid, 1.3 MB high | `docs/overlay-allocation.json` |
| How fast is a disk swap? | Stock serial KERNAL is on the order of 0.4 KB/s, so a 16 KB module takes tens of seconds. The VICE proofs so far used virtual-device traps, which hide this. | estimate, not yet measured on true drive emulation |

So one program is roughly 30 to 80 slot-fulls of code. A D64 holds about 170 KB
and cannot hold either program. Swapping from disk at runtime would spend most
of each mission second waiting on the serial bus. ABI 1 also links arithmetic
helpers and gather/commit views into every module. In module 2 of the CM
clock/display proof, for example, gather and commit alone are 2.3 KB of the
module's 7.8 KB.

## Decisions

1. **Runtime module store is a 17xx REU of 2 MB (David decided this on 2026-10-11).** At boot, the
   program's packed module store goes into the REU. At runtime, a module swap
   is one DMA at one byte per cycle. On the target, a cold load and call of a
   14 KB module measured 23.9 ms, and a cached call 2.2 ms. A 1 MB REU fits the mid estimate. 2 MB
   covers the high estimate for either program. VICE emulates REUs up to 16 MB
   (`-reu -reusize 2048 -reuimage lm.reu`), and so do the Ultimate family and
   current REU clones. A disk-only build still works for module-level proofs
   (ABI 1, and ABI 2 with a disk store), but it is not the integrated runtime.
2. **Two slots.** The job slot `$8000..$BFFF` (16 KB) holds Executive job
   continuations. The task slot `$E000..$FFF9` (8,186 bytes) holds
   interrupt-level bodies: WAITLIST tasks, T4RUPT, T5RUPT/T6RUPT DAP code,
   KEYRUPT and DOWNRUPT. A task can run between job continuations without
   evicting the job's module, so the dominant swap pattern costs nothing.
3. **Runtime memory configuration is `$01=$35`.** BASIC and KERNAL are banked
   out, leaving RAM under both slots, and I/O stays visible. The resident owns
   the RAM vectors at `$FFFA..$FFFF`. KERNAL is banked in only for boot-time
   disk work.
4. **Resident services through a jump table.** Large shared arithmetic (and
   later the interpreter's hot core) is linked once in the resident. Modules
   reach it through 3-byte `JMP` entries at `$6F00`, using the normal C ABI.
   Only functions whose body is larger than a call to them are exported: the
   word operations in `agc_word.c` stay linked into each module, because LTO
   inlines them there and a call through the table costs more bytes than it
   saves (measured in "Reference port").
5. **Lane-owned 16-bit module identities.** High byte is the lane, low byte is
   the lane's own number. Lanes allocate without coordinating.
6. **Continuations are explicit.** A module never calls another module. It
   returns an action to the resident trampoline (`DONE`, `JUMP`, `CALL`, `RET`,
   `JUMP_HISTORICAL` or a kernel action), and the trampoline loads whatever
   comes next.

## Memory map (ABI 2 runtime, `$01=$35`)

| Address | Owner | Bytes |
|---|---|---:|
| `$0002..$008F` | LLVM-MOS imaginary registers and resident zero page; modules allocate none | 142 |
| `$0100..$01FF` | 6502 hardware stack | 256 |
| `$0200..$03FF` | Resident DMA buffers (store record, module header), IRQ queues | 512 |
| `$0400..$07E7` | Screen RAM: DSKY rendering and diagnostics | 1,000 |
| `$0801..$6EFF` | Resident code and initialized data | 26,367 |
| `$6F00..$6FFF` | Resident service jump table (85 entries max) | 256 |
| `$7000..$7FFF` | Canonical 2,048-word AGC erasable image (unchanged from ABI 1) | 4,096 |
| `$8000..$BFFF` | Job slot | 16,384 |
| `$C000..$C7FF` | Transient views; overflow space for resident BSS if Lane A needs it | 2,048 |
| `$C800..$CFFF` | LLVM-MOS software stack (measured peak so far 773 bytes) | 2,048 |
| `$D000..$DFFF` | I/O; REU controller at `$DF00` | 4,096 |
| `$E000..$FFF9` | Task slot | 8,186 |
| `$FFFA..$FFFF` | RAM NMI/RESET/IRQ vectors, resident-owned | 6 |

The canonical image rules from ABI 1 are unchanged. `$7000` is the only
persistent copy of erasable memory, and native views are gathered and committed
at the call boundary. No pointer into a slot survives the call that made it.

### Resident budget

The resident holds the Executive and WAITLIST kernel, SPSIN/SPCOS, the
fixed-fixed constants, the ABI 2 runtime and REU store, IRQ entry and device
adapters. INTER-BANK_COMMUNICATION and INTERRUPT_LEAD_INS turn into the
trampoline/resolver and the native IRQ entry rather than translated bodies.
PHASE_TABLE_MAINTENANCE moves to the task slot. At the mid estimate that is
24.0 to 24.6 KB against 25.7 KB of capacity. The Executive is already about
17 KB at today's translation density, so **Lane A must keep resident code to
roughly 16 bytes per AGC word or move cold paths into modules**. The resident
linker script enforces the ceiling (`overlay_v2_resident.ld`).

## Module store

### Module file and header

A module is a PRG whose two-byte load address is its slot base. The payload
starts with a 24-byte header, and the C entry `JMP` sits at slot base + 24.

| Offset | Bytes | Field |
|---:|---:|---|
| 0 | 4 | `NATV` |
| 4 | 1 | ABI version, 2 |
| 5 | 1 | Header size, 24 |
| 6 | 1 | Slot class: 0 job (`$8000`), 1 task (`$E000`) |
| 7 | 1 | Mission: 1 LM, 2 CM |
| 8 | 2 | Module identity (lane << 8 \| number) |
| 10 | 2 | Entry address, slot base + 24 |
| 12 | 2 | BSS address (after the loaded image, inside the slot) |
| 14 | 2 | BSS bytes |
| 16 | 2 | Loaded bytes, header included |
| 18 | 2 | Service-table version the module was linked against |
| 20 | 2 | Selector count; valid selectors are 0..count-1 |
| 22 | 2 | Reserved, zero |

The fields are little-endian. `overlay_v2_payload.ld` emits the header from
link-time symbols (`__ov2_slot`, `__ov2_mission`, `__ov2_id`, `__ov2_services`,
`__ov2_selectors`). It asserts the entry position, slot bounds, the id range
and zero private zero page.

### Identities

| Lane | High byte | Lane | High byte |
|---|---|---|---|
| A runtime kernel | `$01` | E targeting | `$06` |
| B orbital math | `$02` | F LM powered flight and DAP | `$07` |
| C DSKY and verbs | `$03` | G CM flight, entry, TVC | `$08` |
| D1 IMU and alignment | `$04` | H integration | `$09` |
| D2 rendezvous navigation | `$05` | proof and test data | `$0F` |

Ids `$0100..$0FFF` are valid, so each lane gets 255 per mission. The LM and CM
stores are separate, so the same id can name different modules in each.

### REU store image

`tools/overlay_v2_pack.py` turns module PRGs and a resolver list into one image.

| Region | Content |
|---|---|
| Page 0 | `NREU`, format 1, mission, service version, module count, index page, resolver page, resolver count, total pages |
| Pages 1..256 | Direct index: 4,096 records of 16 bytes, at `id * 16`: id, page, bytes, CRC16, selectors, slot, flags |
| Resolver | Records of 8 bytes, sorted by (bank, location): historical bank word, location, module, selector |
| Modules | Each image starts on a page boundary |

Lookup is one 16-byte DMA. A historical-continuation lookup is a binary search
of 8-byte DMAs. At boot, `apollo_reu_store_verify` checks every present
module's header and CRC16-CCITT and the resolver's order and targets. Per-load
CRC is off in the REU build, because a bitwise CRC costs about 250 cycles per
byte (four seconds for a full slot). A disk-backed store keeps per-load
verification on.

### Disk layout

The packer also writes a D81 plan. Disk 1 carries the resident boot PRG and the
first store segments. Segments are page-aligned slices of the REU image (named
`LM.S00`, `LM.S01`, ...), so a boot loader stashes each 256-byte page straight
into the REU. One program at the mid estimate fits one D81 (3,160 blocks, about
800 KB); the high estimate needs two, on devices 8 and 9. D64 is not a target
for whole programs. On stock serial speed, a cold boot that loads the REU from
disk is slow (on the order of half an hour for 0.8 MB). VICE acceptance runs
attach the `.reu` image directly instead. A fast loader is optional later work.

## Runtime contract

### Entry and frame

```c
uint16_t apollo_ov2_main(apollo_ov2_frame *frame, uint16_t selector);
```

The frame is resident. It carries the canonical image pointer, the context
(core-set position 0..7, or 8 for task level), an input/result `value`, and the
outputs `next`, `ret`, `hist_bank` and `hist_loc`. The return value is an action:

| Action | Trampoline does |
|---|---|
| `DONE` | Returns `value` to the resident caller |
| `JUMP` | Continues at `next` |
| `CALL` | Pushes `ret` on the context's seam stack, then continues at `next` |
| `RET` | Pops the seam stack and continues there |
| `JUMP_HISTORICAL` | Resolves (`hist_bank`, `hist_loc`) through the store's resolver, then continues |
| `>= 0x10` | Kernel action. Returned to the Executive/WAITLIST with `next` as the resume point (sleep, yield, priority change, end of task, and so on: Lane A defines the codes) |

Loads only happen inside `apollo_ov2_run`, between bodies, while no module
frame is live. A nested run is refused with `BUSY`. Any failure latches a fault
until restart and never touches the canonical image. Failure cases: unmapped id,
bad header, bad selector, seam overflow or underflow, unresolved historical
address, undefined action, or more than 64 chained transfers in one run.

### Two kinds of continuation

- **Native seams** split one AGC routine across modules where it does not fit
  one slot. They live on a small per-context stack (depth 4) in the resident.
  They are discarded on restart, on ENDOFJOB (`apollo_ov2_seam_release`), and
  whenever the program would lose them. CHANJOB exchanges them together with the
  core sets (`apollo_ov2_seam_exchange`).
- **Historical continuations** cover everything the original program stores as
  an address: Q, 2CADRs in LST2, phase tables, LOC/BANKSET, saved returns in
  erasable. The native code stores the same historical words in the canonical
  image, and the trampoline resolves them when control returns there. Restart
  recovery therefore works the way the original does: the restart tables name
  historical addresses, which the resolver maps to native continuations.
  Nothing a restart must preserve lives only in a native seam.

### Interrupts

Native IRQ handlers (CIA timers, raster, keyboard) are resident. They only
publish into resident queues, as they do today, and never call module code or
start a DMA. AGC interrupt-level programs run as task-level continuations
(context 8) from the task slot, at the boundaries the Executive already uses for
IRQ admission. Because swaps happen only at those boundaries, a DMA never
overlaps a live module frame. The REU's own interrupt is unused.

### Time

At 1 MHz, a C translation of ones-complement arithmetic runs well below AGC
speed, so the integrated programs cannot keep up with the AGC's real-time clock.
Lane A's timer adapters need a mission clock that advances with executed work
(TIME1/TIME3/TIME4/TIME5/TIME6 driven by the adapter), not by wall-clock CIA
ticks. Otherwise WAITLIST tasks overrun. This is a requirement on Lane A's
adapter, recorded here because the overlay latency figures feed it.

## Allocation plan

`docs/overlay-allocation.json` (`python3 tools/overlay_allocation.py`) assigns
every source file of both programs to a lane and a placement class, with word
and byte estimates.

| Placement | Rule | Files (LM / CM) |
|---|---|---|
| resident | Executive, WAITLIST, SPSIN/SPCOS | 3 / 3 |
| resident-data | Fixed-fixed constant pool | 1 / 1 |
| mechanism | INTER-BANK_COMMUNICATION, INTERRUPT_LEAD_INS, replaced by the ABI itself | 2 / 2 |
| task | Interrupt-level and DAP code, PHASCHNG, IMU compensation | 12 / 10 |
| library | INTERPRETER arithmetic and RTB op codes: hot core resident through services, the rest linked into consumers | 2 / 2 |
| data | Constant and table files, compiled into the modules that use them | 5 / 5 |
| declarations | Erasable, flagword, channel and tag assignments: headers and canonical layout, no code | 5 / 5 |
| job | Everything else | 60 / 57 |

Estimated modules per lane at the mid expansion (job + task slots):

| Lane | LM | CM |
|---|---|---|
| A | 5 + 5 | 5 + 6 |
| B | 7 + 0 | 7 + 0 |
| C | 9 + 0 | 8 + 0 |
| D1 | 9 + 2 | 10 + 2 |
| D2 | 9 + 0 | 6 + 0 |
| E | 5 + 0 | 8 + 0 |
| F | 13 + 11 | — |
| G | — | 11 + 13 |

These are budgets, not targets. A lane replaces its estimate with measured
module sizes as modules land. The per-routine placement inside a file belongs to
the lane, under one rule: anything entered at interrupt level is a task-slot
body.

## Rules for lanes

1. Link with `overlay_v2_slot_job.ld` or `overlay_v2_slot_task.ld`, then
   `overlay_v2_payload.ld` and `overlay_v2_entry.S`. Pick ids from your own lane
   range.
2. Persistent state lives only in the canonical image. Module `.data` and `.bss`
   reset on every load.
3. Return to the trampoline between AGC routines. Use `CALL`/`RET` only to split
   one routine, and store every address the original stores in erasable as its
   historical words.
4. Import exported arithmetic (today `agc_multiply`, `agc_double_add`) from the
   service table; do not link `agc_multiword.c` into a module. Link `agc_word.c`
   privately. New services are appended to `docs/overlay-v2-services.json`
   (minor version bump) in a PR Lane A reviews, and only for functions larger
   than a call.
5. Register each continuation the original can reach by historical address in
   the lane's resolver list for the packer.
6. Report each module's loaded size and selector count in its evidence, so the
   allocation plan can replace estimates with measurements.

## Verification

Host checks (strict C99 and ASan/UBSan): `make -f tools/overlay_v2.mk
overlay-v2-test` from `c64-port/`. `tests/test_overlay_v2.c` drives the runtime
over a store image produced by the real packer, through a register-level model
of the 17xx RAM Expansion Controller (fetch, stash, verify, 64 KB lengths,
status clearing). It covers:

- cold loads, cache hits and swaps in both slots; exact payload bytes, BSS
  zeroing, and a 16 KB module reaching `$BFFF`
- guard bytes around the slots, the image and the vectors
- `CALL` from the job slot into the task slot and `RET` back, with the job
  module still cached
- a kernel action with a seam pending, CHANJOB moving the seam, and the resumed
  body returning through it
- historical resolution, both hit and miss
- seam overflow and underflow, undefined actions, runaway chains, bad selectors,
  unmapped ids and contexts, and nested runs
- header rejection before any slot byte changes, and payload corruption with
  per-load CRC on and off
- a DMA fault at each step of a cold load, and a directory record naming
  another module
- restart dropping seams and caches but keeping the canonical image
- store open and boot verification against each corruption class
- service-table version compatibility

Target proof (`tools/overlay_v2_target_build.py`, then
`tools/overlay_v2_vice_test.py`; llvm-mos SDK v23.3.0, VICE 3.7.1, stock PAL
ROMs, 128 KB REU). The resident and three proof modules are linked with the ABI
2 scripts, and the packer accepts the headers the linker emitted. The resident
then runs in `$01=$35` with its own RAM IRQ/NMI vectors and CIA1 interrupts
live, and does the following:

- opens and boot-verifies the store from the REU
- calls a resident service (`agc_multiply`) through the `$6F00` jump table from
  a job module
- `CALL`s from the job slot into a task module at `$E000` and `RET`s back
- resolves a historical address to a 14 KB job module, which returns a kernel
  action that is then resumed
- faults on an unmapped id and recovers on restart

All 2,048 canonical words other than the four test words are unchanged. Measured
on the target:

| Measurement | Cycles | At PAL clock |
|---|---:|---:|
| Cold load and call, 14,225-byte job module | 23,644 | 24.0 ms |
| Cached call of the same module | 1,976 | 2.0 ms |
| Cold load and call, 181-byte task module | 9,341 | 9.5 ms |
| Boot verification of a 3-module store | 7,994,148 | 8.1 s |
| Software-stack extent | 93 bytes | |

The fixed cost of a cold load (about 7,000 cycles of directory and header work
in C) dominates small modules. Boot verification with the bitwise CRC costs
about 250 cycles per byte plus a scan of the 4,096-entry index. A 0.8 MB store
would take minutes to verify, so a table-driven CRC is the first optimisation
to make before whole-program stores exist.

### Reference port

`tools/cm_service_overlay_v2_build.py` rebuilds the ABI 1 CM service overlay
proof as ABI 2 modules without touching its sources. Each ABI 1 module body is
wrapped by `platform/c64/overlay_v2_adapter.c`, which turns
`apollo_overlay_main` into `apollo_ov2_main` returning `DONE`. The packer puts
all six modules in a 128 KB REU image. `platform/c64/cm_service_overlay_v2_diagnostic.c`
runs the same replay as the ABI 1 diagnostic, with modules from the store,
`$01=$35`, RAM vectors and live CIA1 interrupts. It runs the replay once with
interrupts enabled and once masked. Interrupt-level bodies (lane A modules
below 4) run in the task context. `tools/cm_service_overlay_v2_vice_test.py`
checks the result in VICE:

- mailbox `a5 0000 3a 06`: no failures, 58 checkpoints, 6 replays
- 59 module loads, 16,407 IRQs taken, software stack 821 bytes
- the final 2,048-word canonical image is byte-identical to the ABI 1 run;
  with `build/cm-service-loop-reference.snapshots` present it is also compared
  with original case 320

| Module | ABI 1 | Slot | ABI 2 bytes | ABI 1 bytes |
|---|---:|---|---:|---:|
| `$0101` | 1 | task | 3,240 | 3,015 |
| `$0102` | 2 | task | 8,113 | 7,768 |
| `$0103` | 3 | task | 5,603 | 5,353 |
| `$0104` | 4 | job | 4,460 | 4,215 |
| `$0F01` | 250 | job | 3,391 | 3,338 |
| `$0F02` | 251 | job | 7,426 | 7,351 |

ABI 2 modules are 1 to 8 percent larger. The 24-byte header and the adapter
explain part of that; the rest is not yet accounted for. `$0102` is 73 bytes
under the 8,186-byte task slot. With the eleven `agc_word.c` helpers exported
as services it was 35 bytes over, because LTO could no longer inline them; that
is why rule 4 keeps them private. Lane A should plan task bodies against about
8 KB and split larger interrupt work with `CALL`/`RET`.

Not yet done, in order:

1. Measure true-drive-emulation serial load speed, to replace the disk estimate.
2. Table-driven CRC for boot verification, and a disk-to-REU boot loader for
   real hardware.
