# Native IMU stall and pulse-activation stages

`imu_mode_transfer` translates 37 original local Luminary words and 36 Comanche
words from `IMU_MODE_SWITCHING_ROUTINES.agc`. The new groups are COARSERR,
SENDPULS/CA+ECE, AOTSTALL/RADSTALL/IMUSTALL/STALL and the MG2/MODEGOOD/MODESLP/
MODABORT continuations, including their two local passive data words. It adds
local source coverage; the containing files and mission applications remain
incomplete.

The canonical 2048-word image and IRQ/SUPER/channel11/12/14 latches remain
source-owned. Inputs require ordinary 15-bit RAM and raw 16-bit A/L, coherent
bank07/EB3, fixed Q02000..07777, ordinary channels, valid IRQ/SUPER, quiescent
channel14 gyro/coarse activity and excluded producers. SEND additionally
requires +zero CDU command counters050..052. The original host source backend
can immediately normalize negative-zero counters after activating CDU output;
that device action is outside this isolated basic-stage contract. Active command
transport requires its separately bound device owner.

| Stage | LM entry | CM entry | Actual selected boundary |
| --- | --- | --- | --- |
| SEND | 03133 | 03016 | VARDELAY05224/05272, A074 |
| ENABLE | 03140 | 03023 | TASKOVER05261/05324 |
| COARSE_ERROR | 03115 | 03000 | IMUBAD03637/03522 |
| AOT | 03712 | 03573 | STALL outcomes for device1 |
| RADAR | 03714 | 03575 | STALL outcomes for device2 |
| IMU | 03716 | 03577 | STALL outcomes for device0 |
| STALL | 03717 | 03600 | STALL outcomes for explicit A device0..3 |
| FAILED | 03726 | 03607 | MODEEXIT02755/02640 |
| GOOD | 03731 | 03612 | MODEEXIT or BAILOUT1/POODOO |
| SLEEP | 03735 | 03616 | JOBSLEEP05133/05175 |
| ABORT | 03741 | 03622 | BAILOUT1 05716/POODOO05721 |

The stalling stages preserve the source four-way CCS classification. Positive
MODECADR aborts; +zero enters sleep; negative nonzero enters MODEGOOD; negative
zero completes unsuccessfully. MODEGOOD's second CCS admits exactly diminished
A0 (the original -1 sentinel), increments BUF2 and clears MODECADR. Other positive
diminished values abort. Failed completion clears MODECADR and keeps the return
address. AOT's TC STALL changes Q; RADAR's TCF and IMU's CAF route retain the
incoming Q until an actual later call changes it.

Direct FAILED/GOOD/SLEEP entries require RUPTREG2 device0..3. FAILED requires A0;
GOOD accepts the nonnegative diminished result of the initial CCS. The saved
BUF2 caller must be fixed02000..07777 and its saved bank aligned with low10 bits
clear. These bounds make source MAKECADR's low10-mask plus saved-bank addition
nonoverflowing. Its actual TC Q leaves Q3. The native function compiles those
four source helper operations explicitly, without instruction decoding.
Luminary's MODABORT exchanges A/L with BUF2 before TC BAILOUT1; Comanche's
MODABORT has no exchange and calls POODOO directly. Both source paths remain
separate in the native code and full-state comparisons.

SEND sets channel14's three coarse-drive bits070000, loads600MS=074 and follows
the literal COARS2-1 TC VARDELAY with its actual continuation Q. ENABLE sets
channel12 bit6 and retains the full WOR result in A, then selects the literal TC
TASKOVER frontier. The API does not run the delay, sleep, alarm, task return,
IMUBAD or MODEEXIT callee, and it does not consume command counters. Internal
native returns retain source ownership and add no Executive check.

`imu_transfer_literal` exposes original 600MS and abort1210 using physical fixed
word addresses. Every 65,536 accessor argument per mission matches the original
word map; invalid mission/null output and unowned addresses leave the output
unchanged. The actual C64 diagnostics check both values independently of the
numerical snapshots.

Original fixtures/declarations and the disclosed indentation-only CM repairs
remain pinned in `evidence/imu-pulse-source-audit.json`. The host-only independent
GPL source engine executes unchanged original words and is absent from all
native target binaries. No native expected values are injected into that oracle.

The constructor cohort contains 65,536 complete states per mission/stage,
1,441,792 total. Each 2056-word row retains identity, IRQ/SUPER, channels11/12/14,
status and every 2048 source RAM words. Original, strict native and full ASAN/UBSAN
streams match byte for byte. The factory varies MODECADR/CCS examples, diminished
GOOD inputs, raw A/L, source latches and nonzero unowned-word patterns. It is not
an exhaustive Cartesian or all-caller-address claim. One deterministic XZ per
original stream is retained; native and sanitized streams compare directly to
its decoded bytes.

Twenty-two stock PAL C64 diagnostics check 704 complete source-frozen states,
264 immutable guard calls and 44 literal values. Actual PRGs, maps, mailbox and
measured hardware/software stack dumps are retained. Compiler ZP allocation is
zero; all allocated sections end below C000. Reproduction uses the isolated
`tools/imu_mode_transfer.mk` all/test/sanitize/check-host and mission/stage
vice-test targets. Regeneration creates separate build archives; canonical
retained evidence is not overwritten.

The additive ledger leaves115 local words open in each containing file, chiefly
GOMANUR, R02 and standby scaler/time recovery. This packet does not establish
those bodies, asynchronous coarse transport, downstream scheduler/services or
complete mission execution.
