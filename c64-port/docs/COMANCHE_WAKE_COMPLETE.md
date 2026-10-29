# Comanche canonical complete JOBWAKE body

`cm_wake_complete` implements original JOBWAKE05201, its seven-slot scan,
WAKETEST, core0 SPECTEST and SETLOC through the actual ordinary caller return
or authentic exceptional transfer. It owns only the canonical2048-word image
and explicit13-latch I/O record; output transfer reports the actual source
destination after the C frame returns. No instruction word is decoded and
no selected job or arbitrary register-code body is executed.

The immutable older canonical wrapper requires positive core0. This additive
body also handles a genuinely sleeping core0 while idle: JOBSLEEP retains its
dormant priority/CADR/bank, DUMMYJOB publishes NEWJOB-0, and JOBWAKE matches
that context. The actual SPECTEST path awakens core0, installs its resumed
source pair and publishes NEWJOB+0 for subsequent idle adoption. Positive
NEWJOB follows the actual SETLOC comparison with the queued candidate. Source
FIXLOC/OVFIND remain untouched; no job is exchanged by JOBWAKE itself.

The source's +0 priority path is a distinct defensive fault frontier. It
executes COREINC's numeric00014 as `TC014`, producing Q03055 and source PC014.
The helper preserves that exact partial state and returns its tuple; it does
not interpret the word stored at014. This does not claim normal admission
publishes a zero priority or that arbitrary hardware register-code execution
is part of the compiled application contract. A matched dormant core0 with
NEWJOB+0 instead follows actual SPECTEST to CCSHOLE05747/Q02677. The separately
translated alarm/recovery body is a real continuation, not executed here.

Ordinary completion restores the caller's actual FBANK and PC, leaves IRQ0,
and retains true EXECTEM/LOCCTR, NEWPRIO/NEWLOC, awakened priority, saved-bank
and L debris. No-match is a completed source return with LOCCTR-1 and status
EXEC_NOT_FOUND. ±0 sleeping CADRs compare equal exactly as source arithmetic
does. Unsupported preflight inputs leave image, I/O and transfer unchanged.
The caller owns distinct objects and producer exclusion; normal CADR A,
compiled Q0100..07777, coherent hardware banks/ZERO, legal IRQ/SUPER and typed
NEWJOB -0/0/12..72 are required. A matched sleeping bank uses bits0167.

All32,768 independent original complete2048+13-field states match native,
including allseven matching positions/no-match, all31 priorities, raw L,
bank/SUPER/IRQ variations, ±0 CADRs, actual core0 flag branches and +0 priority
frontiers. The full stream SHA is
`4c6d09ba01f808bee93be9c47b6036d6a95936f798538462cd95199af502e825`.
Strict C99 with conversion warnings and the full ASAN/UBSAN stream pass.
Both source ISR contexts are asserted unchanged in the original oracle.

Stock PAL C64 executes64 frozen cases, preserving incoming physical I/ROM
around each real returned disk-overlay call. It retains complete-state
fingerprints, two direct2061-field samples and a direct final2048-word capture.
Mailbox `a500004000` passes; measured software stack74 bytes. The native
module245 is3305 loaded bytes, input2542759 and checker25510145. Diagnostic IDs
are local to this test disk and reserve no final mission registry entries.

Reproduce with `tools/cm_wake_complete_differential.py`,
`tools/cm_wake_complete_sanitize.py`, `tools/cm_freeze_wake_complete.py`,
`tools/cm_wake_complete_build.py` and `tools/cm_wake_complete_vice_test.py`.
The byte-original Executive and6e3150 rope remain unchanged. The GPL engine
is host-oracle only; native C is MIT. Continuous sleep/interrupt/WAKER/idle
adoption and all selected flight bodies are separate application acceptance.
