# Comanche abort snapshot and alarm chronology

`cm_abort_snapshot_stage` follows the actual BAILOUT (`07755`), POODOO
(`05721`), and CCSHOLE (`05747`) source entries through five explicit native
stages. It records the original caller in ALMCADR, executes the actual BANKCALL
to VAC5STOR, runs the complete diagnostic snapshot/GENTRAN body, returns
through actual SWRETURN, produces the genuine alarm word, and executes BORTENT
recording. Each native frame returns before a disk overlay swap.

The abort snapshot is ordinary data in the sole canonical 2,048-word image.
The actual caller's alarm word is source-qualified passive data supplied after
snapshot return; CCSHOLE uses the source constant `01103`. No saved descriptor
or BRUPT word is decoded as an instruction. The final continuation is genuine
BAILOUT+9 (`07766`, before INHINT/WHIMPER) or POODOO+9 (`05732`, before its
average-G/recovery branch). These later bodies remain separate native source
groups. The caller owns producer exclusion throughout the composition.

All 12,288 uninterrupted original sequences and 61,440 complete image/I/O
checkpoints match strict native and full ASAN/UBSAN execution, using the original
rope `6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The lossless stream SHA256 is
`c6707b3c620653ade83f61eb66e7b7bb8e70fc6b7d5febf19047f570c5cb9dfe`.
The source producers use genuine Executive alarm words `01201/01202` at
`02642/02724` with actual failure bank `02000`, all three abort entries and
4,096 randomized canonical snapshot/phase/FAILREG/bank contexts. Other caller
alarm producers require their own data qualification.

The corrected stock PAL C64 diagnostic loads native control 216 and proof-only
input/checker 254/255 through the checked D64 loader and replays 18 uninterrupted
sequences, 90 complete-state boundaries, two direct complete image/I/O samples
and the final full image. Both incoming C64 interrupt states are represented.
It passes with mailbox `a500005a00` and measured software stack extent 719 bytes.

The first diagnostic trial accidentally retained INITDSP's 2/3-phase loop and
used an incorrect low-byte fingerprint mask. Exact failed source/PRG/map/D64
and capture/log artifacts are retained under
`evidence/cm-abort-snapshot-harness-failure*`. Its captured actual phase-1 full
image matched the original; no success verdict was claimed. Correcting only
the resident diagnostic/checker produced the full 90-boundary pass. Native
control payload bytes and original host/sanitizer proofs remained unchanged.

Run `make -f Makefile -f tools/cm_abort_snapshot.mk test-cm-abort-snapshot`
and `sanitize-cm-abort-snapshot`. This qualifies the stated continuous abort
snapshot/record path, rather than the entire abort file or final recovery.
