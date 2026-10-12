# Certificate review: DOWN-TELEMETRY (CM and LM)

Reviewed 2026-10-12 by the certificate reviewer at `port/c64-native-c` cf907b5a
(the merge of PR #70). It covers the certificates drafted by milestones
`cm-downtelemetry` (Comanche055/DOWN-TELEMETRY_PROGRAM.agc) and
`lm-downtelemetry` (Luminary099/DOWN_TELEMETRY_PROGRAM.agc).

Verdict: **accepted** for both files.

## What was checked

- The four evidence categories cite four distinct records: the doc map,
  `proof.json`, `host.log` and `vice.log`. None cites the source map. The
  comparison is a real yaAGC run of the unmodified rope
  (`tools/*_downtelemetry_reference.c`). Each run starts at DODOWNTM in the
  lead-in state and stops at Z = RESUME. Channels 13, 34 and 35 are compared
  after every downrupt, and the traps and all 2048 erasable words at the end.
- Transfer ABI:
  - DODOWNTM is registered at 05,3521 (CM) and 05,3506 (LM). Callers are the
    INTERRUPT_LEAD_INS `TCF DODOWNTM` and `DWNRPTBB BBCON DODOWNTM`.
  - Every other label reached from another file is registered at its original
    address: LDNPHAS1 → DNPHASE1 (FRESH_START_AND_RESTART) and LDNDUMPI →
    DNDUMPI (EXTENDED_VERBS). The self-stored DNTMGOTO targets are registered
    too.
  - `TC DNTMGOTO` is a registered erasable continuation that accepts exactly
    the six stored words and refuses any other.
  - The routine ends at RESUME through the trampoline, and RESUME is declared
    as an external continuation.
- Downlists, control lists and constants are compiled from the listing and
  checked against the rope at generation time (`--check` in `test-*`). Native
  code never reads the rope. The INDEX sites accept only the compiled
  `CA`/`READ` forms the source uses; there is no generic decoder.

## Reproduced

- `test-`, `sanitize-` (ASan/UBSan) and `-differential` for both milestones:
  48 cases each, with 18,815 (CM) and 18,830 (LM) checks and 0 failures. Every
  case equals the original program, and the vectors have no diff.
- The proofs regenerate byte for byte, and the gcov producer's output matches
  `snapshot_sha256`. `downtelemetry_generate.py --json` reproduces the release
  data JSON.
- `vice-*` (x64sc PAL): 24 checks with 0 failures for each. The PRG hashes
  c4f719fd… (CM) and ce68f2b4… (LM) equal the release logs.
- All evidence files match `SHA256.json` and the certificate hashes.
- gcov over the oracle producer, plus an outcome histogram at every CCS/BZMF
  site, found every untaken outcome to be impossible:
  - masked or RAND values cannot be negative;
  - no list word is 0 or 077777;
  - DNECADR is never ±0;
  - the source marks two outcomes "CAN'T GET HERE" and "A MUST NEVER BE ZERO".

  Everything else untaken is a refusal guard.
- The C64 subset was replayed through the native digest under gcov. Its unrun
  lines, branch outcomes and reached entries (all 8) are identical to the full
  host run.

## Notes for the lane

- The LM C64 stride of 2 aliases with DNLSTCOD = n % 6, so downlists 1, 3 and
  5 never go out on the C64. Both missions also skip the 1-dump and 4-dump
  cases there. Paths are fully covered, but running all 48 cases costs about a
  minute and would make the C64 evidence match the docs.
- `cm-downtelemetry-vice.log` holds two stale runs from an earlier PRG with a
  failing mailbox. `tools/vice_run.py:50` appends to monitor.log in a reused
  work directory. Clear it before each run and recapture the log.
- `docs/LM-DOWNTELEMETRY.md` names `DOWN-TELEMETRY_PROGRAM.agc`. The LM file
  is `DOWN_TELEMETRY_PROGRAM.agc`.
- `ctx()` does not check EB = 0, which the domain states. This is harmless,
  because every erasable address used before EBANK is set is unswitched.
