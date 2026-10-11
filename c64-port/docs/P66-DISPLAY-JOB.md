# Live P66 MAKEPLAY and display continuation

`p66_display_job_mappings` supplies60 actual basic continuation pairs for
admitted MAKEPLAY02546/020067, its sleep/wake and busy paths, and the original
nonflashing V06N60 three-row consumer. The callback imports/exports the sole
canonical image under producer exclusion. Source PRIOCHNG, JOBWAKE and
JOBSLEEP execute once. `EXEC_DISPATCHED_BASIC` adopts their real selected
pair without repeating a source exchange or pending-job poll. The idle route
returns {0,0}; duplicate and completed display jobs execute actual ENDOFJOB05155
and adopt its source-selected basic/interpreter/idle continuation without a
second lifecycle operation. Alarm01206 preserves actual POODOO05652 and faults
pending recovery rather than claiming that recovery ran.

The binding embeds `p66_descent_binding` at offset zero, so the existing
arithmetic/basic/descent callback user ABI remains valid. It adds explicit
channel11, stage status, and a resident display overlay invoker. Append the
60 mappings to the existing mixed table. Every selected non-display job must
also be mapped. The underlying headers define supported source contexts:
normal literal P66 verb/noun flags, bank10/EB7; source ordinary Executive
slots; Pinball-owned DSPTAB and fresh physical noun inputs. Producer hooks,
image, runtime and binding storage must outlive callbacks. The current job
is basic throughout these mappings; no interpretive poll is invented.

`tools/display_job_differential.py` verifies512 complete real native runtime
jobs against unchanged original Luminary099 instructions at23 successive
boundaries each. All11,776 complete2,051-word states agree, SHA256:
`e581375ef1d84cc4051252bcaa83db25314dca33c56b74304d9d0cf3ac059b3f`.
`tools/display_job_lifecycle_differential.py` adds768 jobs covering initial
sleep through actual idle ADVAN03214, busy NVSUB through actual Pinball and
JOBSLEEP idle, and duplicate-display termination. All5,120 complete states
agree, SHA256:
`17579a6e2eca791ebb58ea491c47adf360978eec5ff8286e384e38fefc13b65e`.
Strict validation covers60 unique mappings and22 unchanged invalid source
entries. Both complete corpora also agree under ASAN/UBSAN. The prior before-END proof identities remain unchanged. An additional512
jobs,24 checkpoints each, execute the actual END body and compare its final
basic DTCB, interpreter NEWOPS or idle ADVAN registers and all flight words.
All12,288 states agree, SHA256:
`bac3af9e32c2978387ab18a7decc408b8d2da72f1d8dd4f0bd9e448346ff515f`.
`p66_display_job_finish` executes END321 directly when another basic callback
has already committed actual source PC05155, its banks, io, and Executive view.
It retains that canonical control rather than replacing it with the older
`runtime.current` tuple. The caller returns its source-selected adoption action
through the existing finish seam. A separate strict and ASAN/UBSAN run compares
all 12,288 final states with a deliberately stale runtime tuple03531/064067;
the same original-source SHA256 agrees. This helper introduces no trampoline
or pending-job poll. External producer-driven wake scheduling remains a
distinct runtime service.

A resident build defines `P66_DISPLAY_JOB_DISPATCH_ONLY` and supplies
`invoke_display`. The descriptor in
`platform/c64/p66_display_job_payload.h` retains resident io/channel pointers;
each stage returns before a slot swap. `P66_DISPLAY_JOB_KERNEL_ONLY` omits
resident callbacks; `P66_DISPLAY_JOB_ONLY_SELECTOR` builds one real source
body. `tools/p66_display_job_overlay_build.py` produces22 native overlays,
selectors300..321 with reserved moduleIDs60..81. All link into the fixed
16KB slot with no private zero page or no-init storage; loaded sizes range
604..5,983 bytes. Each underlying source body has its own real C64 regression
proof. Whole callback/disk-router execution is a separate target integration
proof, not claimed by the link checks. Host recipes are in
`tools/display_job.mk`. No native module decodes AGC instructions or uses an
emulated AGC CPU.
