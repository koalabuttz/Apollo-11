# The thin kernel in the ABI 2 flight resident

Lane A (Resident kernel lane), following the steward's plan after
`resident-kernel` (docs/RESIDENT-KERNEL.md): link the thin Executive and
Waitlist into the real overlay ABI 2 resident instead of the certified
routines, move boot-only code out of the resident, and measure what is
left against $0801-$6EFF.

## What is linked

`platform/c64/resident_flight.ld` keeps the map of
`overlay_v2_resident.ld`: resident code and data in $0801-$6EFF, the service
jump table at $6F00, the canonical image at $7000, the job slot at $8000,
the task slot at $E000, the soft stack below $D000. One mission's resident is:

| Component | Sources |
|---|---|
| Kernel | `src/lm_kernel.c` or `src/cm_kernel.c` (`src/resident_kernel_body.inc`), `src/agc_image_ops.c` |
| Transfer ABI trampoline and Executive call sites | `src/agc_transfer.c`; the site operand table from `src/lm_exec_routines.c` or `src/cm_exec_routines.c` (only the table links) |
| ABI 2 runtime and REU store | `src/overlay_v2.c`, `src/overlay_reu.c`, `src/overlay_runtime.c` (the CRC), `platform/c64/overlay_reu_platform.c` |
| Single-precision subroutines | `src/lm_single_trig.c`, or `src/cm_quictrig.c` and `src/cm_single_return.c`; their transfer-ABI table `src/sp_routines.c` |
| Service table exports | `src/agc_multiword.c` (`agc_multiply`, `agc_double_add`, docs/overlay-v2-services.json) |
| AGC timers and interrupt entry | `src/resident_rupt.c` |
| IRQ entry | `platform/c64/overlay_v2_irq.S` |
| Resident state and step | `platform/c64/resident_flight.c` |

`platform/c64/resident_flight.h` is the interface. `rk_boot()` switches
KERNAL and BASIC out ($01=$35), sets the resident RAM interrupt vectors,
starts CIA1 timer A at 10 ms (PAL latch 9851), fills
the kernel's routine tables, opens the REU store and verifies it, and
initializes the runtime on the image at $7000.

`rk_resident_step()` joins the kernel, the AGC clock and the modules. It
first runs the clock and takes a pending interrupt (below), then one
kernel routine from the image's Z, and returns, so the next step can take
an interrupt at the boundary that routine left. At a point with no kernel
routine the step resolves that point through the store's resolver,
keyed by FB with the channel 7 superbank bits (as BBCON holds them) and Z.
It runs the module in context 0 (the running job is always core set 0) or,
inside an interrupt, in context 8. A module leaves Z where the program
continues. `platform/c64/resident_flight_main.c` is the flight entry: boot,
then steps.

### Seams on job switches

The flight resident builds the kernel with `RK_SEAMS`. CHANJOB4, where the
kernel exchanges core set 0 with the set at NEWJOB, first calls
`apollo_ov2_seam_exchange` for job context NEWJOB/12, and ENDOFJOB first
calls `apollo_ov2_seam_release` for context 0. So a module chain that a job
switch suspends inside a CALL follows its job to the core set it moves to,
and a finished job's seam is freed. `make test-resident-kernel` builds the
kernel with counting hooks and checks every exchange names a core set
(NEWJOB a multiple of 12 from 12 to 84): 5,131 LM and 4,851 CM exchanges,
1,042 releases each, none outside a core set.

### SPSIN and SPCOS

`src/sp_routines.c` gives the single-precision sine and cosine a
transfer-ABI routine table (`lm_sp_routines`, `cm_sp_routines`; LM SPCOS
05032, SPSIN 05033; CM 05074, 05075), so the step enters them like a kernel
routine and stops at the caller's return. Each refuses a return address
outside fixed memory (Q not in 02000-07777) before it writes anything.
`make test-sp-routines` runs the frozen yaAGC full-return states (LM
lm-single-trig selectors 2-3, CM cm-single-return selectors 0-1) through the table, and the
refusal with the image unchanged: 128 checks per mission, plain and
ASan/UBSan.

### The AGC clock and interrupts

`src/resident_rupt.c` (`include/resident_rupt.h`) does what yaAGC's
`agc_engine.c` does for the timers and the interrupt entry:

- `rk_timer_tick` is one 10 ms scaler period. TIME1 counts, and its
  overflow counts TIME2. TIME3, TIME4 and TIME5 count, and an overflow
  requests T3RUPT, T4RUPT or T5RUPT. While channel 13 bit 15 is set, TIME6
  takes its sixteen 1/1600 s DINCs. +1 and -1 step to -0 through the one's
  complement adder, and a DINC of ±0 requests T6RUPT and clears the bit.
  T6RUPT therefore lands at the end of the 10 ms period, not at its 0.625
  ms step.
- `rk_rupt_take` vectors the highest pending request when no interrupt is
  in progress, interrupts are enabled, A holds no overflow and the word at
  Z is not RELINT, INHINT or EXTEND. It sets ZRUPT = Z + 1, BRUPT = the word
  at Z and Z = 04000 + 4 × request. It reads that word from erasable (under
  EB from 01400), or from fixed memory through `program->fixed_data`.

The step calls both through `rk_rupts`. That consumes one tick per CIA1
interrupt counted by `overlay_v2_irq.S` since the last step, then tries the
entry. T3RUPT then runs entirely in the kernel: the 04014 lead-in, T3RUPT,
each task due and TASKOVER, to RESUME.

`make test-resident-rupt` checks this against yaAGC itself
(`tools/resident_rupt_reference.c` links Virtual AGC's `agc_engine`, and
`tools/resident_rupt_vectors.py` freezes its results in
`tests/resident_rupt_vectors.h`). Each mission has three kinds of case:

| Cases | What is compared |
|---|---|
| 8,192 entry cases (random image, Z in fixed memory under random FB/EB/superbank or in erasable, overflow, inhibit, in-interrupt, random pending set) | Whether `agc_engine` takes a request before the instruction at Z; when it does, the vector, Z, ZRUPT, BRUPT, the requests left and InIsr |
| 2,048 T3RUPT cases (Z in erasable, the Waitlist in WAITLIST's own form, every task TASKOVER) | The whole interrupt, from the entry through the kernel to RESUME, against `agc_engine` run until RESUME substitutes BRUPT: all 2048 words, channel 7, the interrupt enable and InIsr |
| 40,960 timer periods (every value in all six timers, plus random edges) | TIME1-TIME6, the requests and channel 13 after one period, with yaAGC's own CounterPINC and CounterDINC in `agc_engine`'s order |

All cases pass for both missions, plain and under ASan/UBSan. Every T3RUPT
case reaches RESUME: 4,984 LM and 5,006 CM kernel routines. Two
mutations fail the test: BRUPT off by one (160 checks) and TIME3's
overflow raising T4RUPT (115 checks). The fixed words come from the
unmodified ropes in `evidence/fixed-data-<m>-original.rope.bin`.

### Boot-only code

The linker script places `rk_boot` (section `.boot.text`) in the job slot,
together with whatever LTO leaves out of line of `apollo_reu_store_open`,
`apollo_reu_store_verify`, `apollo_reu_store_backend` and `apollo_ov2_init`.
The PRG loads it at $8000. It runs once from `main`, and the first module
load overwrites it. The disk loader (`apollo_overlay_call`, ABI 1) is not
linked at all, because the REU store needs no file I/O after boot.

## Size

`make resident-flight-size` links with LLVM-MOS at -Oz, LTO and
gc-sections, and writes `build/resident-flight/sizes.json`. The table counts
each linked symbol under the component whose source defines it.
`rk_trampoline` and `rk_run_module` keep `agc_transfer_run` and
`apollo_ov2_run` out of line, so each is counted with its own component
and not inside the step. Each has one caller, and inlining both would save
about 0.3 KB.

| Component | LM bytes | CM bytes |
|---|---|---|
| Kernel (with the seam hooks) | 10,475 | 10,232 |
| Transfer ABI trampoline and Executive call sites | 2,680 | 2,612 |
| ABI 2 runtime and REU store | 5,176 | 5,176 |
| Single-precision subroutines and their table | 1,299 | 1,299 |
| Service table exports | 637 | 637 |
| AGC timers and interrupt entry (`rk_rupts`) | 1,092 | 1,092 |
| Resident state and step | 545 | 563 |
| IRQ entry | 16 | 16 |
| C runtime and library (crt0, multiply, divide, memcpy, memset) | 480 | 480 |
| **Resident, $0801 to the end of bss** | **22,400** | **22,107** |
| Free below the service table (of 26,367) | 3,967 | 4,260 |
| Service table at $6F00 (2 entries) | 6 | 6 |
| Boot-only, in the job slot | 2,837 | 2,821 |

The kernel, including its operand reader (`inline_word`, about 0.7 KB), is
half of the resident. The seam hooks add 225 LM and 222 CM bytes to the
kernel, and the NEWJOB/12 in `rk_seams_exchange` pulls in the 125-byte
`__udivmodqi4`. The single-precision component measures 172 bytes less
with its table linked than without it. Both missions fit with 5.1-5.4 KB to spare, so neither
$C000-$CBFF nor $D000 is needed and the kernel stays as it is.

## Checks on the linked resident

`make vice-resident-flight` runs eight programs (`<m>-check-<part>.prg`, LM
and CM, parts 1 services, 2 jobs, 3 idle, 4 clock and T3RUPT) under VICE with the mission's REU
store attached. Each one links the same resident sources and script as the
flight resident. Its `main` is `platform/c64/resident_flight_check.c` in place
of the flight entry, with the kernel halves of the cases
(`tests/resident_kernel_cases.c` with `RK_RUNS_ONLY`).

Each program does the following:

1. Boots exactly as the flight resident does, so KERNAL is out, the resident
   IRQ vector is live and the store is verified.
2. Runs that part's C64 rows from `tests/<m>_kernel_c64_rows.h`. The case
   generators do not fit beside the resident, so they are ABI 2 modules
   (`platform/c64/resident_flight_cases.c`, lane A ids 0x01F1-0x01F5, one
   per resident-kernel C64 part). For each row the program loads the row's
   case module from the REU into the job slot. The module writes the case
   into the image at $7000. The resident then runs the kernel and compares
   the digest with the frozen yaAGC digest.
   The rows and their digests come from a task-slot data module (0x01F6,
   `platform/c64/resident_flight_data.c`), which for LM also holds the
   RESTARTS routines the lm-exec-services programs chain into. That module
   stays loaded while the kernel's trampoline calls into it.
3. Runs one `rk_resident_step` from a point with no kernel routine. The
   step must resolve that point through the store's resolver to the services
   module's probe selector, and Z must come back where the probe left it.
4. Runs one step from SPSIN with A = 0 and Q in fixed memory. It must go
   through the resident's SPSIN/SPCOS table and stop at Q with A = 0.
5. Checks that interrupts kept arriving through the resident vector.

Part 4 runs no kernel rows. Instead, with the CIA interrupt held off so
that no tick lands, it runs every 32nd T3RUPT case
(`tests/resident_rupt_c64_rows.h`, 64 per mission). A job-slot module
(0x01F7, `platform/c64/resident_flight_rupt_cases.c`) writes each case into
the image. The program sets T3RUPT pending and runs resident steps until
RESUME brings Z back. The image, channel 7, the interrupt enable and the
interrupt flag must give yaAGC's digest. Then, with the timer running, TIME1
must have counted exactly the ticks the steps consumed (at least five), and
CIA1 timer A must be counting down from the 10 ms latch.

| Program | Rows | Checks |
|---|---|---|
| LM services | 153 | 311 |
| LM jobs | 161 | 327 |
| LM idle | 86 | 177 |
| CM services | 129 | 263 |
| CM jobs | 110 | 225 |
| CM idle | 90 | 185 |
| LM clock and T3RUPT | 64 | 71 |
| CM clock and T3RUPT | 64 | 71 |

All 729 rows pass, the same rows as `make vice-resident-kernel`. A one-word
kernel mutation (CS TWO to CS THREE in DUMMYJOB) fails the LM idle program.
The host differential (`make test-resident-kernel`) is unchanged; its cases
now run as an input half and a kernel half
(`rk_*_input`, `rk_*_run`), which the C64 check splits across the module
and the resident.

## Evidence

In `evidence/resident-kernel/` (hashes in `SHA256.json`): the eight VICE
results with their PRG hashes (`resident-flight-vice.log`), the size report
(`resident-flight-sizes.json`, which `tools/overlay_allocation.py` copies
into docs/overlay-allocation.json), the host differential with the seam
counts (`resident-kernel-host.log`), the SPSIN/SPCOS results
(`sp-routines-host.log`), the clock and interrupt results against yaAGC
(`resident-rupt-host.log`), and the `flight_resident` section of
`resident-kernel-proof.json` with the source and rope hashes.

## Not yet in the resident

- The fixed-memory reader for BRUPT. `rk_program.fixed_data` is not set,
  so the resident takes an interrupt only where Z is in erasable. On the
  C64 the only whole-rope copy is the REU image (`include/rope_image.h`),
  which ci_guard reserves for the self-check. The steward and the Kernel
  lane are deciding where this word comes from. The Kernel lane's RESUME
  check (`agc_resume_exact`) needs the same word.
- The vectors other than T3RUPT: the T4RUPT, T5RUPT, KEYRUPT, UPRUPT and
  the other lead-ins, the T4RUPT core, KEYRUPT/UPRUPT and LM T6RUPT. These
  are the Kernel lane's tables (`rupt_lead_ins.h`, `t4rupt_core.h`,
  `keyrupt_uprupt.h`, `lm_t6rupt_routines.h`). Linked into the resident,
  the lead-ins alone leave LM 2.8 KB and CM 1.2 KB free, KEYRUPT/UPRUPT
  leaves under 0.6 KB, and the T4RUPT core does not fit. Per
  docs/OVERLAY_ABI_V2.md they belong in task-slot modules, which the
  step's resolver reaches like any other module. Their packaging waits on
  the steward.
- KEYRUPT and UPRUPT requests. The scheduler takes them (`RK_RUPT_KEY1`,
  `RK_RUPT_UP`) once the DSKY input sets them in `rk_pending`.
- The site operand table holds the Executive and Waitlist call sites only.
  The flight program's table grows with every module that calls them.
