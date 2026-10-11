# Native scheduler overlays

`exec_overlay_bind` connects the resident adapter to the shared native
scheduler service hooks. The existing `exec_native_step` remains the sole
trampoline. Its preparation guard covers disk loading and the original
locked prepare stage; the loaded stage returns before the guard restores and
the resident mapped continuation runs. After that callback returns, the
original completion guard covers loading and the locked finish stage. No
scheduler overlay stack frame or pointer to its code survives a slot swap.
The frame and all callback pointers/targets are resident.

The prepare and finish bodies are extracted from the shared scheduler,
not copied algorithms. They preserve the exact existing start, IRQ-admission,
pending-job and action seams. `EXEC_DISPATCHED_BASIC` retains its no-poll,
no-second-exchange and no-post-return-IRQ-admission behavior. A loader/ABI
failure returns0xffff, latches OUT_OF_SCOPE and stops the native job without
invoking the callback or lifecycle a second time. Callback state already
committed before a finish-load failure remains committed; pending IRQ queue
entries remain pending. Binding rejects absent restoring exclusion hooks; an empty resident IRQ
queue provides the real lock seam. The public header documents these contracts.

`platform/c64/native_scheduler_payload.c` uses the resident frame pointer
through the existing explicit argument word. `tools/native_scheduler_overlay_build.py`
builds scheduler preparation module150 and three finish families selected by
explicit native action:151 for YIELD/ENTER/EXIT,152 for FINISH/SLEEP/REPRIO,
and153 for already source-selected BASIC/INTERPRETER adoption. Target sizes
are7,464/10,202/12,234/4,206 bytes; all fit the16KB slot. `exec_overlay_module`
selects the native family, without AGC instruction fetch or decode. The bodies
are compile-time subsets of the same shared finish algorithm. Linker assertions
reject private zero page, no-init allocations and an oversized slot.
The resident build defines
`EXEC_NATIVE_RESIDENT_SERVICES`, so it cannot pull in default linked service
bodies and must bind the resident adapter before stepping.

`tools/display_job_scheduler_snapshot.c` runs both complete original-source
MAKEPLAY runtime corpora through bound scheduler hooks under the real nested
exclusion API. All11,776 normal and5,120 sleep/busy/duplicate2,051-word
states remain identical to their original assembly references, including
all-source ASAN/UBSAN. Reference SHA256 values are respectively:
`e581375ef1d84cc4051252bcaa83db25314dca33c56b74304d9d0cf3ac059b3f`
and
`17579a6e2eca791ebb58ea491c47adf360978eec5ff8286e384e38fefc13b65e`.
The additional actual-END runtime corpus also agrees through these services:
12,288 complete states under strict C and ASAN/UBSAN, including actual basic,
interpreter and idle adoption, SHA256:
`bac3af9e32c2978387ab18a7decc408b8d2da72f1d8dd4f0bd9e448346ff515f`.
Six strict/sanitized loader/adoption tests cover enabled and masked callers,
prepare and finish load failures, retained callback output, single invocation,
pending IRQ preservation on DISPATCHED_BASIC, and stopped reentry after a
load failure. Recipes are in `tools/native_scheduler.mk` (include
`tools/display_job.mk` for its complete host source lists).

These checks establish source semantics and target slot linkage. Actual
resident/disk execution of the new service modules is a separate integration
proof; no whole target execution claim follows from a successful link. All
native scheduler bodies operate on typed canonical state, with no AGC decoder
or emulated AGC CPU.
