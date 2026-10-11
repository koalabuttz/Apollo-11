# Actual Comanche POODOO recovery

`cm_abort_recovery_stage` continues actual POODOO/CCSHOLE after BORTENT at05732.
Phase0 tests V37FLBIT040 in FLAGWRD7: the average-G branch selects actual
WHIMPER-1 at07766. Otherwise it enters actual DOWNFLAG05560. Phases1..3 call
that generic source-qualified service with original STATEFLG067, REINTFLG0236
and NODOFLAG054 descriptors. Phases4..6 execute real BANKCALL to MR.KLEAN05,2501,
its phase cleanup, real SWRETURN and TC WHIMPER. The result is the actual
WHIMPER07767 entry. Each stage returns before a later overlay is loaded.

The canonical2048-word image is the only persistent flight state. The generic
flag service receives a local SUPER/IRQ view; the other eleven CM I/O fields
remain unchanged. Every compiled transfer uses the genuine source address and
actual saved BUF2 pair. No source words are decoded or executed by native C.
The selected WHIMPER and ENEMA bodies are separate released source-qualified
groups; this packet preserves their actual entry rather than inventing a
completed recovery or selected application callback.

4096 uninterrupted source sequences cover both flag branches, all erasable
banks, qualified SUPER/raw-register contexts, and original phase pairs. The
cleanup branch has seven complete checkpoints; the average-G branch has one.
All16384 complete2048-word+13-I/O states match the fresh original-layout
`6e3150ae...` rope. Stream SHA256 is
`00626b6308bc79fc2c29f7fc5bced43e7e6fc821f82f76cfe6e544b18cf3ba9b`.
Strict host and full ASAN/UBSAN runs retain that same stream. Caller phase
selection, coherent banks/ZERO and the existing generic service/cleanup
ordinary compiled-caller domains are explicit contracts. This packet does not
certify an entire alarm, Executive or restart file.

Stock PAL C64 executes16 representative source sequences with64 complete-state
checkpoints, two whole2061-field samples and an external final2048-word compare.
Both incoming hardware I values occur with both source flag branches, and ROM
mapping/I preservation is checked. Mailbox `a500004000` is PASS; the software
stack extent is1120 bytes including diagnostics and disk transport. Native220
loads5440 bytes, input254 loads9505, and checker255 loads10140; all fit the
single overlay slot and add no allocated zero-page storage.

The first target run completed all64 source checks with zero fixture mismatch
and an exact final original image, but reported eight harness failures: entry
used `(run>>1)&1` to choose incoming I while a stale assertion expected `run&1`.
The exact failed resident/native artifacts and captures are retained in
`evidence/cm-abort-recovery-harness-failure/`. Correcting only that assertion
produced PASS; native220 remained byte-identical and source/SAN streams remained
unchanged.

Run `make -f Makefile -f tools/cm_abort_recovery.mk test-cm-abort-recovery`
and `sanitize-cm-abort-recovery`; target replay uses
`cm_freeze_abort_recovery.py`, `cm_abort_recovery_build.py` and
`cm_abort_recovery_vice_test.py`. Native MIT code uses no AGC decoder; the GPL
original engine is restricted to the host oracle. Released historical modules
and artifacts remain unchanged.
