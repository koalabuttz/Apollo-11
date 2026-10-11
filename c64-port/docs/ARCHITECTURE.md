# Architecture and original Executive investigation

## Translation boundary

The core executes Apollo control logic directly in C. Its arrays are native
data structures containing historical words, not emulated memory with an
instruction execution loop. Each saved core consists of the actual 12 fields;
the five VAC areas each retain 44 words including their use word. Locations and
bank values remain historical words, mapped as a pair to native continuations.
The live running pair is held in `exec_native.current`, because the original
dispatch epilogue writes scratch debris into core-zero LOC/BANKSET.

`src/executive.c` contains allocation, selection, context exchange and basic
sleep/wake/priority-change logic. `src/agc_word.c`
defines representations and necessary arithmetic. `platform/host` provides
snapshot transport. `platform/c64` provides diagnostic I/O and RAM mailboxes.
`src/native_dispatch.c` provides a cooperative native callback trampoline.
`src/irq_delivery.c` buffers IRQ-origin admission requests; optional platform
guards protect foreground mutation. The C64 raster harness preserves compiler
registers and uses a separate ISR stack (IRQ.md). `src/waitlist.c` retains WAITLIST/T3RUPT2/TASKOVER timer/list state;
`src/native_tasks.c` runs mapped foreground tasks under exclusion. A CIA1 Timer B
producer publishes clock ticks without dispatch. There is no general OS-service layer. Interfaces are explicit structures/functions suitable for later
component-by-component Rust replacement; no Rust is included.

## Core sets and priority representation

The historical definitions (`ERASABLE_ASSIGNMENTS.agc`, lines 374–383) allocate
eight sets of twelve words starting at MPAC, address octal 0154:

| Relative word | Historical field | Meaning |
|---|---|---|
| 0–6 | MPAC..MPAC+6 | Seven multi-purpose accumulator words |
| 7 | MODE | Interpreter scalar/vector mode |
| 8 | LOC | Basic positive / interpretive negative saved location |
| 9 | BANKSET | Saved BBANK plus superbank information |
| 10 | PUSHLOC | Packed interpreter pointer; sign can encode overflow |
| 11 | PRIORITY | Upper priority bits plus low nine-bit working-area base |

The running context is position zero. CHANJOB swaps all twelve context words
with the position named by NEWJOB; it does not simply move a queue node.
PRIORITY is **-0** for a free position, positive nonzero for active work, and
negative nonzero for dormant work. **+0 is not a valid core priority.**

There is no generic priority queue in this port. NOVAC scans positions in
ascending order, identifies exactly -0, and compares the **entire packed word**
against the priority at NEWJOB. Larger positive values win. Equal packed values
do not replace a pending candidate. Thus equal upper-five-bit priorities can
still compare differently because their low working-area bits differ.

Important experimentally verified correction to the initial brief: NO.CORES
contains decimal 7, but CCS's diminished-absolute-value down-counter yields
**eight** admission attempts (positions 0..7). After an occupied position 6,
EXECTEM2=1 is positive; CCS loads +0 and branches to NOVAC3, which tests position
7. Only the next CCS of +0 skips to the bailout. EJSCAN examines positions 1..7.
The stale Executive comment about "seven sets of eleven" is not implemented as
an allocation limit. Scenarios 12/13 and 24/25 establish the eighth admission
and genuine full-capacity bailout.

NEWJOB is -0 while idling, +0 when the running position is selected, or a positive
multiple of 12 for another candidate. Admission sets a candidate; it does not
immediately interrupt an arbitrary basic job. Interpreter polling (INTERPRETER
ADVANCE/CCS NEWJOB -> CHANG2) and basic job yield points cooperate with interrupt
requests. Preserving priority selection alone does not reproduce scheduling timing.

## VAC areas

Definitions at `ERASABLE_ASSIGNMENTS.agc`, lines 432–443, reserve five blocks of
44 words: a use word and 43 working words. Use addresses are octal 0400, 0454,
0530, 0604, 0660. Fresh-start code puts each use word's **own address** in it.
Zero denotes reserved. FINDVAC2 tests these words in order. CCS provides address
minus one; AD TWO yields the first working word. LXCH reserves the use word with
zero and leaves its former value in L. The working base is added to NEWPRIO.

NOVAC instead adds FAKEPRET = MPAC - 36 decimal = octal 0110. This lets later
interpreter-compatible addressing point into core-zero storage without a VAC.
ENDJOB1 distinguishes those cases through the low nine bits and FAKEPRET; for
a VAC it writes the use address back to the preceding use word.

## Calling conventions and dependencies

| Entry | Historical calling convention / behavior | Initial port |
|---|---|---|
| NOVAC | Priority in A; TC NOVAC followed by two-word 2CADR at Q; INHINT; return Q+2 via ENDFIND | Explicit C priority/location/bank arguments; stops before ENDFIND |
| FINDVAC | Same inline 2CADR convention; priority in A; first allocates VAC | Same bounded C interface |
| SPVAC | Priority already in NEWPRIO, 2CADR in A/L; caller inhibits interrupts; adjusts Q by -2 | Not implemented |
| CHANG1 | Basic job yield; Q -> L; saves bank; CHANJOB | `exec_change_basic`; explicit resume pair, native C ABI |
| CHANG2 | Interpreter yield; complement LOC; save banks; CHANJOB | Not implemented |
| JOBSLEEP | Fixed wake CADR in A; marks current priority dormant; EJSCAN | `exec_sleep_basic`; resources retained, explicit bank input |
| JOBWAKE | CADR in A; finds first matching dormant LOC; recomplements priority; repairs 2CADR | `exec_wake_basic`; all eight positions, caller L=0 adaptation |
| PRIOCHNG | New priority in A; preserve low nine bits; EJSCAN may resume caller or change jobs | `exec_priority_basic`; explicit resume pair and original CCS scan bias |
| ENDOFJOB | Switch Executive bank; ENDJOB1 releases resources then selects/dispatches | `exec_complete_basic`; basic dispatch, explicit SUPERBNK |
| DUMMYJOB / ADVAN | Idle flag, activity lamp, self-check execution, pending-job polling | Not implemented |
| SUPDXCHZ | 2CADR in A/L; writes SUPERBNK, BBANK, transfers through L | Bounded NUDIRECT startup adaptation; native pair lookup |

Supporting state includes Q, A/L, FBANK/BBANK/SUPERBNK, NEWPRIO, NEWLOC, LOCCTR,
EXECTEM1/2, FIXLOC, OVFIND, BUF..BUF+2, and the saved context arrays. Full support
also needs constants LOW9, TWO, NEG2, ZERO, bank masks, DSALMOUT, SELFRET, and
SELFCHK. ENDFIND restores the caller's bank; it does not RELINT. A caller must
restore interrupt permission according to its context. The native caller is
responsible for exclusion at raw primitive entry; bound native wrappers provide
it for one foreground caller and one non-nested IRQ producer. See IRQ.md.

Implicit semantics that mattered in this translation: CCS distinguishes both
zeros and sets diminished absolute value; INDEX accepts -0 as a zero index;
DCA/DXCH transfer paired A/L values; DXCH NEWLOC writes **A/L debris**, not the
old context's location, into NEWLOC; BZMF tests the uncorrected accumulator sign
and zeros; TS into erasable memory corrects the accumulator's extra sign bit.
Native snapshots preserve the relevant NEWLOC debris too. A/L/Q and scratch
bank/counter temporaries are outside the saved-state comparison boundary.

## WAITLIST, alarm, and restart relationships

WAITLIST is a distinct timer/task subsystem: TIME3 plus differential LST1 times
and LST2 2CADRs, at most nine tasks, executed with interrupts inhibited; tasks
finish via TASKOVER. T3RUPT is entered through INTERRUPT_LEAD_INS. A task may
request an Executive job. The bounded native translation now covers insertion, timer/list reload and
basic task completion; CIA1 Timer B publishes two-centisecond samples in the PAL
diagnostic. Foreground task delivery and per-call IRQ restoration adapt Apollo
interrupt timing. WAITLIST.md defines the exact state boundary, ENDTASK body
exclusion, scratch separation and ownership/source-rate limits.

FINDVAC2 reaches BAILOUT1 with octal 1201 when no VAC is free. NOVAC's exhausted
scan reaches BAILOUT1 with octal 1202 when all eight cores are occupied. The
VAC is allocated **before** the core scan, so the latter failure does not release
the reserved VAC. Native state retains that reservation and latches a bailout
boundary marker; subsequent admission is blocked pending an external reset.

BAILOUT1 (`ALARM_AND_ABORT`) records address/bank context, uses CHKFAIL1 and
FAILREG, drives the alarm output, then follows the abort route through WHIMPER /
ENEMA. FRESH_START_AND_RESTART, RESTARTS_ROUTINE, RESTART_TABLES, phase tables,
and device state rebuild protected work. CCSHOLE also handles impossible CCS
branches through a separate alarm path. The admission diagnostic still stops at BAILOUT1 entry. Separate bounded alarm
recording, STARTSB1 cleanup and protected Type C job stages are now implemented
(RESTART.md); complete abort/restart glue remains untranslated. A numeric marker
alone cannot stand in for recovery.

## Scope invariants

Inputs use canonical 15-bit words and priorities 01000..037000 with zero low
nine bits. VAC use words are zero or their defined positive use address. Saved
core priorities are -0 or nonzero packed active/dormant priorities; +0 is outside
this API's valid-state domain. NEWJOB is -0 or a valid core displacement. No
concurrent mutation occurs during an operation. Completion applies to a valid
running context at core zero. NEWJOB=+0 implies a valid current context; a free
core zero has NEWJOB=-0 or a positive queued candidate, never +0. The older `exec_endjob_select` result is a **boundary observation**.
`exec_complete_basic` extends it through native basic dispatch; interpreter
continuations remain rejected before release. See DISPATCH.md for the actual
entry/epilogue register adaptations and verified scope.

Sleeping contexts hold a fixed one-word CADR in LOC and current EBANK/SUPERBNK
flags in BANKSET. Wake reconstructs the basic 2CADR. Sleep retains core/VAC
ownership, so idle may contain dormant work and reserved VACs. Native callbacks
return explicit sleep/reprioritize actions before exchange; LIFECYCLE.md records
the API, scratch conventions and all 115 original-derived checkpoints.

The deterministic initializer additionally zeroes unused working words for tests.
That is not the full Apollo fresh-start routine and is not a promise to clear
VAC contents at admission. Neither code nor tests clear work areas when reusing
resources. No allocation, recursion, OS threads, or whole-rope resident image is
needed by the native core.

## Explicit bounded recovery state

`src/restart.c` records alarms, translates STARTSB1 cleanup with explicit device
inputs, validates protected phase pairs and re-enters Type C variable jobs. It
uses a separate 130-byte `restart_state` and 60-byte `restart_phases`; the existing
648-byte Executive and 58-byte WAITLIST representations are unchanged. Raw stages
require restoring exclusion and stopped custom publication sources. Cleanup
preserves alarm history and payloads rather than calling the test initializer.
Native runtime faults stay latched until explicit reinitialization. RESTART.md
separates each original-assembly stop boundary from the composed native driver,
which omits complete ENEMA/display/flag/return glue and remaining phase routes.

## Bounded guidance arithmetic

`src/agc_multiword.c` adds normal-memory MP and erasable-pair DAS with signed
zeros, low-word carry, independent word signs and full accumulator overflow
results. `src/guidance.c` directly translates RODCOMP's initial rate adjustment.
Its separate 12-byte state retains count, raw scale, desired-rate words and A/L;
it does not modify Executive scratch or assume a complete interpreter context.
The caller owns the state and excludes a count producer. The standalone target
uses restoring guards rather than carrying original INHINT into an untranslated
continuation. There is no AGC instruction decoder, floating point or allocation.
GUIDANCE.md defines the entry/stop boundary and excluded P66 work.
