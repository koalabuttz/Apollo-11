# General native double-precision division

`p66_dp_divide` implements DDV/BDDV with arbitrary ordinary DP divisor.
Backward division exchanges the dividend/divisor roles. The arithmetic follows
the source's normalization, TPAGREE, two quotient stages and near-one branch;
it is not a replacement with rounded mathematical division. MPAC+2 is retained
on early exits unless BUFZERO ran; successful division clears it. OVFIND changes
only on overflow. BUF/DVSIGN/DVNORMCT/MAXDVSW and A/L preserve source effects.
MPTEMP and unrelated context words remain intact.

Caller owns distinct state/scratch objects, excludes producers and supplies
ordinary 15-bit MPAC/scratch words with MODE=+0. MODE and operand bits are
validated; unsupported inputs reject without mutation. Ordinary stored-state
words are a caller obligation. The returned path describes original zero,
quotient or overflow exits; it does not execute an alarm or dispatch.

All163,840 unchanged original-assembly comparisons match17 modeled words and
path, with surrounding guards, banks, MODE and pointer checked independently.
Retained native fields are separately checked with whole-object comparisons.
There are481 zero,79,917 quotient and83,442 overflow cases;8,555 quotients use
the near-one flag. Snapshot SHA256:
`43f40a751558ac2f30deb014d24925940f91c449c5f0e1c3fa74f043ac8c9c42`.
The oracle selects the pinned hardware DV model as documented in DIVIDE.md,
without altering historical assembly or the pinned checkout. Native code is
excluded from the GPL reference executable. Independent review found no
blocking defect; malformed operand checks and caller-word requirements were
added from that review.

573 frozen complete states pass host, ASan/UBSan and PAL C64 VICE. Every
unsupported MODE and malformed high/low divisor check preserves all objects.
The full163,840-case sanitizer stream also equals the original-assembly stream.
Mailbox `a500003d02` records zero failures. PRG29,320 bytes: text11,223,
rodata18,083/BSS180/noinit2, image/state ends at $7b3c; no extra compiler ZP.
Measured hardware/software stacks24/135 bytes. No timing deadline is claimed.

Use `make test-dp-divide`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-dp-divide`,
`python3 tools/dp_divide_differential.py deps/fixture`, `make c64-dp-divide`
and `tools/dp_divide_vice_test.py` with standard VICE/ROM arguments.
Retained `evidence/dp-divide-*` includes deterministic gzip snapshots and
sanitizer stream, logs, PRG/map and target evidence. Earlier hashes and
historical sources are unchanged. The native module has no opcode decoder,
heap, floating point or GPL engine linkage.
