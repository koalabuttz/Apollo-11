# P66 operator and task-entry services

The native services translate DESCBITS through RESUME entry, RODTASK through
its original priority-22 FINDVAC admission and TASKOVER entry, and P66VERTA
through phase-three termination, TWIDDLE +100 centiseconds, and immediate
RODCOMP entry. RODTASK admits RODCOMP; it does not reschedule itself.
The task's original 2CADR is 03275/064067, retaining SUPERBNK 060.

Caller-owned exclusion and source bank selection are explicit. Phase scratch
061..064 aliases Waitlist scratch; task EXECTEM1/2 alias 061/062. The canonical
image adapter must reconcile these words at each owned boundary. Invalid
physical bank selection rejects unchanged. Resource failures retain their
original BAILOUT1 paths and mutations rather than rolling insertion back.

All 73,728 original-entry checkpoints agree: 65,536 exhaustive operator count
cases, 4,096 task admissions, and 4,096 phase/timer cases. Modeled state sizes
are 2/332/85 words respectively; eight adjacent guards per case remain unchanged.
Snapshot SHA256:
`ed7b3eb1eded2b573eaa8e9cf173e4115ba4da72eb22d5269c5472452fc21272`.
The independent oracle links only source input constructors and the engine.

Strict host and ASan/UBSan and 128 complete-state PAL C64 checks pass;
mailbox `a500008003`. Lossless frozen storage uses 24 whole baselines and 2,063
field replacements. The PRG is 31,765 bytes, with 16,145 text, 15,606 read-only,
670 BSS and 78 static-stack bytes; extra zero page is zero. Measured hardware
and software stack extents are 18/43 bytes.

STARTP66 initialization, interrupted-context restoration, TASKOVER's subsequent
transfer, and the full RODCOMP continuation belong to ongoing services. These
entry-boundary checks do not substitute placeholders for their execution.
`tools/p66_startup.mk` provides host, sanitizer and C64 targets; differential
and VICE tools reproduce the retained evidence.
