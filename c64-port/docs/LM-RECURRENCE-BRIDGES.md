# Canonical task interrupt and idle transfer

Three compiled source bridges operate on the sole 2048-word erasable image.
`lm_waitlist_dispatch` starts at literal T3 vector `04014`, or `TASKOVER`
`05261` with `RUPTAGN=+1`, and stops at the actual selected basic task entry.
It retains saved A/L/Q, bank/SUPER words, all WAITLIST scratch, list shift,
TIME3, and source register debris. `lm_taskover_resume` implements the literal
`TASKOVER` route with `RUPTAGN=-0`, through architectural `RESUME` (including
RELINT), stopping before the saved interrupted instruction executes.
`lm_idle_start` implements `ADVAN` `03214` through `NUDIRECT` and `SUPDXCHZ`
to an already admitted positive-priority basic job. Its returned address must
be used before source channel11/activity scratch overwrites LOC.

These bridges perform the original bounded operations directly in C. They do
not decode source instructions. Saved BRUPT remains raw source data. Selected
fixture addresses other than actual RODTASK/RODCOMP are entry observers;
their program bodies are outside this proof. No native IRQ ticket is consumed
and no scheduler check is inserted at a bridge return. Producer exclusion and
hardware interrupt admission belong to the caller.

The idle domain is the literal positive-core0 admission and NEWJOB=+0 route.
The task-return domain requires an already captured architectural interrupt
frame. The dispatch domain includes both signs of TIME3 arithmetic overflow;
source negative overflow stores `RUPTAGN=-1`, although the existing typed
WAITLIST primitive covers ordinary positive-overflow timing. The canonical
adapter preserves the exact source word without changing that typed primitive.
No claim is made here for subsequent `RUPTAGN=-1` execution or SVCT3.

Each bridge matches 8,192 independent original assembly transitions. Comparison
retains every erasable word plus SUPER, source IRQ enable, channel11/channel14,
and (for task bridges) architectural InIsr. All 24,576 original states pass
strict native and full ASAN/UBSAN comparisons. Thirty-eight invalid domain
checks and ten null interfaces prove unchanged rejection.

Two isolated stock-ROM PAL C64 D81 diagnostics compare 128 complete frozen
source states. Each uses 32 input/check fixture modules with isolated IDs
160..191; these are test-only IDs and do not allocate production registry
entries. Both mailbox results are `a50000400002`. The idle/RESUME image uses
50 software stack bytes and 33 hardware stack bytes; the dispatch image uses
134 and 29. Extra zero-page storage is zero. Original-source snapshots, target
images, maps, loader manifests, stock-ROM traces, and dependency hashes are
retained in `evidence/lm-recurrence-bridges-proof.json`.

This component closes real CPU/register seams needed for queued recurrence.
The complete hardware-clock → queued RODTASK → admitted RODCOMP composition is
qualified separately; one scheduled RODTASK does not imply periodic recurrence.
