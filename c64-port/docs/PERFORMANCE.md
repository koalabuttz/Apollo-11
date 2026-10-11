# Native C64 resource and timing findings

The retained artifacts use LLVM-MOS SDK v23.3.0, `-Os`, and PAL x64sc 3.7.1 with
ordinary ROMs. `executive.prg` is the unbound console regression; `irq.prg` and
`lifecycle.prg` use `-fno-static-stack -mlto-zp=0` and the bounded raster gate.
These are diagnostic samples, not flight-software bounds or AGC cycle equivalence.
Exact binaries, maps, logs, mailbox/stack dumps and hashes are in `evidence/`.
The three existing artifacts are unchanged from the lifecycle milestone. The
additional WAITLIST artifact and CIA measurements are recorded separately below.

## Image and RAM

| Quantity | Console regression | IRQ diagnostic | Lifecycle diagnostic |
|---|---:|---:|---:|
| PRG file, including two-byte load address | 21440 bytes | 23692 bytes | 28499 bytes |
| Loaded image | 21438 bytes, $0801..$5bbe | 23690 bytes, $0801..$648a | 28497 bytes, $0801..$7751 |
| BASIC SYS entry | $082b | $080d | $080d |
| Native text | 20392 bytes | 22715 bytes | 27802 bytes |
| Read-only data | 1004 bytes | 963 bytes | 683 bytes |
| Linked BSS | 658 bytes, $5bbf..$5e50 | 1668 bytes, $648b..$6b0e | 1663 bytes, $7752..$7dd0 |
| Linked `.noinit` | 658 bytes, $5e51..$60e2 | 1 byte, $6b0f | 1 byte, $7dd1 |
| Image + BSS/noinit | 22754 bytes | 25359 bytes | 30161 bytes |
| Compiler zero page | 91 bytes, $02..$5c | 32 registers, $02..$21 | 32 registers, $02..$21 |
| Executive state | 648 bytes | 648 bytes | 648 bytes |
| Admission-request buffer | Unbound | 32 bytes, four requests | 32 bytes, four requests |
| Reserved ISR software stack | None | 256 bytes inside BSS | 256 bytes inside BSS |
| Foreground software-stack sentinel extent below $d000 | 98 bytes | 1356 bytes | 762 bytes |
| ISR-stack sentinel extent | N/A | 16 bytes | 16 bytes |
| Hardware-stack sentinel extent below entry baseline | 25 bytes | 34 bytes | 35 bytes |

The guarded BSS includes two Executive copies for unchanged-state checks, queue,
runtime, saved registers and ISR stack. The target native runtime is 21 bytes,
including staged priority and callback-active metadata. The foreground stack
includes local fixtures and diagnostics; it is not all dispatch overhead. The
console build has a 658-byte static fixture frame instead. A single linked
runtime `.noinit` byte remains in each guarded artifact; application/producer
frames are dynamic and the ISR calls no console helpers. No core, dispatcher
or diagnostic heap allocation is used. All three images fit stock C64 RAM.

The console build places a 30-byte callback table in initialized zero page and
allocates 29 additional spill bytes. It is unsuitable for the IRQ gate's 32-byte
preservation contract. Guarded maps have no additional ZP allocation, checked
by their VICE helpers. The helpers discover SYS from the actual PRG header.

## Guarded IRQ operation samples

The IRQ monitor pairs stores $10/$20 at $c030, with group/case at $c031/$c032.
Setup, screen output and fingerprints are outside these intervals. The first
71 paths mask IRQs around the raw operation; two four-request batches use the
protected native poll. Intervals include marker/call overhead and VIC bus effects.
Raster production has stopped; stock KERNAL IRQs can still affect wrapper batches
and the guard pair. PAL rate is approximately 985248 cycles per second.

| Group | Samples | Maximum PAL cycles | Time | Case |
|---|---:|---:|---:|---|
| Admission / ENDJOB selection | 26 | 3820 | 3.877 ms | Scenario 13, eighth-core FINDVAC |
| Basic context exchange | 36 | 4535 | 4.603 ms | Dispatch 24 |
| Completion / basic dispatch | 6 | 6179 | 6.272 ms | Dispatch 37 |
| Bounded idle startup | 3 | 3771 | 3.827 ms | Dispatch 52 |
| Four-request safe point | 2 | 18450 | 18.726 ms | Case 8, four FINDVACs |
| Exclusion enter/leave | 1 | 24 | 0.024 ms | Restoring initially enabled I |

The 74 intervals cover all 26 old scenarios, 45 isolated dispatch fixtures,
two full buffered batches and one guard pair. The NOVAC batch is retained in JSON.
These are maxima over tested fixtures with five mappings, not all valid states
or table sizes. The guard pair is optimized diagnostic code, not a universal
indirect-hook cost. The largest batch nearly occupies a PAL frame; capacity alone
cannot establish a source-rate or scheduling guarantee.

## Actual IRQ and safe-point latency

Nine owned IRQ intervals run from marker $81, after source recognition and
acknowledgement, to $82, after register restore. Eight ordinary entries take
2060..2214 PAL cycles (2.091..2.247 ms); the register-poison canary takes 2630
cycles (2.669 ms). They include rc0..31 save/restore, separate-stack setup,
C publication, 16-byte ISR-local canaries and marker overhead.

Initial CPU entry/KERNAL prologue and final KERNAL epilogue are excluded. For the
pinned ROM and stack range, the unmarked instruction budget is 86 cycles: seven
hardware entry, 29 KERNAL prologue, 25 source recognition/first marker, and 25 final
jump/pop/RTI. This is instruction arithmetic, not a measured response bound:
instruction completion, masking, another handler and VIC stalls may add time.
Raster coordinates and CPU status at each marker remain in the monitor record.

The second two-job run omits hashing. It retains fixed 512-iteration native C
arithmetic, safety checks and payload touches, with a raster request armed in
the low callback. Recorded intervals are:

| Boundary | PAL cycles | Time |
|---|---:|---:|
| Low callback start to poll return | 163309 | 165.754 ms |
| IRQ start to poll return | 152788 | 155.076 ms |
| IRQ restore to poll return | 150574 | 152.829 ms |
| Poll entry to poll return | 3789 | 3.846 ms |
| Poll return to higher callback entry | 7811 | 7.928 ms |
| Deliberately masked three-arithmetic interval | 463075 | 470.009 ms |
| Pre-unmask to completed deferred delivery | 2567 | 2.605 ms |

IRQ publication leaves Executive state unchanged. Poll admits while the low
callback still runs; the higher callback begins after yield and another step.
The fingerprint-checked sequence is also retained, but hashing dominates its
latency and it is not used for this table. Application latency includes remaining
callback/poll work, masked work, source/ISR interference and guarded admission.
Dispatch adds the wait until an action return and the next step. The test source
has a finite budget; a continuous source could overflow during the long callback.
No unconditional request latency or flight deadline is established.

## Lifecycle operation and IRQ bridge samples

The lifecycle monitor pairs $10/$20 stores at $c048, with the case at $c04d.
Its 76 isolated fixtures run under exclusion; setup and fingerprinting remain
outside the measured operation. Intervals include marker/call overhead and VIC
bus effects. Five mappings are present.

| Group | Samples | Maximum PAL cycles | Time | Case |
|---|---:|---:|---:|---|
| JOBSLEEP / exchange or idle | 22 | 5280 | 5.359 ms | 11 |
| JOBWAKE / first dormant match or no match | 29 | 4111 | 4.173 ms | 37 |
| PRIOCHNG / direct return or exchange | 25 | 5537 | 5.620 ms | 57 |

After the full fingerprint suite, a VAC-owning job sleeps. A real raster IRQ
publishes an admission for the waker, preserving Executive state. Foreground
admission/start runs that job; it wakes the sleeper and lowers its own priority.
The sleeper resumes with retained payload/overflow/VAC, finishes and releases
its VAC, then the waker completes. The additional bridge intervals are:

| Boundary | PAL cycles | Time |
|---|---:|---:|
| Enable marker to observed publication | 2585 | 2.624 ms |
| Observed publication to awakened dispatch | 64076 | 65.035 ms |
| Awakened completion | 10280 | 10.434 ms |
| Waker completion | 6234 | 6.327 ms |

The 64076-cycle interval includes a full 648-byte unchanged-state comparison,
safety checks, and native admission/start/wake/priority actions. It is not a
scheduler-only latency measurement. Bridge intervals omit hashing but retain
integration checks; the single finite IRQ event is not a continuous-source bound.
The timing JSON retains all intervals, markers and stack observations.

## Console regression and stack limits

The console diagnostic still passes 135 original-derived fingerprints and eight
native two-job sequences. Four IRQ-enabled samples are 1187 cycles for idle NOVAC,
2129 for full-core FINDVAC/1202, 5395 for slot-seven completion/dispatch and 4011
for slot-seven CHANG1. Compiler flags, LTO layout and IRQ phase differ, so these
cannot establish guarded-build costs. Older measurements refer to older binaries.

Sentinels give test-specific lower bounds: coincidental $cd values, untouched
paths or IRQ phase can hide deeper use. Hardware instrumentation spans
$0100..$01f1, protecting the 14-byte BASIC/start chain. Guarded extents correspond
to about 48/49 bytes including that chain. Foreground software instrumentation
spans $c100..$cfff. ISR locals, interrupted arithmetic and explicit SP/register
canaries complement these observations. Reserve the full hardware-stack page
and conservative separate software-stack space; no complete BASIC exit or future
callback stack bound is established.

The SDK exposes RAM under BASIC, retains KERNAL/I/O and sets software-stack top
$d000. Respect $0000/$0001, compiler zero page, hardware stack, screen RAM, image
and BSS, plus mailboxes $c000..$c00c (console), $c020..$c026 and $c030..$c032 (IRQ),
and $c040..$c04d (lifecycle). Target `int`/pointers are 16 bits. The trampoline
retains no suspended C stack; persistent job values belong in MPAC/VAC. Physical
C64, NTSC, interpreter dispatch and guidance feasibility remain unmeasured.

## WAITLIST diagnostic resources and timings

The fourth artifact uses the same guarded compiler flags. Its retained file is
31804 bytes; the loaded image is 31802 bytes at $0801..$843a, with SYS $080d.
Text is 30315 bytes ($080d..$7e77), read-only data 1475 bytes ($7e78..$843a),
BSS 1793 bytes ($843b..$8b3b), and `.noinit` one byte at $8b3c. Image plus BSS /
noinit totals 33596 bytes, with heap start $8b3d; no heap is used. Compiler ZP is
only the preserved 32 registers. This diagnostic fits stock C64 RAM.

Task state is 58 bytes, task runtime 18 bytes and tick publication three bytes.
The diagnostic includes two task states, two 648-byte Executive states, a 32-byte
admission buffer for bound Executive helpers, saved IRQ registers and a 256-byte
ISR stack. Foreground sentinel extent is 105 bytes, ISR extent 16, and hardware
extent 31 below the protected 14-byte entry chain (about 45 total observed bytes).
These are lower-bound observations with the same sentinel limits described above.

156 isolated operations pair $10/$20 stores at $c058, with case at $c05d.
Source is not armed during this suite. Setup and fingerprinting are outside each
measured primitive interval; marker/call/VIC bus effects remain included.

| Group | Samples | Maximum PAL cycles | Time | Case |
|---|---:|---:|---:|---|
| WAITLIST insertion, including capacity failures | 120 | 8660 | 8.790 ms | 3, full list displaced before 1203 |
| T3RUPT2 dispatch/reload | 24 | 4018 | 4.078 ms | 125 |
| TASKOVER chain/return | 8 | 4187 | 4.250 ms | 145 |
| Invalid-delay alarm boundary | 4 | 658 | 0.668 ms | 154 |

The complete fingerprint suite also checks synthetic multi-wrap PINC loops.
Those large elapsed-time tests mask IRQs with the CIA B source stopped; they
are correctness fixtures, not admissible continuous-timer service budgets.

Six CIA IRQ marker intervals ($81/$82 at $c060) are 2377, 1853, 1853, 1854,
1876 and 1790 PAL cycles. The first is a poison-register canary (2.413 ms);
the five normal entries span 1790..1876 cycles (1.817..1.904 ms), including
register/stack preservation, ISR locals, two tick publications and marker stores.
CPU/KERNAL entry, ICR read/selection and final exit/chaining are excluded.
Coincident Timer A service adds the stock KERNAL clock/keyboard routine after
register restore. No universal interrupt-response bound is inferred.

A 10 ms trial left insufficient margin between worst observed insertion and IRQ
handling. The retained source uses 19705 cycles per event, approximately 20 ms,
publishing two logical centiseconds. Four finite periodic events occur during
unmasked checked native arithmetic. Their three start-marker periods are 19706,
19705 and 19704 cycles; small entry-phase variation does not indicate tick loss.
No elapsed tick is inferred for a coalesced CIA flag. The longest mask plus
handler/entry/other-source delay must stay below the configured period.

The separate one-event task/job bridge records:

| Boundary | PAL cycles | Time |
|---|---:|---:|
| Pre-unmask marker to observed publication ($92..$93) | 21560 | 21.883 ms |
| Observed publication to selected task ($93..$94) | 52547 | 53.334 ms |
| Task callback admission/wake and TASKOVER ($94..$95) | 8441 | 8.567 ms |
| Resumed sleeper and admitted-job completion ($95..$96) | 13584 | 13.787 ms |

The 52547-cycle interval includes both unchanged-state comparisons (648-byte
Executive and 58-byte task state), safety checks, clock collection and dispatch.
It is not a scheduler-only latency measurement. Callback time includes guarded
Executive helpers and assertions. The final interval covers two separate job
steps with restoring I state, not one continuous mask. Source has a finite budget
and stops after publication, so these bridge intervals do not prove continuous
source capacity or application deadlines. Odd-centisecond deadlines additionally
wait for two-centisecond sampling before foreground collection.

The helper retains all events/intervals, verifies periodic spacing and rejects
extra compiler ZP or exhaustion of instrumented stacks. Reserve $c050..$c05d and
$c060 for mailbox/timing stores in addition to the earlier diagnostic regions.
CIA ownership, stock-ROM restoration, callback/mask constraints, and unsupported
NTSC/NMI/multi-source behavior are defined in WAITLIST.md.

## Sixth milestone: standalone alarm/restart diagnostic

`evidence/restart.prg` is 21796 bytes including its load address; 21794 bytes load
at $0801..$5d22, SYS $080d. The map reports 20712 bytes of text
($080d..$58f4), 1070 of read-only data ($58f5..$5d22), 902 of BSS
($5d23..$60a8) and one noinit byte at $60a9. Loaded image plus BSS/noinit is
22697 bytes. Only the 32 LLVM-MOS imaginary-register ZP bytes are used; there
is no additional ZP data/BSS allocation. Executive/task states remain 648/58
bytes; added modeled restart state/phase records are 130/60 bytes. The full test
case is 902 bytes. No heap allocation or custom IRQ stack is used by this PRG.

The final PAL VICE run observes 29 bytes below the $01f2 hardware-stack baseline
(43 bytes including the 14-byte pre-existing BASIC/KERNAL frame) and 146 bytes of
instrumented dynamic software-stack extent. Sentinel extents are observations,
not stack proofs. The diagnostic reserves $c080..$c08c for mailbox/markers and
$c100..$cfff for software-stack instrumentation.

| Sample | PAL cycles | At 985248 cycles/s |
|---|---:|---:|
| Fourth alarm recording, register history full | 332 | 0.337 ms |
| STARTSB1 cleanup | 3735 | 3.791 ms |
| Six protected phase-pair checks | 956 | 0.970 ms |
| Type C NOVAC admission with a late free core | 4191 | 4.254 ms |
| Complete recovery demo setup and six native completions | 135110 | 137.133 ms |

The four primitive intervals are guarded; setup and fingerprint hashing are
outside their markers. The integration interval includes input construction,
failed-admission/runtime checks, bounded reconstruction, mapping initialization,
six separate native job steps and outcome checks. It is not one continuous mask
or a scheduler-only recovery measurement. VIC/IRQ timing and marker overhead are
included. Source publication is stopped during recovery; this milestone makes no
continuous CIA-source deadline guarantee. The full 226-case 448-word fingerprint
workload takes many emulated cycles and uses a one-billion-cycle harness limit.
RESTART.md defines the bounded recovery contract and remaining source exclusions.

## Seventh milestone: standalone P66 rate and arithmetic diagnostic

`evidence/guidance.prg` is 10902 bytes including the load address; 10900 bytes
load at $0801..$3294, SYS $080d. Text is 3891 bytes ($080d..$173f), read-only
data is 6997 ($1740..$3294), BSS is 13 ($3295..$32a1), and noinit is one byte
at $32a2. Loaded image plus BSS/noinit is 10914 bytes. Read-only data includes
6816 bytes of complete expected output words. The native rate state is 12 bytes,
and the test case is 13. No heap, custom IRQ stack or floating point is used.
The compiler uses its 32 imaginary-register ZP bytes with no extra ZP data/BSS.

The final PAL run observes 23 bytes below the $01f2 hardware-stack baseline
(37 including the existing 14-byte BASIC/KERNAL frame) and 26 bytes of dynamic
software-stack sentinel extent. These are observed extents, not stack proofs.
The diagnostic reserves $c080..$c088 and instruments $c100..$cfff.

| Sample | PAL cycles | At 985248 cycles/s |
|---|---:|---:|
| Positive maximum MP product | 1711 | 1.737 ms |
| Negative maximum MP product | 1920 | 1.949 ms |
| DAS low-word carry | 478 | 0.485 ms |
| DAS high-word overflow | 345 | 0.350 ms |
| Zero-count P66 rate update | 553 | 0.561 ms |
| Maximum-count P66 rate update | 2108 | 2.140 ms |

Each interval covers `gn_apply` operation selection and the native call between
marker stores under exclusion. State construction, guard entry/exit and complete
word comparisons lie outside the markers; marker overhead and VIC bus effects
are included. Zero-count uses a zero scale; the larger sample uses negative
maximum count/scale and an existing mixed-sign desired-rate pair. These samples
are not worst-case bounds, whole P66 timing, continuous-source capacity or flight
deadline evidence. The count producer is a test input, with no custom live IRQ.
GUIDANCE.md records original inhibition versus the restoring native boundary.

## Eighth milestone: standalone pre-interpreter preparation diagnostic

`evidence/prepare.prg` is 34921 bytes including its load address; 34919 bytes
load at $0801..$9067, SYS $080d. Text is 9148 bytes ($080d..$2bc8), read-only
data is 25759 ($2bc9..$9067), BSS is 78 ($9068..$90b5), and noinit is one byte
at $90b6. Loaded image plus BSS/noinit is 34998 bytes. Read-only data includes
25272 bytes of complete expected words and 324 of expected skip masks. This
standalone test image is not a combined flight image. The 78-byte preparation
state includes the existing 12-byte rate state. No heap or floating point is
used. Only the 32 compiler imaginary-register ZP bytes are allocated, with no
additional ZP data/BSS.

The PAL run observes 31 bytes below the $01f2 hardware-stack baseline (45
including its existing BASIC/KERNAL frame) and 191 bytes of dynamic software-stack
sentinel extent. These observed extents include full-state checking and its
local composed case, not just the native preparation function. They are not stack
proofs. Mailbox is $c080..$c088; software-stack instrumentation is $c100..$cfff.

| Sample | PAL cycles | At 985248 cycles/s |
|---|---:|---:|
| PIPA/time snapshot | 1059 | 1.075 ms |
| Vector inputs without TS skips | 4292 | 4.356 ms |
| Six skips, negative final clear stores | 3984 | 4.044 ms |
| Six skips, positive final clear stores | 3996 | 4.056 ms |
| Complete rate/snapshot/vector preparation | 6692 | 6.792 ms |

Calls are guarded; input setup, guard entry/exit and full-state comparisons are
outside markers. The complete-call sample is the first step of history 144,
with its explicit count/sensor/time inputs and preserved stale words. Marker
overhead, sample dispatch and VIC bus effects are included. Separate stage calls
use different inputs and bus phases, so their intervals need not sum to the
composed interval. No custom sensor IRQ or continuous source runs in this PRG.
These are finite samples, not a worst-case execution bound, full P66 timing or
flight deadline evidence. PREPARATION.md documents ownership and the interpreter
boundary.

## VXSC KPIP1 diagnostic

The eighth standalone PRG has 32979 file bytes / 32977 loaded bytes at
$0801..$88d1, SYS $080d. Text is 8818 bytes, read-only data 24147 (23944 are
292 full 41-word expectations), BSS 82 and noinit one byte. Total image plus
BSS/noinit is 33060 bytes. It fits stock C64 RAM, uses 32 imaginary ZP registers
and no extra compiler ZP, heap or floating point. Four new scratch bytes plus
unchanged 78-byte preparation state give 82 target bytes.

| Guarded sample | PAL cycles | Time |
|---|---:|---:|
| Below-half third word | 23415 | 23.766 ms |
| Exact-half third word | 24297 | 24.661 ms |
| Positive rounding propagation | 24799 | 25.170 ms |
| Negative rounding propagation | 25737 | 26.122 ms |
| Full RODCOMP prefix plus scaling, case 280 step 0 | 19838 | 20.135 ms |

These include marker/call and VIC bus overhead, with inputs/checks outside the
interval. The largest sample exceeds one PAL frame while interrupts are masked;
a live guidance job requires explicit source-rate/exclusion and scheduling
budgets. No WCET or flight-deadline conclusion follows from these five samples.
Hardware-stack extent is 26 bytes below entry, 40 including the 14-byte ROM
frame; software-stack extent is 187 bytes including diagnostic fixtures.
`scale-timing.json`, the exact PRG/map and VICE logs/dumps are retained. Earlier
seven PRGs and their measured evidence are unchanged. SCALING.md defines scope.

## PDDL THISTPIP diagnostic

The ninth standalone PRG is 46543 file bytes / 46541 loaded bytes at
$0801..$bdcd, SYS $080d. Text is 11925 bytes, read-only data 34604 (34400 are
200 full 86-word expectations), BSS 172 and noinit one byte. Image plus
BSS/noinit is 46714 bytes, ending at $be7a. It fits stock C64 RAM with 32 imaginary
ZP registers and no extra compiler ZP, heap or floating point. The owned VAC
payload/FIXLOC/PUSHLOC add 90 bytes; combined modeled state is 172 bytes.

| Guarded sample | PAL cycles | Time |
|---|---:|---:|
| PDDL at VAC start | 912 | 0.926 ms |
| PDDL at final six-word slot | 1028 | 1.043 ms |
| PDDL in fifth VAC | 1051 | 1.067 ms |
| PDDL after scaled skip input | 1033 | 1.048 ms |
| Full RODCOMP through PDDL, case 188 step 0 | 20790 | 21.101 ms |

Intervals include marker/call and VIC bus overhead, with setup/checks outside.
The sampled full prefix exceeds one PAL frame under masking; a live job still
requires an exclusion/source-rate and scheduling budget. No WCET or deadline
bound is inferred. Hardware-stack extent is 32 bytes below entry (46 including
the 14-byte ROM frame); software-stack extent is 472 bytes, including full-state
and rejection fixtures. Earlier eight PRGs and their measured evidence are
unchanged; diagnostic images are not combined into a flight image. Exact
binary/map, logs/dumps and `push-timing.json` are retained. See PUSHDOWN.md.

## DSU PIPTIME diagnostic

The tenth standalone PRG passes 172 complete 89-word/path checks in PAL VICE,
including rare resolved overflows, all integrated skip inputs and three retained
histories. File size is 44887 bytes, loaded image 44885 at $0801..$b755;
text 13748, rodata 31125, BSS 178 and noinit one byte. Image plus BSS/noinit
is 45064 bytes, ending at $b808. Frozen expected words/masks occupy 30788 bytes.
These standalone diagnostic images are not a combined flight workload.

| Guarded sample | PAL cycles | At 985248 Hz |
|---|---:|---:|
| No overflow | 861 | 0.874 ms |
| Resolved positive overflow | 1251 | 1.270 ms |
| Positive final-only skip | 1381 | 1.402 ms |
| Negative two-skip overflow | 1304 | 1.324 ms |
| Full RODCOMP through DSU, history step 0 | 21427 | 21.748 ms |

Marker/call and VIC bus effects are included; setup/full comparisons are outside
these intervals. Full-prefix masking exceeds a PAL frame. No WCET, continuous
source-rate or flight-deadline guarantee follows from these samples. Observed
hardware-stack extent is 38 bytes below entry (52 including ROM's 14-byte frame);
software-stack extent is 313, including the complete fixture/checker. The build
uses `-fno-static-stack -mlto-zp=0`, 32 imaginary ZP registers and no extra ZP,
heap or floating point. Total modeled state is 178 bytes; elapsed PIPTIME/BUF
adds six bytes. Exact logs, map, PRG, dumps and timing JSON are in `elapsed-*`.
ELAPSED.md defines arithmetic and ownership; STORE 30D remains next.

## STORE 30D diagnostic

The eleventh standalone C64 PRG checks 176 full 89-word/path states with zero
failures. PRG file 46651 bytes / loaded 46649 at $0801..$be39; text 14792,
rodata 31845 (31504 expectation bytes), BSS 178 and noinit one byte. Image plus
BSS/noinit is 46828 bytes, ending at $beec. A preflight check keeps image/state
below $c000, separate from the completion mailbox and instrumented stacks.

| Guarded sample | PAL cycles | At 985248 Hz |
|---|---:|---:|
| Direct STORE cases 0 / 1 / 15 / 18 | 279 / 317 / 236 / 314 | 0.283 / 0.322 / 0.240 / 0.319 ms |
| Full RODCOMP through STORE, history step 0 | 21841 | 22.168 ms |

Marker/call and VIC bus costs are included, setup/full checks excluded. Full
prefix masking exceeds one PAL frame. These finite samples are not WCET,
source-rate or flight-deadline bounds. Observed hardware-stack extent is 44
bytes below entry (58 including ROM frame); software-stack extent 498 includes
full-state rejection fixtures. The build has 32 imaginary ZP registers, no
extra ZP, heap or floating point. State stays 178 bytes. STORE.md defines scope;
logs, PRG/map, dumps and timing JSON are retained under `store-*`.

## DDV 4SEC(28) diagnostic

The twelfth standalone PRG passes 156 complete 93-word/result expectations.
File 46492 bytes / loaded 46490 at $0801..$bd9a; text 16907, rodata 29571
(29172 expectation bytes), BSS 186 and noinit one byte. Image plus BSS/noinit
is 46677 bytes, ending at $be55, below reserved diagnostic RAM. Full context
is 186 bytes; the four new scratch words add eight bytes.

| Guarded sample | PAL cycles | At 985248 Hz |
|---|---:|---:|
| Zero | 447 | 0.454 ms |
| Positive quotient | 6236 | 6.329 ms |
| Negative quotient | 6423 | 6.519 ms |
| Overflow | 1222 | 1.240 ms |
| Full RODCOMP through DDV, history step 0 | 27940 | 28.358 ms |

Setup/full comparisons are excluded; marker/call and VIC bus costs included.
Full prefix masking exceeds one PAL frame. No WCET, live producer-rate or flight
budget follows. Hardware-stack extent is 44 bytes below entry (58 with ROM),
software extent 324 including the full checker. Compiler uses 32 imaginary ZP
registers and no extra ZP, heap or floating point. Separate diagnostics are not
a combined flight image. Evidence is under `divide-*`; DIVIDE.md qualifies the
hardware-DV oracle and scope. Pending PDVL GDT/2 remains next.

## PDVL GDT/2

PRG40690 bytes; text19665/rodata21011/BSS198/noinit1, highestRAM$a7b7.
ZeroextraZP; stack50 below$01f2/software371. Four sampledPDVL intervals615..736
PALcycles; fullprefix28164 (28.586ms). Lossless fixture20649 bytes. NoWCET claim.

## Verified native VAD/VSU

Reusable vector arithmetic now matches 13,600 original-assembly checkpoints.
128 complete states pass host, sanitizer and C64 diagnostics. See [VECTOR.md](VECTOR.md).
