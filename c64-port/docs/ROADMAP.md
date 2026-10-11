# Milestones and next work

## Completed: basic context exchange and native dispatch

- Basic CHANG1/CHANJOB exchange, MPAC/MODE, LOC/BANKSET debris, PUSHLOC/PRIORITY,
  FIXLOC and OVFIND behavior match controlled original assembly executions.
- Historical two-word addresses map explicitly to native C continuations.
- Eight sequential two-job executions cover admission, yielding, completion,
  priority relations and stale core/VAC data; 53 cases / 109 checkpoints match.
- Actual native callbacks run on host and PAL C64 VICE, with fresh evidence.
- No generic ready queue or interpreter dispatch was introduced.

Acceptance for this milestone is met: two native basic jobs with different
priorities are executed, yielded and completed, with resource/state exchanges
and observed continuation order matching controlled original executions.
See DISPATCH.md for the precise continuation and startup adaptations.

## Completed: platform exclusion and interrupt request delivery

- Nested C64 guards restore the previous IRQ-enable state on success and faults.
- A four-entry FIFO buffers requests from one non-nested IRQ producer; foreground
  safe points admit them without preempting a C callback.
- Actual VIC raster IRQs preserve A/X/Y, P, all 32 imaginary-register bytes and
  the foreground software stack. An isolated ISR stack is exercised.
- Overflow fail-stops separately from exact retained 1201/1202 boundaries.
- Host wrap/fault tests, sanitizers, unchanged original comparisons, nine real
  C64 IRQs, eight original-derived checkpoints and 74 timing samples pass.

Acceptance for this bounded milestone is met. IRQ.md defines its ownership and
reset contract. PERFORMANCE.md reports observed operation and latency samples;
callback/source/mask bounds are still application requirements, not flight proof.

## Completed: basic sleep, wake and priority changes

- JOBSLEEP retains core/VAC ownership and complements dormant packed priorities.
- JOBWAKE scans all eight contexts, wakes the first matching dormant CADR and
  reconstructs the original 2CADR without dispatching a live callback.
- PRIOCHNG preserves low bits and the original CCS threshold/tie behavior.
- Native sleep/priority actions take effect after C frame return; foreground
  wake and action mutation preserve previous IRQ permission.
- 81 cases / 115 full-state/transfer checkpoints match original assembly.
  Five native sequences, host fault/guard tests and sanitizers pass.
- A third PAL C64 diagnostic checks every fingerprint, retained payloads, a real
  IRQ-admitted waker and 76 guarded primitive intervals.

Acceptance for this bounded milestone is met. LIFECYCLE.md records the entry
scratch adaptations, physical bank checks, synthetic boundaries and exclusions.

## Completed: WAITLIST and CIA task timer adapter

- Nine LST2 task pairs, eight differential LST1 intervals, TIME3 and RUPTAGN
  retain original insertion/reload/TASKOVER semantics and signed-zero behavior.
- 1203 full-list mutation and 1204 delay boundaries match original execution.
- Native task callbacks run excluded, reschedule inside a task and return through
  TASKOVER; ten actual task sequences match independent assembly snapshots.
- 196 cases / 306 checkpoints, four host/sanitizer suites and a fourth C64
  diagnostic pass. Six real CIA B IRQs preserve registers and native arithmetic,
  keep KERNAL Timer A service and drive task admission/wake of native jobs.
- PAL CIA1 B samples two centiseconds per 19705-cycle event. Continuous-source
  capacity, masks and polling remain application requirements (WAITLIST.md).

Acceptance for this bounded milestone is met. Native foreground task delivery,
per-call restoring I state, scratch separation and the excluded SVCT3 body are
explicit adaptations; this does not complete Apollo interrupt/task hardware.

## Completed: bounded alarm recording and protected Type C job recovery

- ALARM1/BAILOUT1/POODOO1 record alarms through their explicit entry/return
  boundaries, preserving history and lamp memory.
- STARTSB1 cleanup retains payloads and phases while restoring scheduler resources
  and modeled device inputs. Channel-13 trap latches are separate from readback.
- GOPROG3 validates protected phase pairs; Type C variable job demands re-enter
  through original FINDVAC/NOVAC semantics.
- 190 cases / 226 checkpoints and five host/sanitizer suites pass. A fifth C64
  diagnostic verifies all fingerprints and six recovered native job completions.

Acceptance for this bounded milestone is met. RESTART.md distinguishes the
independently verified source stages from the composed native recovery adaptation.
Complete ENEMA glue, table/timed/display/Type B/longcall routes, phase updates,
POODOO phase/service cleanup and hardware restart remain untranslated.

## Completed: P66 rate adjustment and required multiword arithmetic

- RODCOMP's first six instructions consume RODCOUNT, multiply RODSCAL1 and add
  the double-word product to VDGVERT; the boundary precedes PIPA/interpreter work.
- MP preserves memory-zero/accumulator-zero distinctions. DAS preserves low-word
  carry, independent result signs and full 16-bit A/L overflow indicators.
- 12,835 cases / 12,856 checkpoints match actual Virtual AGC instructions and
  unchanged original RODCOMP; earlier snapshots and the rope remain unchanged.
- Six host/sanitizer suites pass. A sixth C64 diagnostic checks 568 complete
  states and three repeated rate histories, restoring I, ZP and stack canaries.
- State is 12 target bytes; larger sampled rate update is 2108 PAL cycles.

Acceptance for this bounded numerical/guidance milestone is met. GUIDANCE.md
defines raw scales, input domains and interrupt/scratch adaptations. This does
not complete P66 or establish an application deadline.

## Completed: RODCOMP PIPA/time and vector-input preparation

- Current and prior PIPA/time snapshots, RUPTREG exchange and retained A/L debris
  match original source through the snapshot boundary.
- Six AD/TS effects preserve skipped loads and skipped final CAF ZERO. MODE is
  MPAC +7; untouched MPAC +2 and DELVROD low words remain intact.
- 1171 cases / 3396 checkpoints match all 39 words and real next-PC skip effects,
  including every skip mask in both sign families and three repeated histories.
- Seven host/sanitizer suites pass. A seventh C64 PRG checks 324 complete states,
  direct composed histories, restoring I, restricted ZP and stack sentinels.
- State is 78 bytes; complete sampled preparation takes 6692 PAL cycles.

Acceptance for this bounded milestone is met. PREPARATION.md defines ownership
and the stop before TC INTPRET at 32,3353. No interpreter or complete P66 job is
claimed; stable sensor/time inputs and native scratch remain explicit adaptations.

## Completed: P66 VXSC KPIP1 scaling

- Independent native DMPSUB/VROUND arithmetic preserves signed zeros, mixed signs,
  intermediate correction, exact-half rounding and final MPAC/A/L scratch.
- 3355 cases / 3364 original checkpoints match 41 modeled words. Genuine full
  interpreter entries verify RELINT and constant-bank resolution.
- Eight host/sanitizer suites pass; an eighth C64 PRG checks 292 full states.
- Four added scratch bytes bring combined state to 82 bytes. The largest sampled
  call is 26.122 ms; the sampled prefix plus scaling is 20.135 ms.

Acceptance for one bounded vector operation is met. SCALING.md defines the
numeric boundary and omitted interpreter machine-control state. A flight deadline
and complete P66 interpreter continuation are not established.

## Completed: PDDL THISTPIP

- An owned 43-word VAC payload plus raw FIXLOC/PUSHLOC preserve the six scaled
  vector words, timestamp load, DP MODE and MPAC/A/L exchange debris.
- All five VACs at all 38 legal offsets match original assembly: 2323 cases /
  3496 complete 86-word checkpoints, with three retained four-push histories.
- Nine host/sanitizer suites pass; the ninth C64 PRG checks 200 full final states.
- Native scope rejection leaves owned objects unchanged. Total state is 172 bytes;
  PDDL-only samples reach 1.067 ms, sampled full prefix 21.101 ms.

Acceptance for this bounded operation is met. PUSHDOWN.md defines ownership and
the stop before DSU. Interpreter resume, live producers and flight deadlines
remain future work; the native VAC is not implicitly bound to an Executive job.

## Completed: DSU PIPTIME

- DP subtraction preserves signed zeros, word carry, mixed signs and the exact
  OVERFLOW realignment, including initial overflow resolved without OVFIND write.
- 3491 cases / 4664 complete 89-word checkpoints match original assembly and
  actual TS 7 skips; three retained histories preserve saved vectors/VAC state.
- Ten host/sanitizer suites pass; the tenth C64 PRG checks 172 complete states.
- Total state is 178 bytes. DSU samples reach 1.402 ms; full prefix 21.748 ms.
- Twelve byte-identical source overlays retain the same assembled rope.

Acceptance for this bounded operation is met. ELAPSED.md defines stable PIPTIME,
BUF/OVFIND ownership and the stop before STORE 30D. Live job integration still
requires producer exclusion and a scheduling budget.

## Completed: STORE 30D

- DP MPAC stores to VAC-relative words 30/31; A clears and old word 31 remains
  in L. All other modeled words, including PUSHLOC, are retained.
- 3681 cases / 5044 checkpoints match original assembly, covering all 190
  VAC/push positions, 35 overlapping saves and 256 source/destination corners.
- Eleven host/sanitizer suites pass; the eleventh C64 PRG checks 176 full states,
  native rejection, restoring I and reserved memory layout.
- State stays 178 bytes; STORE samples reach 0.322 ms; full prefix 22.168 ms.

Acceptance for this bounded operation is met. STORE.md defines the native scope
check, old-word register effects and stop before DDV/PDVL.

## Completed: DDV 4SEC(28)

- Fixed raw divisor {0,400}; exact sign agreement, word normalization, integer
  quotient stages, signed-zero/overflow and retained register/scratch effects.
- 6735 cases / 8098 complete 93-word/result checkpoints match original assembly
  with pinned hardware DV. The fast-DV negative-zero discrepancy is audited;
  a one-line host-only branch selects the existing hardware model.
- Twelve host/sanitizer suites and the twelfth C64 PRG's 156 full states pass.
- Total context 186 bytes; DDV samples 6.519 ms; full prefix 28.358 ms.

Acceptance is met for the fixed-divisor domain and declared hardware-model
oracle. DIVIDE.md records the model difference and explicit interpreter boundary.

## Completed: PDVL GDT/2

Quotient pushdown and gravity-vector load match all 99 words over 5218 original
checkpoints. Every legal DP push position, integrated skip inputs and retained
histories pass; 156 complete states pass on C64. Lossless compressed expectations
reduce fixture storage while retaining exact comparison. See GRAVITY.md.

## Next guidance group: updated velocity

VSU VBIAS, implicit quotient-fed VXSC, VSL2, VAD V, implicit vector VAD and
STOVL 24D/R. Reusable arithmetic, Executive and phase services proceed as
separate independently verified commits. Complete P66 then requires UNIT/DOT,
altitude and acceleration arithmetic, CDU rotation, measured acceleration,
force clamps, throttle, horizontal-speed display and live scheduling/recovery.

## Remaining design questions

- Which interpreter resume state requires a separate native representation?
- Which CIA/task source rate and callback safe-point budget justify a future
  request-buffer capacity beyond the finite raster harness?
- Which phase/restart groups are indispensable for one selected flight routine?
- Which numerical operations and fixed-point scales does the first guidance slice
  require, and can overlays fit stock C64 memory without losing native contexts?
- Should upstream transcription build errors be repaired in a separate project?
  This branch deliberately retains the historical sources unchanged.

## Verified native VAD/VSU

Reusable vector arithmetic now matches 13,600 original-assembly checkpoints.
128 complete states pass host, sanitizer and C64 diagnostics. See [VECTOR.md](VECTOR.md).

## Verified SPVAC and CHANG2

Staged VAC admission and interpretive context exchange match 768 complete
original-assembly checkpoints. Host/sanitizer tests and all 768 C64 state
fingerprints pass. Native mixed-context execution is a separate integration
stage. See [EXECUTIVE_SERVICES.md](EXECUTIVE_SERVICES.md).

## Translate native single and double phase-table changes

All 49,152 complete original-assembly states match; 128 host/sanitizer/C64 states pass. See [PHASE.md](PHASE.md).

## Translate arbitrary-factor vector scaling and unrounded DP products

All 8,192 complete assembly checkpoints and 128 host/sanitizer/C64 states pass. See [PRODUCT.md](PRODUCT.md).

## Translate scalar and vector shifts with original rounding and scratch effects

All 26,880 complete assembly states and 168 host/sanitizer/C64 states pass. See [SHIFT.md](SHIFT.md).

## Translate general double-precision DDV and backward division

All 163,840 original-assembly comparisons and 573 host/sanitizer/C64 states pass using the hardware DV model. See [GENERAL_DIVIDE.md](GENERAL_DIVIDE.md).

## Translate complete P66 updated-velocity calculation through STOVL and position load

All 4,648 complete assembly checkpoints pass; all 156 C64 states pass across two RAM-safe images. See [VELOCITY.md](VELOCITY.md).

## Translate scalar square root and normalized root with source error boundary

All 132,096 assembly comparisons and 637 host/sanitizer/C64 states pass; negative inputs stop at the declared pre-1302 boundary. See [NORM.md](NORM.md).

## Translate abort flags and selected restart-group cleanup paths

All 49,152 complete assembly comparisons and 128 host/sanitizer/C64 states pass. See [RECOVERY_CLEANUP.md](RECOVERY_CLEANUP.md).

## Translate unrounded triple-precision dot product and overflow accumulation

All 10,272 complete assembly comparisons and 96 host/sanitizer/C64 states pass. See [DOT.md](DOT.md).

## Translate scalar sums and backward vector subtraction with exact overflow effects

All 27,200 complete assembly comparisons and 256 host/sanitizer/C64 states pass. See [SUM.md](SUM.md).

## Execute native basic and interpretive continuations through original context exchanges

Native C callbacks now enter and exit interpretation, yield at completed operation pairs, restore interpreter payloads, and resume through the original Executive lifecycle. All 140 full original-assembly checkpoints agree; host and sanitizer tests and the PAL C64 diagnostic pass, including masked repeats and an actual raster IRQ admission. This milestone supplies scheduling plumbing; remaining guidance bodies still need translation and binding. See [INTERPRETER_NATIVE.md](INTERPRETER_NATIVE.md).

## Translate ENEMA through restart validation and source-defined exit boundaries

The native restart entry composes the verified cleanup and phase validation routines through the original MMDSPLAY, PTBAD, and fresh-start request boundaries. All 4,096 full 457-word original-assembly comparisons agree, and 128 complete C64 target states pass with host and sanitizer verification. Subsequent display and restart execution remain to be completed. See [ENEMA.md](ENEMA.md).

## Translate Comanche Executive admission and release with seven native core sets

A separate Comanche 055 Executive retains its seven core sets, five VACs, and source BAILOUT boundaries. All 1,536 source-derived assembly comparisons agree; host and sanitizer tests and the C64 run pass, including 40 complete-state samples. The generated fixture documents five indentation repairs and matches the pinned corrected full rope; historical repository files remain unchanged. CM dispatch and lifecycle are next. See [COMANCHE_EXECUTIVE.md](COMANCHE_EXECUTIVE.md).

## Translate absolute value, vector length and half-unit normalization with signed-zero fidelity

ABS, ABVAL and UNIT now preserve raw length, half-unit vector, overflow, and scratch effects. All 50,736 complete 104-word assembly comparisons agree, including sixteen signed-zero regressions found by P66 composition. Host and sanitizer tests and all 118 full-state C64 checks pass. Full application binding and remaining flight routines are still in progress. See [UNIT.md](UNIT.md).

## Compose native P66 radial velocity and predicted altitude from verified arithmetic

The P66 radial group now computes half-unit radius, radial velocity and predicted altitude with original VAC, signed-zero and scratch effects. All 4,952 complete 134-word original-assembly checkpoints agree; full-prefix host and sanitizer checks pass. Six C64 images verify 203 complete stage outputs from exact original pre-UNIT states. The full-prefix C64 diagnostic exceeds RAM, so whole-program overlay integration remains required. See [RADIAL.md](RADIAL.md).

## Translate P66 desired radial acceleration and gravity through the rotation boundary

The native P66 acceleration group preserves original VAC, scalar division, vector magnitude, gravity scaling and implicit stack pop effects through DANZIG before CDU rotation. All 16,384 full-stage assembly states and the 7,428-checkpoint composed host prefix agree. Strict host, sanitizer and 96 full-state C64 stage checks pass; whole-program overlay execution remains in progress. See [ACCELERATION.md](ACCELERATION.md).

## Execute bounded native disk overlays with canonical erasable storage on the C64

A resident Executive and loader execute separately linked native C payloads from a D64 image, swap and reload them after ordinary C returns, reset transient BSS, and preserve the 2,048-word canonical source image and platform mappings. Host bounds and error checks, sanitizers and the stock PAL C64 disk-loading proof pass. The demonstrated payloads test the ABI; full mission module allocation, media handling and guidance integration remain unfinished. See [OVERLAYS.md](OVERLAYS.md).

## Fix source-exact WAITLIST positive-zero control flow and final-slot insertion

WAITLIST now skips interval additions on early CCS positive zero and follows the final positive-zero insertion route instead of raising a false full-queue alarm. The 256-case full-word regression detects 688 previous field errors and passes after the fix; the original 196-case suite remains unchanged and passes. Host, sanitizer and all 256 C64 states agree with original assembly. See [WAITLIST_ZERO.md](WAITLIST_ZERO.md).

## Execute Comanche basic and interpretive jobs through source-exact lifecycle transitions

The separate seven-core CM runtime now dispatches native C callbacks, exchanges basic and interpreted contexts, sleeps and wakes jobs, and changes priorities through original Comanche semantics. All 876 complete source-derived checkpoints and twenty real native sequences agree; host and sanitizer checks and four stock C64 diagnostics pass. IRQ coordination remains caller-owned pending the dedicated adapter. See [COMANCHE_DISPATCH.md](COMANCHE_DISPATCH.md).

## Translate P66 operator counts, task admission and phase-timer entry paths

DESCBITS, RODTASK and P66VERTA now follow original operator arithmetic, priority-22 admission, phase termination, delayed task scheduling and immediate RODCOMP entry with exact shared scratch. All 73,728 original-entry checkpoints agree; host, sanitizer and 128 full-state C64 checks pass. STRTP66 bias initialization, display admission and later continuations are separate ongoing milestones. See [P66_STARTUP.md](P66_STARTUP.md).

## Port exact native trigonometric services

All 65,536 complete source states match CDULOGIC, SIN, COS and CDUTRIGS; all-state sanitizers and 128 frozen C64 cases pass. The native fixed-point coefficients and overflow behavior retain the original instruction semantics. See [TRIG.md](TRIG.md).

## Run live P66 continuations from native disk overlays

The canonical 2,048-word image drives actual compiled arithmetic callbacks through the mixed Executive. All 5,887 source checkpoints pass; the C64 disk proof passes 72 full-state fingerprints including two real P66 jobs, both IRQ states and SUPERBNK forms. The current driver ends before UNIT and retains its core/VAC for further translation. See [P66_NATIVE_DRIVER.md](P66_NATIVE_DRIVER.md).

## Port original native axis rotations

All 32,768 complete AX*SR*T source states match; strict and all-state sanitizer checks pass. C64 verification compares 128 full frozen states with source scratch aliases and both rotation directions preserved. See [ROTATION.md](ROTATION.md).

## Port live CDU projection and acceleration feedback

Live native trig and rotation feed DOT14 and AFC with original source scheduling boundaries. All 39,432 direct and full-prefix checkpoints match; entire-corpus sanitizers and all 144 frozen C64 checkpoints pass. See [PROJECTION.md](PROJECTION.md).

## Implement exact native hardware divide recurrence

The independently authored fixed-point recurrence matches 327,680 isolated original hardware-DV instruction states, including raw register overflow and signed zero. Full-state sanitizers and all 255 frozen C64 register-pair checks pass. See [HARDWARE-DIVIDE.md](HARDWARE-DIVIDE.md).

## Port source STOPRATE and FASTCHNG initialization services

All 1,024 complete source checkpoints match including return-register debris, physical banks and IRQ state. Strict and sanitizer checks and 128 complete frozen C64 states pass. See [P66_INITIALIZATION.md](P66_INITIALIZATION.md).

## Port original force feedback and throttle clamps

All 16,384 complete force-feedback states match unchanged assembly with all three clamp paths covered. Full-state sanitizers and 64 complete frozen C64 cases pass. See [THROTTLE_FEEDBACK.md](THROTTLE_FEEDBACK.md).

## Port complete bounded native THROTTLE service

THROTTLE+3 through THDUMP matches all 16,384 complete 147-word source states, including hardware-DV weighting, force saturation, critical branches, FASTCHNG and pulse commands. Strict and all-state sanitizer checks and 96 complete C64 states pass; the enclosing return bridge is a separate continuation. See [THROTTLE-SERVICE.md](THROTTLE-SERVICE.md).

## Port canonical whole-image P66 bias initialization

All 1,024 original STRTP66A entry cases match the complete 2,048-word erasable image plus physical SUPERBNK/IRQ. Strict and sanitizer ownership checks and 64 complete whole-image C64 states pass. See [P66_BIAS.md](P66_BIAS.md).

## Port canonical vertical-guidance startup through RODCOMP

All 1,024 complete original source images match through STOPRATE, flag updates, phase termination, timer admission and POSTJUMP. Strict and sanitizer checks and 64 whole-image C64 cases pass, including source resource bailouts. See [P66_VERTICAL_START.md](P66_VERTICAL_START.md).

## Port Comanche timer queue and native task execution

All 306 complete original WAITLIST checkpoints match, including zero-sign branches, source task transfers and 01203/01204 resource alarms. Strict, sanitizer and C64 checks cover all checkpoints, ten actual native callback sequences and ownership guards. See [COMANCHE_WAITLIST.md](COMANCHE_WAITLIST.md).

## Connect Comanche Executive to real native C64 interrupts

Exact source admission and callback states verify the CM runtime adaptation. C64 checks pass 512 admission fingerprints, seven real raster interrupts and 15 full callback states while preserving native registers, compiler storage, caller IRQ state and KERNAL coexistence. See [COMANCHE_IRQ.md](COMANCHE_IRQ.md).

## Port real P66 major-mode display job admission

NEWMODEX matches 1,024 complete original erasable images, admitting the actual DSPMMJB basic job. Strict, sanitizer and C64 checks pass 64 full images; deferred display formatting and consumption remain separate continuations. See [P66_MAJOR_MODE.md](P66_MAJOR_MODE.md).

## Compose complete native STARTP66 through RODCOMP

The complete source landing-start sequence matches 1,024 whole-memory entry states, with actual display-job and timer admission and both resource bailouts. Strict and sanitizer checks and 32 complete C64 images pass. Deferred display execution and the remaining landing cycle are separately tracked. See [P66_LAUNCH.md](P66_LAUNCH.md).

## Port complete native horizontal velocity arithmetic

All 65,536 original horizontal-velocity checkpoints match, preserving UNIT overflow continuations and physical bank effects. Strict and all-state sanitizer checks and all 256 complete frozen C64 states pass. See [HORIZONTAL.md](HORIZONTAL.md).
