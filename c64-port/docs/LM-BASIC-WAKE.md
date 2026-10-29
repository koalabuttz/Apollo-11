# LM basic job wake

`lm_basic_wake` translates JOBWAKE05137 through the actual ordinary basic
caller return. It finds the first matching dormant job, rebuilds its banked
wake address, restores its positive priority and compares it with the pending
Executive candidate. Source not-found returns normally with LOCCTR=-1.
The header defines the supported active-current-core, saved-bank, caller and
pending-job domain. Unsupported inputs leave image and SUPER/IRQ unchanged.

The full canonical image is authoritative. The wrapper uses the native
Executive wake service while retaining exact original scan scratch and raw
register effects, including caller L in DXCH debris. The wake CADR in A must
be sign extended, so source TS does not overflow and skip the return setup.
Source IRQ remains inhibited at the return; the caller owns exclusion.

`tools/lm_wake_differential.py` compares 8,192 complete original Luminary099
2,050-word states. Cases cover both zero wake encodings, all seven dormant
slots, repeated matches, no match, pending priorities, raw caller L and all
initial EB/FB/SUPER values. Snapshot SHA256:
`0afd4d95a2421662a4937ba6af41e4272f0e5fc11b068664b0e5d607adce2fb6`.
Strict tests cover 64 frozen complete images and20 invalid boundaries; all
source cases agree under ASAN/UBSAN. C64 PAL VICE passes all64 images:
mailbox `a500004000`, PRG10,543 bytes, no extra zero page, measured hardware
stack20 bytes (plus14 ROM bytes) and software stack704 bytes. The target
runner allows180 seconds because concurrent VICE diagnostics share CPU.

Standalone recipes are in `tools/lm_wake.mk`. Native code contains no AGC
instruction decoder or emulated CPU. This milestone covers waking from an
active basic caller; sleeping and idle-current-core wrappers are separate.

**Core set 0 (exec-closure).** The domain now also admits core set 0 asleep or free (-0). That is the state a task's JOBWAKE meets while the dummy job runs. A sleeping job matched there takes WAKETEST's `TCF SPECTEST`, then SPECTEST's `TCF SETLOC` or its NEWJOB -0 path, as in the original. `lm-exec-jobs` compares those cases with yaAGC, and the strict test's core-0 -0 boundary is replaced by a +0 core set 3 priority.
