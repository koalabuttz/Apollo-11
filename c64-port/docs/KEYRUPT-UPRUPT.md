# Run the KEYRUPT and UPRUPT handlers of both missions as native routines

Lane A. Milestone `keyrupt-uprupt`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

KEYRUPT_UPRUPT.agc handles a DSKY key (KEYRUPT1) and an uplink word (UPRUPT). Both save BB and Q, sample the time (LODSAMPT) and start a CHARIN job through NOVAC with the 5-bit key code in its MPAC. An uplink word must carry its code three times (high and low five bits the code, the middle five its complement); a word that fails sets UPLOCKFL and locks out the uplink until an ERROR RESET code arrives. The file is the same code in both missions (LM 04,3274-3400, CM 07,3673-3776). Every word is native: `lm_keyrupt_routines` and `cm_keyrupt_routines` (`src/keyrupt_uprupt.c`). Offsets are from KEYRUPT1.

| AGC | Native |
|---|---|
| KEYRUPT1 +0, UPRUPT +023 (TS BANKRUPT; XCH Q; TS QRUPT; TC LODSAMPT) | `save`: ends at LODSAMPT (LM 04400, CM 04414) with Q at the entry +4. A Q holding overflow makes TS QRUPT skip the call, and the routine runs on without it. |
| KEYRUPT1 +4 (CAF LOW5; EXTEND; RAND MNKEYIN), KEYCOM +7, ACCEPTUP +013 | `keys`, `keycom`, `acceptup`: the key code (channel 15) to RUPTREG4, DSKYFBIT set in FLAGWRD5, CAF CHRPRIO; TC NOVAC with the 2CADR of CHARIN after it (LM 05072, CM 05134) |
| ACCEPTUP +017, NOVAC's return (CA RUPTREG4; INDEX LOCCTR; TS MPAC; TC RESUME) | `to_job`: the code to the new job's MPAC, then RESUME (LM 05270, CM 05333) |
| UPRUPT +027 to TMFAIL2, UPTEST +075 | `uplink`, `uptest`: INLINK to KEYTEMP1 (INLINK cleared), UPACT on (WOR channel 11 bit 3), the two redundancy tests (MP BIT10 shifts), then ERROR RESET clears UPLOCKFL (CLUPLOCK), a locked code ends at RESUME, an accepted one goes to ACCEPTUP, a failed one sets UPLOCKFL (TMFAIL2) |

KEYCOM is an entry because Comanche's SXTMARK jumps to it with a key code. On UPTEST's +0 path the original executes the HI10 constant (OCT 77740) as MASK 7740. That only changes A, which TMFAIL2 overwrites at once, so the native routine leaves it out. Luminary's file ends with a CS XDSPBIT at 04,3400 that no instruction or CADR reaches. The next label, UPPART2, is at 04,3401. That word has no native counterpart.

Channel 15 (the key code) joins `agc_hardware` as `channel15`, an ABI addition: initializers that name only the earlier members leave it zero.

### Input domain

- Every entry runs inside the interrupt, with FB in the file's bank, as the lead-ins and the callers' returns leave it.
- At KEYRUPT1, UPRUPT and KEYCOM, A holds no overflow. A lead-in leaves the old BB there, and SXTMARK leaves a key code.
- At NOVAC's return, LOCCTR is a core set offset (0, 12, ..., 72), as NOVAC leaves it.

Anything else is refused (`EXEC_OUT_OF_SCOPE`) with the image and io unchanged.

## Evidence

Hashes: `evidence/keyrupt-uprupt/SHA256.json`.

- **Source maps.** `tools/keyrupt_uprupt_source_map.py` (`evidence/keyrupt-uprupt/keyrupt-uprupt-source-map-{lm,cm}.json`) lists all 69 Luminary099 and 68 Comanche055 assembled words. The original program executed every reachable instruction of both files in the oracle run below, including HI10 as an instruction. The script also checks the bank, base address and LODSAMPT, NOVAC and RESUME targets built into the native routines against the assembled words. The repository's Luminary file names UPOK as UPCK; nothing refers to either label.
- **Independent assembly comparison.** `make keyrupt-uprupt-differential` runs the unmodified Luminary099 and Comanche055 ropes in yaAGC (host-only, `tools/keyrupt_uprupt_reference.c`) on 256 cases per entry, 1,536 per mission. The images are random apart from coherent bank registers, LOCCTR at a core set and FLAGWRD7's UPLOCKFL either way. A third of the uplink words carry a valid triple code, and a sixth of those are ERROR RESET. Channels 11 and 15 are random. The native routine runs once. The original then runs from the same entry until it first reaches the continuation the native run left in Z. Every case equals the original in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channel 11. Per-case digests are frozen into `tests/keyrupt_uprupt_vectors.h`; see `evidence/keyrupt-uprupt/keyrupt-uprupt-proof.json`.
- **Host and sanitizer results.** `make test-keyrupt-uprupt sanitize-keyrupt-uprupt` (`evidence/keyrupt-uprupt/keyrupt-uprupt-host.log`) checks:
  - the digests and the continuations;
  - that every entry is reached;
  - every uplink path: the first or second redundancy test failing, ERROR RESET, locked, accepted, and the TS QRUPT skip;
  - six refusals with the image unchanged.
- **C64 execution results.** `make vice-keyrupt-uprupt` (`evidence/keyrupt-uprupt/keyrupt-uprupt-vice.log`) runs every sixteenth case of both missions (192).
