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

## Next bounded milestone: alarm/abort and protected restart behavior

Trace BAILOUT1/POODOO1 and the protected restart tables against unchanged source.
Choose one bounded abort/reset path with explicit phase/device inputs, preserving
alarm records and retained resources before recovery. Extend the original-assembly
oracle before substituting native reset/re-entry actions. Resource alarm detection
is already verified; the alarm body and restart routing remain untranslated.

Follow with required interpreter/multiword semantics and one bounded
landing-guidance computation. Flight feasibility remains a separate measurement
milestone. Rust is optional for later components.

## Remaining design questions

- Which interpreter resume state requires a separate native representation?
- Which CIA/task source rate and callback safe-point budget justify a future
  request-buffer capacity beyond the finite raster harness?
- Which phase/restart groups are indispensable for one selected flight routine?
- Which numerical operations and fixed-point scales does the first guidance slice
  require, and can overlays fit stock C64 memory without losing native contexts?
- Should upstream transcription build errors be repaired in a separate project?
  This branch deliberately retains the historical sources unchanged.
