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

## Next bounded milestone: DSU PIPTIME

Trace elapsed PIPA time subtraction from the loaded timestamp. Translate the
DP subtract with signed-zero, carry, scratch and register behavior; verify the
boundary before STORE 30D against the unchanged original instructions, while
retaining all saved vector/VAC words. Extend to storage and later division only
after that bounded arithmetic contract is established. Live job integration
still needs explicit exclusion and measured scheduling budgets.

## Remaining design questions

- Which interpreter resume state requires a separate native representation?
- Which CIA/task source rate and callback safe-point budget justify a future
  request-buffer capacity beyond the finite raster harness?
- Which phase/restart groups are indispensable for one selected flight routine?
- Which numerical operations and fixed-point scales does the first guidance slice
  require, and can overlays fit stock C64 memory without losing native contexts?
- Should upstream transcription build errors be repaired in a separate project?
  This branch deliberately retains the historical sources unchanged.
