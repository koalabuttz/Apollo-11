# P66 source-owned pending-job exchange

The additive `p66_application_pending` binding runs the retained P66 startup,
display and one-shot recurrence with a real higher-priority deferred mode job.
It preserves the existing application, recurrence, resolver and Executive
source files and their earlier proof identities.

The independent original-assembly factory changes the deferred DSPMMJB core's
priority to `037110` and sets NEWJOB to its core offset `014` at the first
completed RODCOMP pair, LOC `03353`. The native factory introduces exactly the
same input event at that boundary. This is a documented scheduling stimulus;
no retained output word is normalized. Original code exchanges at the first
legal point, LOC `03356`, runs DSPMMJB and its actual ENDOFJOB body, and restores
P66 through INTRSM/NEWOPS, LOC `03357`, with its actual incoming Q (`03206`).
The complete remaining descent and display then run, followed by idle,
100 hardware-counter ticks, T3 delivery, the one-shot RODTASK admission and a
second P66 iteration.

The compiled resolver wraps only known interpretive continuations. A completed
pair returning `EXEC_YIELD` with a positive canonical NEWJOB calls the existing
source-qualified DANZIG/CHANG2 bridge. It returns the existing
`EXEC_EXCHANGED_BASIC` or `EXEC_EXCHANGED_INTERPRETER` action, which adopts the
already selected context without a second lifecycle, admission or IRQ poll.
Basic callbacks retain their existing source-owned scheduling behavior. The
wrapped elapsed call records its operands and incoming Q only while it owns
that call; the existing elapsed return observer supplies the exact DSU/DAS Q
effect. The sole persistent Executive, scratch and VAC image remains at $7000.

Native stages 88 (exchange), 89 (elapsed return), 98 (view import) and 99 (view
export) are C adapters, not original AGC addresses or an instruction decoder.
Stage 97 initializes diagnostic native pointers; stage 245 checks frozen
source checkpoints and introduces the explicit test stimulus. Metadata stage
148 contains the checked 118-entry directory. The other 112 stages retain their
published images, including returned application adapters 155 and 159.

Each loaded C frame returns before another image is loaded. Producer exclusion
spans metadata lookup, checked stage load and invocation; the resident copied
descriptor contains its own bounded filename. Normal source IRQ/SUPER/channel
state is explicit I/O metadata. The host adoption test posts a native queued
request during the return observation and proves that this request remains
queued; it does not claim physical C64 interrupt arrival at that instant.

The checked allocation is:

| Address | Use |
| --- | --- |
| $7000–$7fff | Canonical 2048-word source image |
| $8000–$bfff | Single returned-C-frame module slot |
| $c080 | Diagnostic mailbox |
| $c090–$c09f | Diagnostic returned-frame progress |
| $c100–$c3d3 | Transient Executive/task view lease |
| $c400–$c4c5 | Native runtime, binding and request queue: 198 bytes |
| $c600–$c63f | Copied module descriptor lease: record 27 bytes |
| $c800–$cfff | Software stack |

The production-style diagnostic resident ends at $6fda, leaving 38 bytes before
the canonical image. Target ABI measurements are runtime 36 bytes, binding 130,
queue 32 and Executive view 648. The first oversized resident link failed the
fixed layout gate; its map/log are retained. Moving native initialization and
view adaptation into returned service stages, and specializing the checked
fixed binding, made the final image fit. Loader integrity checks remain intact.

Qualification compares all 2053 raw fields at each checkpoint: all 2048 words
and explicit SUPER, source IRQ, channels 14/11 and source PC. Sixteen original
paths produce 2476 checkpoints. Strict and full ASAN/UBSAN streams match decoded
SHA256 `822b4b18d80e7cc4bf201b25862712bcc0a344d9b59c508e6d26a28a0dc5df48`.
An additional 64 original first-adoption states verify enabled/masked guard
restoration, stale resolver lookup and retained queued requests. Removing the
new scheduling stimulus reproduces all 2460 historical recurrence states,
SHA256 `d1aa5f4de9e02bf126731818227c4bc7592e62bddbbd66a425cc56346683f186`.

The actual PAL C64 stock-ROM/D81 run passes 312 checkpoints across two frozen
inputs (154/158), mailbox `a50000380102`, using the real metadata, scheduler,
arithmetic, basic and source-view stages. Measured software/hardware stack
extents are 801/91 bytes, with no additional zero-page allocation. A separate
directory run passes 236 complete descriptors and unknown/error/stale/recovery
checks with all canonical sentinel words preserved, mailbox `a50000ec0002`,
stack extents 71/41 bytes. Both use the existing 48-billion-cycle cap. The first
directory launcher failed before execution because its private XDG directories
were missing; the corrected launcher passed with unchanged target images.

`p66-pending-port-test` and `p66-pending-port-sanitize` rebuild host executables
in private temporary directories and stream retained original evidence. They
verify every retained module header, SHA, size, BSS and CRC, and do not rebuild
the qualified disks. This milestone qualifies this concrete application path;
generic Executive/Waitlist closure and the remaining landing programs require
their own source qualification.
