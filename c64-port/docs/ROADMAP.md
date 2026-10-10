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

## Next bounded milestone: platform exclusion and interrupt request delivery

1. Define C64-owned interrupt exclusion wrappers that restore the caller's
   previous interrupt-enable state. Verify LLVM-MOS imaginary-register and
   hardware/software-stack preservation, including interrupted C callbacks.
2. Admit requests from a bounded IRQ source at explicitly documented safe points.
   Preserve the distinction between admission and cooperative basic yield.
3. Measure maximum observed admission/completion/exchange costs over the admitted
   domain, IRQ entry/exit and request latency, rather than extrapolating the four
   current timing samples into a flight deadline claim.

Do not accept interrupt-origin requests through the present single-threaded API.
Follow with JOBSLEEP/JOBWAKE and PRIOCHNG; WAITLIST task scheduling and CIA timer
mapping; BAILOUT1 and protected restart groups; required interpreter/multiword
semantics; then one bounded landing-guidance computation. Flight feasibility
remains a separate measurement milestone. Rust is optional for later components.

## Remaining design questions

- Which interpreter resume state requires a separate native representation?
- Which IRQ source and request-buffer capacity are justified for the next bounded
  test, and how should resource exhaustion interact with the pending bailout?
- Which phase/restart groups are indispensable for one selected flight routine?
- Which numerical operations and fixed-point scales does the first guidance slice
  require, and can overlays fit stock C64 memory without losing native contexts?
- Should upstream transcription build errors be repaired in a separate project?
  This branch deliberately retains the historical sources unchanged.
