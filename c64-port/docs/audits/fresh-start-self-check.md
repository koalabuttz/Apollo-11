# Certificate review: FRESH_START_AND_RESTART (LM, CM) and Comanche055 AGC_BLOCK_TWO_SELF-CHECK

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` a90e77ac
(after Lane A2 PR #124), against the certificates drafted by milestones
`lm-fresh-start-complete`, `cm-fresh-start-complete` and
`cm-self-check-complete`. This is a re-review of the rejections in #126.

Verdict: **accepted** for Luminary099 FRESH_START_AND_RESTART, Comanche055
FRESH_START_AND_RESTART, Comanche055 AGC_BLOCK_TWO_SELF-CHECK and Luminary099
AGC_BLOCK_TWO_SELF_CHECK. The LM self-check entry was accepted after Lane A2
PR #141 (98510065), as described below.

## What was checked

- The combined certificates keep the agreed shape:
  - one record per category;
  - manifests that cite each milestone record by sha256 and count;
  - the `*_complete.py` tools pass `--check`, so the milestone maps union to every word of the file with no overlap;
  - the comparison manifests point at yaAGC streams and proofs, not at maps.
- The cross-bank entries are registered at their original addresses.
- Every item of #126 is fixed:
  1. The LM fresh-start and V37 rows are chosen by outcome, not by stride. They now include the ERESTORE-mismatch fresh start, the no-group-active GOTOPOOH restart and the DOFSTART/MR.KLEAN/STARTSUB path.
  2. The CM word cover is replaced by an outcome cover. It now includes CCS PHASEn −0, BZF GOTOPOOH and the cm-v37 outcomes.
  3. The self-check rows include both CCS NEWJOB outcomes and the other missed outcomes.
  4. Every VICE log's PRG sha256 equals a fresh tip build.
  5. LM `BZF KILL2` with MODREG = P20 is compared. The CM warning-still-on restart loop is modelled on both sides: yaAGC's WarningFilter leaves channel 33 bit 14 clear. All 30 warning cases reach 05,2545, take `BZF NONAVKEY +1`, and equal yaAGC.

## Reproduced

- All test, sanitize, differential and VICE targets give 0 failures. The proofs and cover records are byte-identical to the evidence.
- Independent gcov `-b` over every linked source, comparing the host oracle program against exactly the C64 rows, finds 0 host-only lines and 0 host-only branch outcomes in all 8 builds.
- The only untaken outcomes or refusals that the change introduced are defensive guards in the shared inter-bank code.

## Luminary099 AGC_BLOCK_TWO_SELF_CHECK (after #141)

At a90e77ac, this entry still cited the four pre-#124 records and had no
`c64_branch_outcomes`. The cause was that `tools/self_check_complete.py` named
the file with the listing's spelling, `SELF-CHECK`.

PR #141 adds `REPO_SOURCE` and changes only the `source_path` of the
comparison, source-map and line-map records. No native or test source
changed. The entry now cites the post-#124 records, the
`lm-self-check-complete-c64-cover.json` record and
`vice_targets {vice-self-check: 84}`.

Rechecked at 98510065:
- `cert_precheck` gives 0 FAIL;
- `evidence.py verify` passes;
- `test-self-check` and `sanitize-self-check` give 25068 checks and 0 failures;
- `self-check-differential` passes: 4096 + 43 cases equal yaAGC, and 77 + 7 C64 rows take all 426 + 392 host arcs;
- `self_check_complete.py --check` passes with no record change.

## Notes for the lane (not blocking)

- Add `vice_targets` to each certificate (#127 migration):
  - LM fresh start 259/136/256;
  - CM 259/168/242;
  - CM self-check 85.
- The LM and CM self-check certificates cite the same host and C64 files, because self-check is one milestone. That is acceptable.
- Some earlier notes are unchanged:
  - the hard-coded OWNS ranges in the `*_complete.py` tools;
  - the "EDOP nonzero" slip in docs/SELF-CHECK.md.
