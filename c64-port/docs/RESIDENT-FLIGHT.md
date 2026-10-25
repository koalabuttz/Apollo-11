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
the task slot at $E000, the soft stack below $D000 (1 KB, $CC00-$CFFF).
Below the stack, $C000-$CBFF is slot class 3 (`APOLLO_OV2_SLOT_PINNED`):
boot loads the store's one pinned module there by plain DMA, verifies it,
zeroes its BSS and runs its main once to build its table list
(`rk_pinned`); from then on `rk_direct` tries its tables before the job and
task slots', and no run loads it. Boot writes a canary at $CC00, the
stack's last byte; a resident step that finds it changed (the stack ran
into the pinned module) returns `EXEC_INVALID_ACTION` before running
anything. A store can also keep the task slot's RAM for one module, slot
class 4 (`APOLLO_OV2_SLOT_PINNED_TASK`): boot loads it at $E000 by plain
DMA and zeroes its BSS before the class-3 main runs, and that main's table
list names its tables, so `rk_direct` reaches both regions through
`rk_pinned`. Such a store has no task-slot modules (the packer refuses
them), so task runs load into the job slot. The RAM under I/O, $D000-$DFFF, is the interpreter extension:
up to 4 KB of cold math bodies, slot class 2 (`APOLLO_OV2_SLOT_FIXED`). Boot
loads every class-2 module there once (`apollo_reu_store_fixed`,
`rk_hidden_dma`): the REU registers are set with I/O in, the command is armed
without execute-now, and a write to $FF00 starts the DMA with $01=$34. Its
code runs with I/O banked out, so the IRQ entry saves $01, sets $35 to
acknowledge CIA1 and restores it.

The top of the service-table page holds the device records flight modules
reach by absolute address (`--defsym` at link, `RK_HARDWARE`,
`RK_FLIGHT_IO` in `resident_flight.h`): `rk_hardware` (`agc_hardware`) at
$6FBE, which also holds the channels compiled routines use (5, 6, 30-33 on
the CM, the LM DAP's channels, built with `-DAGC_CHANNELS_HW=rk_hardware`;
on the CM inputs 30-33 start at the values yaAGC starts with), and
`rk_flight` (the `flight_io` fields neither `rk_io` nor `agc_hardware`
holds) at $6FDD. Below them, at $6FBC (`RK_IRQ_COUNT`), is
`ov2_irq_count`, the CIA1 interrupt count, so that a long-running module
(the interpreter) can return to the step when an interrupt arrives, and
below that, at $6FB8 (`RK_DILATION`), `rk_dilation`, the 32-bit count of
clock ticks scaled time dropped (below). The service table must end at or
below $6FB8 (61 entries); the linker script asserts it. One mission's resident is:

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
starts CIA1 timer A at 10 ms × `RK_TIME_DIVISOR` (PAL latch 9851 at the
default 1), fills
the kernel's routine tables, opens the REU store and verifies it,
initializes the runtime on the image at $7000, and reads where the
boundary-word table lies from the store's last four bytes (below).

`rk_resident_step()` joins the kernel, the AGC clock and the modules. It
first runs the clock and takes a pending interrupt (below), then one
kernel routine from the image's Z, and returns, so the next step can take
an interrupt at the boundary that routine left. At a point with no kernel
routine the step resolves that point through the store's resolver. The key
is canonical (docs/OVERLAY_ABI_V2.md, `rk_resolver_bank`): for fixed-fixed
04000-07777 the bank is 04000 or 06000 whatever FB and the superbank hold,
and elsewhere it is FB's bank bits, plus the superbank bit 0100 only where
FB is 030-037 and that bit is set (banks 040-043). Superbank bits that
change no address never change the key.
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

- `rk_timer_phase` is one of the three steps of a 10 ms scaler period,
  at the SCALER1 counts where yaAGC steps each counter, and
  `rk_timer_tick` is all three:
  - 2.5 ms (SCALER1 010): TIME4 counts, and its overflow requests T4RUPT.
  - 5 ms (020): TIME1 counts, and its overflow counts TIME2. Then TIME3
    counts, and its overflow requests T3RUPT.
  - 10 ms (040, the next period's 000): TIME5 counts, and its overflow
    requests T5RUPT.

  So when TIME4 and TIME3 overflow in one period, T4RUPT is requested
  first, as on the AGC, though T3RUPT outranks it (#511). The scalers
  count AGC time with them: SCALER1 (channel 4) by 8, 8 and 16, as it
  counts 3,200 times a second, and SCALER2 (channel 3) on its overflow.
  Before scaled time the C64 never advanced them at all. While channel 13
  bit 15 is set, TIME6 takes its 1/1600 s DINCs, 4, 4 and 8. +1 and -1
  step to -0 through the one's complement adder, and a DINC of ±0
  requests T6RUPT and clears the bit. T6RUPT therefore lands at the end of
  the step, not at its 0.625 ms DINC.

  On the CM the timers read and clear channel 13 in `rk_device`
  (`cm_native_io`, build flag `RK_DEVICE_CH13`), the record the CM tables
  write it in (T6START's WOR, the radar routines).
  `rk_hardware.channel13` is only the copy a module on another CM io kind
  leaves behind. While the CM timers read that copy, T6 did not start on
  the C64 CM (0 T6RUPTs in 300 cs of CM P40 from 70660 cs; 30 since).
- `rk_rupt_take` vectors the highest pending request when no interrupt is
  in progress, interrupts are enabled, A holds no overflow and the word at
  Z is not RELINT, INHINT or EXTEND. It sets ZRUPT = Z + 1, BRUPT = the word
  at Z and Z = 04000 + 4 × request. It reads that word from erasable (under
  EB from 01400), or from fixed memory through `program->fixed_data`. A
  fixed word it cannot read takes nothing, and the request stays pending.

The step calls both through `rk_rupts`. Under the work clock (the flight
default, below) that takes each step due since the last, a step being due
when the work the resident ran reaches it, and tries the entry before
each: once it takes a request, the later steps wait for the next
boundary, so their requests come after it. Under the CIA wall clock
(`RK_WALL_CLOCK=1`) it consumes one whole period per CIA1 interrupt
counted by `overlay_v2_irq.S`, at most `RK_TICK_LAG` of them (scaled time,
below), then tries the entry. T3RUPT then runs entirely in the kernel: the 04014 lead-in, T3RUPT,
each task due and TASKOVER, to RESUME.

### Scaled time

The C64 runs some flight work slower than the AGC did (the LM DAP needs
about 1.7 times the CPU a PAL C64 has in 100 ms, as measured). The project chose scaled time: AGC time runs
behind wall time when the C64 falls behind, by one factor shared by every
AGC clock and timer, so the AGC program sees the same sequence of events
it would at full speed. Two build settings control it, both make
variables passed to `tools/resident_flight_build.py` (its `COMMON` flags,
which every flight build shares):

- `RK_TICK_LAG` (default 2): the most clock ticks one step catches up.
  When more CIA1 interrupts arrived since the last step (it ran longer
  than that many 10 ms periods), `rk_rupts` runs `RK_TICK_LAG` timer
  periods and drops the rest, counting them in `rk_dilation` ($6FB8). The
  timers therefore never bunch: no step sees more than two periods' worth
  of TIME3-TIME6 requests, and AGC time stretches by exactly the dropped
  ticks. Wall time is AGC time plus `rk_dilation` periods.
- `RK_TIME_DIVISOR` (default 1): AGC time at 1/n of wall time, by a CIA1
  latch of 9852 × n − 1. `make vice-resident-flight-half` builds the
  resident at x0.5 in build/resident-flight-half and runs the clock checks
  (part 4). It has no `c64-` twin, so only full and nightly runs take it.

Ticks are counted as wall-clock interrupts and dropped whole, so a
dropped tick never splits a timer period: TIME1-TIME6 step together, in
`agc_engine`'s order, once per period the AGC sees.

The invariant this rests on: **every AGC-visible input is an AGC-time
event.** IMU and CDU counts, radar, the hand controller, keys, uplink and
discretes must reach the AGC on its clock (at a step, through the image or
`rk_hardware`), never from a C64 wall-clock source in between. A lane that
models an input against wall time breaks the equivalence and must instead
schedule it in AGC periods.

The proof is a replay against yaAGC (docs/FLIGHT-STORE.md, host
differential, scaled time): `tools/flight_host_differential.py` runs
`tools/flight_host_run.c` with yaAGC's own clock frozen and the AGC clock
driven by a modeled C64 tick schedule through the same cap, and every
native routine must match yaAGC run from the same state under the same
periods, the clock counters included. Under every schedule both sides see
the same AGC-time events, so the run is one yaAGC run on an AGC whose
clock ran at the schedule's speed. On 30 s of AGC time:

| Scenario | Schedule (K, D, lag, seed) | Native routines | yaAGC did not match | Periods taken / dropped |
|---|---|---|---|---|
| LM V37E12E | x1 (1, 1, 2) | 77,140 | 0 | 3,000 / 8 |
| LM V37E12E | x0.5 (1.7, 2, 2) | 120,442 | 0 | 3,005 / 0 |
| LM V37E12E | overrun (4, 1, 2, 1) | 16,839 | 0 | 3,000 / 1,228 |
| LM V37E12E, DAP running | x1 | 52,885 | 0 | 3,004 / 47 |
| LM V37E12E, DAP running | x0.5 | 98,239 | 0 | 3,000 / 0 |
| LM V37E12E, DAP running | overrun | 12,534 | 0 | 3,101 / 1,304 |
| CM V37E06E | x1 | 160,987 | 0 | 3,000 / 5 |
| CM V37E06E | x0.5 | 209,481 | 0 | 3,000 / 0 |
| CM V37E06E | overrun | 30,257 | 0 | 3,002 / 398 |

K is the C64's slowdown over the AGC, D the time divisor; x0.5 is the
half-speed build on a C64 1.7 times slower than the AGC (the measured LM
DAP load), and overrun a C64 four times slower with every span jittered
from 1/4 to 4 times, so module swaps and long spans drop ticks. With the
DAP running (`FLIGHT_CH30=036377 FLIGHT_CH31=057777`, CDUY moved at
15 s), the unscaled run misses six DAP routines 92 times each because
yaAGC's IMU CDU drive drains the CDU command counters 050-052 on its own
cycle clock and nothing drives them natively. That drive is an
AGC-visible output the invariant above still has to bring onto AGC time.
The tick schedule logged from a whole C64 flight run replaces the modeled
one once the flight image runs end to end.

The resident check (part 4) covers the cap: after five or more CIA
interrupts TIME1 counts `RK_TICK_LAG` and `rk_dilation` the rest, and with
a backlog past the cap and both T5RUPT and T6RUPT coming due in the first
period, one step takes `RK_TICK_LAG` periods, drops the rest and enters
T6RUPT alone, leaving T5RUPT pending: AGC priority order, no overlap.

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

### The work clock

The CIA wall clock above starves the AGC on a C64: module loads and
dispatch take wall time the AGC never spends, so the cap drops most ticks
(measured: one T5RUPT a second instead of ten). The flight default is
instead a work clock: AGC time advances by the work the resident ran.

- Every routine is charged a calibrated MCT weight. `rk_work` counts the
  MCT since the last clock step, and one tick is 853⅓ MCT in three steps
  at 2.5, 5 and 10 ms (`RK_PHASE_MCT(phase)`, nine steps over three
  periods: 214, 213, 427, 213, 213, 427, 213, 214, 426, so the periods
  still end every 854, 853 and 853 MCT), each taken at the next routine
  boundary. Nothing is dropped, and module loads, IRQs and dispatch are
  not counted, so the same state and inputs always give the same tick
  schedule.
- Weights are per module, one byte (`FS_MCT`, 4-255): the module's total
  MCT over its total runs, from Lane B's shared calibration
  `docs/flight-routine-mct.json` (the x86_64 build uses the same file).
  `tools/resident_flight_build.py` and `tools/flight_store_build.py`
  compute them. The resident's weight leaves out the Executive idle table
  (LM 35, CM 73). The compact interpreter charges each interpretive word
  it runs from a table fitted per order and store kind to yaAGC's MCT
  over the CM entry and LM P63 scenarios (`include/interp_word_mct.h`,
  `docs/flight-interp-mct.json`, `tools/interp_word_mct.py`; RMS error
  102 MCT a word). It is a fitted charge, not yaAGC's count for the word:
  the host and the C64 share it, so "C64 matches host" says nothing about
  it. Each scaled host run prints its sum against yaAGC's MCT over the
  same spans ("interpreter charge"). For a run of 1,000 routines or more,
  `tools/flight_host_differential.py` fails the run, and program and
  device acceptance mark its row failed, when the net error is over 20% or
  the error summed routine by routine (so over- and under-charges cannot
  cancel) is over 45% of yaAGC's MCT. On da027bee the 58 such runs are
  -0.7..+17.8% net (CM P11 highest) and 14.4..39.3% routine by routine;
  the second bound guards against regressions and is not an accuracy
  claim. Smaller runs are reported, not bounded (issue #765). The host's
  interpretive tables with no calibrated runs get the mean per word. The interrupt wrappers report their weight in
  `rk_module_env.scale`, and a module reports its routine count in
  `rk_module_env.work`.
- A module runs at most `rk_module_limit(budget, mct)` routines before it
  hands back, so a clock step is never more than one routine late.
  With a request waiting (interrupts inhibited, or inside an interrupt)
  the budget is 0 (`rk_budget`), so a run goes one routine at a time and
  the host's take at the first routine boundary it can (RELINT, RESUME)
  falls at the same place, on both missions. That one routine counts as
  the run's whole limit (`rk_cut`): if the request is not taken, the next
  step goes on in the same module and is charged its table's weight, as
  the host keeps charging the chain's table. Without that, cm-entry 17000
  charged an SP routine in an entry DAP chain the SP table's mean (73 MCT,
  not 13) and took T4RUPT 28 MCT late (issue #886).
- Every table has one weight, its mean MCT per routine, on the host and
  on the C64 alike. That holds for the interrupt lead-ins too: on the LM a
  direct run that starts at a lead-in vector (04000-04053, GOPROG to
  RUPT10) charges its first routine the lead-in table's 9 MCT. The AGC's
  own costs are 5 (GOPROG), 7 (banked vectors), 8 (RUPT10) and 10 (T5,
  T6), so each interrupt's lead-in is charged between 3 MCT too much and
  1 too little. At 50 interrupts a second that is at most 150 of the
  about 85,000 MCT in an AGC second (0.18%). It moves where a routine falls
  against the timers, not the timers' own rate, and the host is charged
  the same, so a replay still compares like with like (issue #886).
- The self-check idle tables (`*_self_check_idle_routines`) carry weight
  0 (`RK_MCT_IDLE`): one idle pass takes the rest of the tick, unless a
  step on the way raises a request that is taken, which ends the idle
  there (the rest is dropped, as the AGC leaves its idle loop for the
  interrupt). Without it
  the AGC background fills each second with about 85,000 MCT of
  self-check, and the LM DAP hold ran at 221x real time. The LM idle
  module also packs the interbank tables (`CO_LOCATE` in
  `tools/flight_store_build.py`), so an idle pass loads nothing.
  It packs the interrupt lead-ins as well, so an interrupt taken in the
  idle loop starts there instead of loading the wrapper $01E1, and the
  T4RUPT pack carries the idle loop (docs/FLIGHT-STORE.md). A module that
  holds the idle loop beside interrupt code is built with `FS_IDLE`
  (`rk_module_tables.idle`): outside an interrupt its runs are idle,
  inside one they weigh its other tables' MCT (the lead-ins count only
  where no other table does: `LEAD_INS`). On the P63 descent-thrust window
  from 17500 cs (30 cs) this took 17.27x to 16.09x real time, with module
  switches 57 to 31 and lead-in loads 15 to 0 (+17 B LM resident).

Measured in VICE (LM, 100 AGC cs from the cs 3000 snapshot,
`tools/dap_pass_vice.py`): the DAP hold runs at 16.7x real time and
firing at 16.9x, with 10 T5RUPT, 9 T4RUPT and no tick dropped. The rest is
background work (12 M cycles per AGC second) and job reloads (118 per AGC
second).

The host replay schedules ticks the same way (`FLIGHT_SCALED=W` in
`tools/flight_host_run.c`, with the idle rule and no swap charge), and
every routine of the work rows matches yaAGC:

| Scenario | Native routines | yaAGC did not match | Periods taken / dropped |
|---|---|---|---|
| LM V37E12E | 14,697 | 0 | 3,000 / 0 |
| LM V37E12E, DAP running (V37 at 16 s; reaches P12) | 18,337 | 0 | 3,000 / 0 |
| CM V37E06E | 10,380 | 0 | 3,000 / 0 |

The real-time cap keeps AGC time from running ahead of wall time on a
machine faster than real time (a turbo C64). Before each tick, the step
waits until CIA1 has counted one more 10 ms period than the ticks taken
(`rk_cap`, against `ov2_irq_count`'s low byte). A lag of more than
`RK_CAP_LAG` (16) periods is forgiven, so a machine that fell behind
doesn't race later to catch up. A stock C64 runs slower than real time,
so it never waits there: the DAP pass cycles don't change. The cap is 24
bytes of assembly in `rk_rupts`' tick loop (40 bytes of resident in all).
Part 4 of the resident check takes three ticks with no lag owed and
checks that CIA1 counted three more periods.

`make ... RK_WALL_CLOCK=1` builds the CIA wall clock instead;
`make vice-resident-flight-half` builds it at x0.5.

### Idle memo

Between interrupts the AGC spends most of its time in two steps of the
idle loop: the self-check pass from SELFRET's return (CCS SMODE, CHECKNJ,
POSTJUMP to ADVAN, at 43,3304 LM / 43,3306 CM) and ADVAN with no job ready
(01,3214 LM / 01,3231 CM), which returns through SELFRET to the pass.
On the CM the pass's run stops at POSTJUMP (Z 04701), which runs as a step
of its own, as on the host (`tools/flight_host_run.c` takes each routine as
a step): an interrupt that ends the pass's idle returns to POSTJUMP, which
idles again, not to ADVAN. Chaining the pass through POSTJUMP put the C64
one step ahead after such an interrupt (cm-entry 29000+400 under
`flight_scenario_vice.py --replay --replay-exact` left the host at its
first checkpoint; with the stop it matches there). Each
costs a module run (and after a DAP excursion, a reload of the idle
module), yet each is a pure function of a few words. With NEWJOB -0, SMODE
+0, FB at the step's bank and outside an interrupt, each reads only its key
(Z, L, EB, BB, SELFRET and the superbank) and writes only its result (those
words and A, Q, FB). `tests/test_idle_memo.c` (`make test-idle-memo`, LM and
CM builds) checks both claims on random images: no other erasable word, io
field or hardware field changes, no job-switch seam is called, and two runs
that share the key end on the same result.

So `rk_resident_step` (under the work clock, not in the check builds
`RK_RUNS_ONLY`) remembers each step: the key it started on, the result it
ended on, and the work it charged (the idle loop's rest of the period, or
the count it added). A flight step (no stop point) that meets the gate and
starts on the remembered key writes the result and charges the same work
without running the step or loading anything. The two memos swap after each
step, since the steps alternate; Z, in the key, keeps either from answering
for the other. On the P63 descent-thrust window from 17500 cs (30 cs, base
73b062d5 + #425) this took 16.09x to 13.86x real time with the same 168
steps and end state; an idle step costs about 5K cycles instead of 7-16K.
It costs about 560 B of LM and CM resident (`rk_memo_*` and the step split
into `rk_step_run`).

### The interrupt bodies

The vectors other than T3RUPT run the Kernel lane's tables unchanged, from
task-slot modules the step's resolver reaches like any other
(`platform/c64/resident_rupt_module.c`, the steward's call):

| Module | Table | LM bytes | CM bytes |
|---|---|---|---|
| 0x01E1 | the lead-ins (`rupt_lead_ins.h`; CM with the RESUME family) | 2,642 | 4,358 |
| 0x01E2 | the T4RUPT core (`t4rupt_core.h`) | 7,733 | 7,878 |
| 0x01E3 | KEYRUPT and UPRUPT (`keyrupt_uprupt.h`) | 6,384 | 6,379 |
| 0x01E4 | LM T6RUPT and the LM RESUME family (`lm_t6rupt_routines.h`) | 7,711 | |

Each wrapper binary-searches its one table for the routine at Z and runs it
with the resident's program and io (`rk_env`, the step's module value),
until Z reaches a point its table does not hold or 256 routines have run.
A routine's failure, or a first point its table does not hold, goes back
through `rk_env.status`, which the step returns. The task slot holds 8,186
bytes, so the wrappers link with `-mllvm -inline-threshold=-50`: inlining
the image operations into every routine is what overflows it (the LM
T4RUPT core is 8,506 bytes at -Oz alone). The store's resolver records for
them (`wrapper_records` in `tools/resident_flight_build.py`) are one per key
the resident's own tables do not hold, in the canonical form above.

`make test-resident-rupt` also runs 1,152 cases per mission through every
other vector (`tests/resident_vector_cases.c`, 128 per vector: T6RUPT,
T5RUPT, T4RUPT, KEYRUPT1, KEYRUPT2, UPRUPT, DOWNRUPT, RADARUPT, HANDRUPT).
Each takes its image and io from the Kernel lane's own generator for the
body the vector reaches (T4RUPT from `t4_input`, KEYRUPT1 and UPRUPT from
`ku_input`, LM T6RUPT from `lm_t6_input`, the rest from `rli_input`), then
moves Z to an erasable word before the interrupt with only that request
pending. The run (`tests/resident_vector_run.c`) takes the interrupt and
resolves each point over the resident's tables and the wrapped ones, until
RESUME brings Z back or the run reaches a point no table holds. yaAGC runs
the original from the same state to the same point
(`tools/resident_vector_stops.c` gives it each native run's end), and all
2048 words, the superbank bits, the interrupt enable and flag and channels
5, 6, 10, 11 and 13 must agree. All 2,304 cases pass: 229 LM and 323 CM
reach RESUME, the rest stop where a body continues into code the resident
does not hold yet (a Waitlist task, a banked T5RUPT handler). A 2CADR names
fixed-fixed code by its fixed-fixed address, as yaYUL assembles it, so the
cases do too: run through its banked alias, RESUME's XCH BBANK would switch
the bank under Z.

### The boundary-word table

The resident's `fixed_data` is `rk_fixed_word`. It reads the word at Z
from a table of every fixed point where the resident can stop between
routines: the keys of the resident's routine tables, of the interrupt
bodies (`rupt_lead_ins.h`, `t4rupt_core.h`, `keyrupt_uprupt.h`,
`lm_t6rupt_routines.h`) and of the store's resolver records, and the
interpreter's fetch point (LM NEWOPS 06070, CM 06037) and DANZIG (LM 06061,
CM 06030). At an interpretive order Z stands at one of those two while the
order's key is LOC's, so without their words an interrupt there would stay
pending (the whole-program differential lost 2,500 of 3,000 LM V37E00E
samples that way; docs/FLIGHT-STORE.md).
`tools/resident_boundary_words.py` collects the keys (it builds
`tools/resident_boundary_keys.c` against the tables), takes each word from
the unmodified rope, checks it against the yaYUL listing when one is built,
and writes `platform/c64/resident_boundary_words_<m>.h`: 69 LM and 61 CM
sorted {physical key, word} records. A key the listing has no word for is
an error.

The table lives at the end of the REU store image, not in the resident and
not in any slot, so it grows with the store's resolver records (the
whole-program store: about 2,500 LM and 2,800 CM words, past what one task
slot holds). The packer writes it there (`tools/overlay_v2_pack.py --tail`,
`tail_bytes` in the tool above): the entries, then their count and a zero
word, ending on the image's last page and inside one 64 KB REU bank. Boot
reads the count from the image's last four bytes and the table's address
from the page count; after that `rk_boundary_entry` fetches an entry with
one 4-byte DMA (16-bit address arithmetic, which the one-bank rule allows)
and `rk_fixed_word` binary-searches them, as the store's resolver does.
Lane A id 0x01E0 (`platform/c64/resident_boundary_module.c`), which carried
the table until it outgrew the task slot, is now a null module whose main
only returns: the load check times a cached call through it. A key that is not in
the table gives no word, so the interrupt entry refuses to vector there and
the request stays pending. A store builder that adds resolver records passes
them with `--resolver lm=JSON` so their keys get words; `make
resident-boundary-words` (part of `make test`) fails if a committed table no
longer matches the routine tables, the rope or the listing.

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
| Kernel (with the seam hooks) | 10,493 | 10,256 |
| Transfer ABI trampoline and Executive call sites | 3,501 | 3,433 |
| ABI 2 runtime and REU store | 5,235 | 5,235 |
| Single-precision subroutines and their table | 1,307 | 1,307 |
| Service table exports | 637 | 637 |
| AGC timers and interrupt entry (`rk_rupts`, with the scaled-time cap and the scalers) | 1,183 | 1,183 |
| Resident state, step (with the stack canary), boundary-word reader and direct calls (with the pinned module; a refusing loaded table falls through to the resolver; a direct-only module's point runs in the step that loads it) | 1,512 | 1,530 |
| IRQ entry (saves and restores $01; its count is at $6FBC) | 24 | 24 |
| C runtime and library (crt0, multiply, memcpy, memset) | 361 | 361 |
| **Resident, $0801 to the end of bss** | **24,253** | **23,966** |
| Free below the service table (of 26,367) | 2,114 | 2,401 |
| Service table at $6F00 (24 entries) | 72 | 72 |
| Boot-only, in the job slot (with the $D000 and $C000 boot loads) | 4,216 | 4,195 |

The kernel, including its operand reader (`inline_word`, about 0.7 KB), is
half of the resident. The seam hooks add 194 LM and 197 CM bytes to the
kernel. `rk_seams_exchange` finds the job context by comparing NEWJOB with
each multiple of 12, because NEWJOB/12 pulled in the 125-byte
`__udivmodqi4`. The single-precision component measures 172 bytes less
with its table linked than without it. `rk_fixed_word` is 253 bytes: it
programs the REU registers itself, because `apollo_reu_transfer`'s 32-bit
address arithmetic doubled it, and boot runs no module through
`apollo_ov2_run`, because a second caller stops LTO inlining it and costs
about 450 bytes. The wrappers add 55 bytes to the
resident: the step hands each module `rk_env` and returns its status. Both
missions fit with 2.2-2.4 KB to spare, so the kernel stays as it is.

Since flight modules share the resident's runtime (docs/FLIGHT-STORE.md,
runtime sharing), the resident exports 21 more services (service table
0x0203): `agc_transfer_run`, `agc_transfer_point`, `agc_fixed_physical`,
`memcpy` and 17 kernel image ops (`k_ca` ... `k_bzmf`). LTO can no longer
inline or specialise an exported function, which costs about 800 bytes in
the transfer ABI. Exports that measured too costly for the 2 KB floor stay
private: `ai_put` 604, `k_das` 716, `ai_word` 208, `k_incr` 118, `k_qxch`
86, `k_mask` 63 and `k_dcsf` 27 bytes. `rk_direct` (direct calls into a
loaded module) is 496 bytes. The check programs (`RK_RUNS_ONLY`) link with
the 3-entry table (0x0202), keep the kernel ops static and leave out
`rk_direct` (`RK_NO_DIRECT`), since their case code needs the space.

## Checks on the linked resident

`make vice-resident-flight` runs ten programs (`<m>-check-<part>.prg`, LM
and CM, parts 1 services, 2 jobs, 3 idle, 4 clock and T3RUPT, 5 the other
vectors and the boundary-word table) under VICE with the mission's REU
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
   A second step from fixed-fixed 07753 with FB 036000 and superbank bits
   0160 must find the same probe under its canonical record (bank 06000).
4. Runs one step from SPSIN with A = 0 and Q in fixed memory. It must go
   through the resident's SPSIN/SPCOS table and stop at Q with A = 0.
5. Checks that interrupts kept arriving through the resident vector.

Part 4 runs no kernel rows. Instead, with the CIA interrupt held off so
that no tick lands, it runs every 32nd T3RUPT case
(`tests/resident_rupt_c64_rows.h`, 64 per mission). A job-slot module
(0x01F7, `platform/c64/resident_flight_rupt_cases.c`) writes each case into
the image. The program sets T3RUPT pending and runs resident steps until
RESUME brings Z back. The image, channel 7, the interrupt enable and the
interrupt flag must give yaAGC's digest. Then, with the timer running,
after at least five interrupts TIME1 must count `RK_TICK_LAG` and
`rk_dilation` the rest, CIA1 timer A must be counting down from its latch,
and a backlog past the cap with T5RUPT and T6RUPT coming due must enter
T6RUPT alone (Scaled time, above).

Part 5, again with the CIA interrupt held off, runs every 16th other-vector
case (72 per mission, eight per vector). A job-slot module (0x01F8,
`platform/c64/resident_flight_vector_cases.c`) writes each case, and the
program runs resident steps with that request pending: the entry, then the
lead-in and the body from their wrapper modules, loaded from the REU into
the task slot, and the kernel where a body calls it. It stops when RESUME
brings Z back or a step finds neither a routine nor a module. The module,
called again with selector 1, checks the run's digest against yaAGC's;
the digest code, the frozen rows and the case itself live in the module
and in free RAM at $C200, because the program has no room for them beside
the resident. Last, every key of
the boundary-word table must give its word through `rk_fixed_word` and the
next key not in the table none; an entry at the first fixed-fixed key that
is not RELINT, INHINT or EXTEND must store its word in BRUPT; and an entry at
07754, not in the table, must take nothing and leave T3RUPT pending. With the
table's REU address four bytes off and the canonical key removed, the LM
program fails two checks; with the T4RUPT wrapper flipping a bit of A after
each routine, the LM part 5 program fails five.

| Program | Rows | Checks |
|---|---|---|
| LM services | 153 | 315 |
| LM jobs | 161 | 329 |
| LM idle | 86 | 179 |
| CM services | 129 | 265 |
| CM jobs | 110 | 227 |
| CM idle | 90 | 187 |
| LM clock and T3RUPT | 64 | 74 |
| CM clock and T3RUPT | 64 | 74 |
| LM other vectors and boundary words | 72 | 82 |
| CM other vectors and boundary words | 72 | 82 |

All 873 rows pass, the same rows as `make vice-resident-kernel`. A one-word
kernel mutation (CS TWO to CS THREE in DUMMYJOB) fails the LM idle program.
The host differential (`make test-resident-kernel`) is unchanged; its cases
now run as an input half and a kernel half
(`rk_*_input`, `rk_*_run`), which the C64 check splits across the module
and the resident.

## Evidence

In `evidence/resident-kernel/` (hashes in `SHA256.json`): the ten VICE
results with their PRG hashes (`resident-flight-vice.log`), the size report
(`resident-flight-sizes.json`, which `tools/overlay_allocation.py` copies
into docs/overlay-allocation.json), the host differential with the seam
counts (`resident-kernel-host.log`), the SPSIN/SPCOS results
(`sp-routines-host.log`), the clock and interrupt results against yaAGC
(`resident-rupt-host.log`), the scaled-time replay rows of the host
differential (`scaled-time-replay.json`), and the `flight_resident` section (with the
boundary-word table's check in `boundary_words`) of
`resident-kernel-proof.json` with the source and rope hashes.

## Not yet in the resident

- The flight store's resolver records. The committed boundary-word tables
  hold the check's two probe records; the flight store
  (`tools/flight_store_build.py`) builds its own boundary-word table with
  its records and ends its store with it.
- What the interrupt bodies continue into: Waitlist tasks T4RUPT and the
  T5RUPT handlers start, and the other lane's banked code a lead-in reaches.
  The step resolves those through the store like any module once their
  lanes' modules are in it.
- KEYRUPT and UPRUPT requests. The scheduler takes them (`RK_RUPT_KEY1`,
  `RK_RUPT_UP`) once the DSKY input sets them in `rk_pending`.
- The site operand table holds the Executive and Waitlist call sites only.
  The flight program's table grows with every module that calls them.
- DOWNRUPT in flight builds, which is a standing default (steward ruling,
  2026-10-14), not a gap. The AGC raises DOWNRUPT every 20 ms. The resident
  never requests it in flight, so a flight build sends no downlink. The
  harness's verification builds (`FLIGHT_DOWNRUPT=1`, `make
  flight-downlink-check` and `flight-downlink-vice`; see FLIGHT-DOWNLINK.md)
  raise it on the AGC's schedule, and DOWN-TELEMETRY certificates are judged
  on those. The reason is cost, measured in VICE on PAL with 100
  back-to-back passes:
  - LM: a median pass is ~130k cycles and 2 REU loads (the lead-ins and the
    10,570 B lm_downtelemetry module evict each other).
  - CM: a median pass is ~104k cycles and no loads.
  - At 50 passes per AGC second that is 6.6-6.9x real time on the LM and
    ~5.5x on the CM, for DOWNRUPT alone.
  - Loads are only 15-20% of the cost. With zero loads a pass would still
    take ~105k cycles, ~17-25k of it in resident overhead for each of its
    three steps. A hot/cold split would need ~5.1 KB pinned, and classes 3
    and 4 have 227 B free.
  Nothing on board reads downlink state, so guidance and the DAP are the
  same either way. Leaving DOWNRUPT out does give the other jobs the time
  its MCTs would have taken, so job timing differs slightly from yaAGC's.
  Revisit if the resident's per-step cost drops a lot.
  Numbers: /mnt/project-files/steward/downrupt-cost.md.
