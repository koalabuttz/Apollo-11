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

## Next bounded milestone: sleep, wake and priority changes

Translate JOBSLEEP/JOBWAKE and PRIOCHNG, including dormant-priority encoding,
wake-address matching, packed low-bit preservation, scan/tie behavior and native
continuation outcomes. Extend independent original-assembly comparisons and
native C64 sequences before integrating timer tasks.

Follow with WAITLIST task scheduling and CIA timer
mapping; BAILOUT1 and protected restart groups; required interpreter/multiword
semantics; then one bounded landing-guidance computation. Flight feasibility
remains a separate measurement milestone. Rust is optional for later components.

## Remaining design questions

- Which interpreter resume state requires a separate native representation?
- Which CIA/task source rate and callback safe-point budget justify a future
  request-buffer capacity beyond the finite raster harness?
- Which phase/restart groups are indispensable for one selected flight routine?
- Which numerical operations and fixed-point scales does the first guidance slice
  require, and can overlays fit stock C64 memory without losing native contexts?
- Should upstream transcription build errors be repaired in a separate project?
  This branch deliberately retains the historical sources unchanged.
