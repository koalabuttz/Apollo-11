# Native continuations for basic and interpretive jobs

The mixed continuation table extends the existing cooperative C trampoline to
source-exact Executive exchanges of interpretive contexts. It executes mapped
native C bodies and uses original AGC addresses as identity and resume metadata.
It does not fetch or decode operation words. Historical original sources remain
unchanged, and no GPL engine code enters the native binary.

`exec_native_context_init` accepts an immutable `exec_context_mapping` table.
Each entry has a positive actual continuation address, complete BBCON, explicit
basic/interpretive kind, and C callback. Both kinds may use the same address;
lookup distinguishes kind and both address words. Unsupported kinds, malformed
BBCON/address values, null callbacks and duplicate complete mappings are
rejected. The old `exec_native_init` and its basic table remain supported.

Admission, including IRQ-buffered admission, targets basic entries. The original
NUDIRECT starts basic code; a job enters interpretation through an explicit
`EXEC_ENTER_INTERPRETER` return. Its supplied resume pair represents INTPRET's
first operation-word address and physical bank. The runtime sets LOC to that
address, BANKSET to the hardware BBANK mask 076007, INTBIT15 to bit 040000, and
EDOP to +0, stopping before NEWOPS. A job that will interpret must reserve its
VAC using FINDVAC admission. Native callbacks can call translated guidance C
kernels and keep their required logical register/global scratch in caller-owned
storage; such scratch has the same lifetime/ownership as its original erasable
symbol. The Executive itself saves the original seven MPAC words and MODE.

An interpretive `EXEC_YIELD` supplies the next complete operation-pair address.
The callback must have consumed its pair, leaving EDOP=+0. This return identifies
the completed LOC as resume minus one. With NEWJOB=+0, the native DANZIG boundary
advances LOC and continues directly. With a pending job, CHANG2 complements the
completed LOC and exchanges contexts through the verified Executive API.
An incoming interpretive context resumes through INTRSM, advances its saved
LOC by one, and recomputes BANKSET, INTBIT15 and EDOP. An incoming basic context
uses its DTCB transfer metadata. The interpreter's SUPERBNK restriction is
preserved: a continuation does not silently change the current superbank.

`EXEC_EXIT_INTERPRETER` is explicit. Its resume identifies the basic word after
EXIT. The runtime retains the EXIT word's LOC, restores the bank metadata and
changes callback kind, matching original EXIT's TC LOC+1. Interpretation does
not invent a direct sleep/completion operation. A mapped basic continuation
then returns `EXEC_SLEEP`, `EXEC_FINISH` or `EXEC_REPRIORITIZE` using the existing
source lifecycle contracts. Thus a sleeping interpretive job wakes at a basic
CADR and can explicitly re-enter INTPRET. Its MPAC/MODE/VAC ownership survives.

The mixed basic dispatch paths also handle incoming interpretive contexts after
CHANG1, ENDOFJOB, JOBSLEEP, PRIOCHNG and idle NUCHANG2. Source priority selection,
resource release, signed PUSHLOC overflow encoding, FIXLOC, NEWJOB and epilogue
LOC/BANKSET debris are retained. Native mappings do not replace the original
packed-priority scans with a new ready queue. Unsupported mappings fail at a
bounded native fault; changes already made by a callback are not rolled back.
Callbacks always return through the C ABI before a context exchange. They must
store persistent job payloads in Executive MPAC/VAC or their declared original
global storage, rather than retain a suspended C stack.

## Verification

The shared fixture constructors supply only inputs to the host-only original
assembly oracle. That oracle links no native Executive, dispatcher or arithmetic
implementation. It executes unchanged INTPRET, DANZIG/CHANG2, CHANG1, INTRSM,
EXIT, ENDOFJOB, JOBSLEEP/JOBWAKE, PRIOCHNG and idle dispatch. It checks actual
transfer addresses and BBANK/SUPERBNK independently. Native callbacks verify
restored payloads, MODE, PUSHLOC, overflow sign, ordinary frame returns and
restoring exclusion, beyond the saved-state comparison.

Thirty-five cases produce 140 complete checkpoints, with all 324 Executive
words, status, actual continuation address/kind, INTBIT15 and EDOP matching.
Seven sequential two-job cases put the second job in each saved slot 1..7.
The low job enters interpretation and yields to a higher-priority basic entry.
The high job enters interpretation, continues without a pending job, exits and
finishes into the low interpretive context. The low job exits, sleeps, wakes,
re-enters interpretation and completes. Additional cases cover each slot for
basic CHANG1, JOBSLEEP, PRIOCHNG and idle NUCHANG2 into interpretive contexts.
These include high fixed banks and nonzero EBANK/SUPERBNK metadata. The snapshot
SHA256 is `5129b28f03344d70358554c53bb9ccc8b341db676f94c5418f059388f1336041`.

Host frozen fingerprint tests and ASan/UBSan pass. Type/mapping and EDOP fault
checks pass; old basic Executive, IRQ, lifecycle and restart suites pass after
the runtime change. Fingerprints supplement the full-word assembly comparison
and retain the usual possibility of hash collisions.

This is native continuation plumbing. The test job bodies exercise retained
payloads and lifecycle boundaries; they do not claim a complete Apollo landing
trajectory or a mission deadline. A full guidance job must map its translated
operation bodies and own all required shared numerical scratch and peripheral
inputs. Peripheral delivery, instruction fetch/decode, an AGC emulator and a
mission-complete flight claim are outside this milestone.

The PAL C64 diagnostic passes all 140 original-derived fingerprints, repeats
16 complete-sequence checkpoints with caller IRQs already masked, and uses a
real raster ISR to publish the higher-priority basic entry. The ISR leaves the
Executive unchanged; foreground poll admits it under exclusion before the low
interpretive callback yields. Callback IRQ status and restoring guards are
checked on target. Mailbox `a500008c100191000000` records zero failures, 140 plus
16 checks and one actual IRQ publication. Hardware/software/isolated stack
sentinel extents are 27/89/16 bytes within their instrumented ranges. The
hardware extent excludes the 14-byte stock ROM BASIC SYS prefix already on the
stack. The PRG is 29,105 file bytes; text is 28,274, read-only data 817, BSS
1,690, and no-init one byte. RAM ends at $804a. There is no extra compiler zero
page beyond the 32 imaginary registers preserved by the IRQ gate. These are
finite diagnostic measurements, not a flight timing or worst-case stack proof.
