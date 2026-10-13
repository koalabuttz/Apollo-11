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
| Single-precision subroutines | `src/lm_single_trig.c`, or `src/cm_quictrig.c` and `src/cm_single_return.c` |
| Service table exports | `src/agc_multiword.c` (`agc_multiply`, `agc_double_add`, docs/overlay-v2-services.json) |
| IRQ entry | `platform/c64/overlay_v2_irq.S` |
| Resident state and step | `platform/c64/resident_flight.c` |

`platform/c64/resident_flight.h` is the interface. `rk_boot()` switches
KERNAL and BASIC out ($01=$35), sets the resident RAM interrupt vectors, fills
the kernel's routine tables, opens the REU store and verifies it, and
initializes the runtime on the image at $7000.

`rk_resident_step()` joins the kernel to the modules. The trampoline runs
kernel routines from the image's Z until it reaches a point with no kernel
routine. The step then resolves that point through the store's resolver,
keyed by FB with the channel 7 superbank bits (as BBCON holds them) and Z.
It runs the module in context 0 (the running job is always core set 0) or,
inside an interrupt, in context 8. A module leaves Z where the program
continues. `platform/c64/resident_flight_main.c` is the flight entry: boot,
then steps.

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
| Kernel | 10,250 | 10,010 |
| Transfer ABI trampoline and Executive call sites | 2,609 | 2,541 |
| ABI 2 runtime and REU store | 5,176 | 5,176 |
| Single-precision subroutines | 1,471 | 1,471 |
| Service table exports | 637 | 637 |
| Resident state and step | 540 | 558 |
| IRQ entry | 16 | 16 |
| C runtime and library (crt0, multiply, memcpy, memset) | 336 | 336 |
| **Resident, $0801 to the end of bss** | **21,035** | **20,745** |
| Free below the service table (of 26,367) | 5,332 | 5,622 |
| Service table at $6F00 (2 entries) | 6 | 6 |
| Boot-only, in the job slot | 2,797 | 2,780 |

The kernel, including its operand reader (`inline_word`, about 0.7 KB), is
half of the resident. Both missions fit with 5.3-5.6 KB to spare, so neither
$C000-$CBFF nor $D000 is needed and the kernel stays as it is.

## Checks on the linked resident

`make vice-resident-flight` runs six programs (`<m>-check-<part>.prg`, LM
and CM, parts 1 services, 2 jobs, 3 idle) under VICE with the mission's REU
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
3. For the LM services rows, reads the RESTARTS routines the lm-exec-services
   programs chain into from a task-slot module (0x01F6,
   `platform/c64/resident_flight_restart.c`). That module stays loaded while
   the kernel's trampoline calls into it.
4. Ends with one `rk_resident_step` from a point with no kernel routine. The
   step must resolve that point through the store's resolver to the services
   module's probe selector, and Z must come back where the probe left it.
5. Checks that interrupts kept arriving through the resident vector.

| Program | Rows | Checks |
|---|---|---|
| LM services | 153 | 310 |
| LM jobs | 161 | 325 |
| LM idle | 86 | 175 |
| CM services | 129 | 261 |
| CM jobs | 110 | 223 |
| CM idle | 90 | 183 |

All 729 rows pass, the same rows as `make vice-resident-kernel`. A one-word
kernel mutation (CS TWO to CS THREE in DUMMYJOB) fails the LM idle program.
The host differential (`make test-resident-kernel`) is unchanged; its cases
now run as an input half and a kernel half
(`rk_*_input`, `rk_*_run`), which the C64 check splits across the module
and the resident.

## Not yet in the resident

- The production interrupt entry. `overlay_v2_irq.S` acknowledges CIA1 and
  counts; the T3RUPT/T4RUPT/T5RUPT/T6RUPT and KEYRUPT dispatch onto the
  kernel and the modules belongs to integration.
- Seam bookkeeping on job switches. `apollo_ov2_seam_exchange` and
  `apollo_ov2_seam_release` are not called when the kernel's CHANJOB or
  ENDOFJOB exchanges or frees core sets. A module chain that sleeps a job
  inside a CALL needs them.
- The single-precision subroutines link but have no transfer-ABI routine
  table yet, so the step cannot enter them.
- The site operand table holds the Executive and Waitlist call sites only.
  The flight program's table grows with every module that calls them.
