# Run the Luminary T6RUPT programs as native routines

Lane A. Milestone `lm-t6rupt`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

Luminary099 T6-RUPT_PROGRAMS.agc times the RCS jets. The DAP loads a schedule (T6NEXT, T6FURTHA, NXT6ADR) and starts the TIME6 counter. When TIME6 runs out, the T6 interrupt writes the next jet word to channel 5 or 6 and restarts the counter for the next change. Every word of the file is now native (`lm_t6rupt_routines`, `src/lm_t6rupt.c`), together with the T6RUPT vector from INTERRUPT_LEAD_INS.agc and the RESUME exits from WAITLIST.agc that end this and other interrupts:

| AGC | Native |
|---|---|
| T6RUPT vector 04004 (DXCH ARUPT; EXTEND; DCA T6ADR; DTCB) | `vector`: ends at DOT6RUPT 17,2065 (FB 17, EB 6) |
| DOT6RUPT 17,2065 (LXCH BANKRUPT; EXTEND; QXCH QRUPT; TC T6JOBCHK; TCF RESUME) | `dot6rupt` → `t6jobchk`, then RESUME 05270 |
| T6JOBCHK 17,2027 (CCS TIME6; TC Q; TC CCSHOLE; TC T6JOBCHK +3) | `t6jobchk`: return, or CCSHOLE 05705 (Q 02032) |
| T6JOB 17,2033 to GOCH56 17,2064 | `t6jobchk`: stop the clock (WAND CHAN13), shift the schedule, restart the clock (ENABLET6: WOR CHAN13) unless TIME6 + PRIO37 overflows, drop a next time that would overflow, then INDEX L; TCF WRITEP -1 |
| WRITEP -1 05753, WRITEP 05754 | `fire`, `writex`: (NEXTP) to channel 6 |
| WRITEU -1 05757, WRITEU 05760 | `fire`, `write5`: channel 5 with its U jets (00314 clear) replaced |
| WRITEV -1 05770, WRITEV 05771, 00314OCT 05774 | `fire`, `write5`: channel 5 with its V jets (00314 kept) replaced |
| RESUME 05270, NOQRSM 05272, NOQBRSM 05274 | `resume`: restore Q (RESUME only), BB from BANKRUPT (not NOQBRSM), A and L from ARUPT, RELINT, end the interrupt at ZRUPT-1 |

The instructions run in order on the image through `agc_image_ops` (one call per source instruction, TS and CCS skips as in the source). The channel instructions follow yaAGC: WAND and WOR combine the overflow-corrected A with the channel and write the result back, and A takes it sign-extended; channel 13 keeps only what the CPU can leave set (043777). The jet channels and channel 13 live in `program->hardware`, which gains `channel5`, `channel6` and `channel13` (an ABI addition: initializers that name only `channel11` and `in_interrupt` leave them zero).

A return (TC Q) leaves Z at Q and Q at 3, which is what the original's RETURN does: it executes Q as a TC from address 2.

### Input domain

- The T6 entries need EBANK 6 (where T6NEXT, T6FURTHA, NXT6ADR and NEXTP/U/V live), and DOT6RUPT needs FB 17 and the interrupt, as the vector leaves them.
- NXT6ADR is 0, 4 or 13 (WRITEP, WRITEU or WRITEV), the only values the DAP (INDEXES) and DAPIDLER store.
- TIME6 is positive, +0 or -0. A negative nonzero TIME6 sends T6JOBCHK +3 into a TC to itself; nothing produces it.
- A TC Q return address lies in fixed memory (02000-07777), and a WRITEU or WRITEV entry has no overflow in A (TS L would skip).

Anything else is refused (`EXEC_OUT_OF_SCOPE`) with the image, io and hardware unchanged.

## Evidence

Hashes: `evidence/lm-t6rupt/SHA256.json`.

- **Source map.** `tools/lm_t6rupt_source_map.py` (`evidence/lm-t6rupt/lm-t6rupt-source-map.json`) lists all 53 assembled words of T6-RUPT_PROGRAMS.agc with their native function. All 51 executable instructions but one were executed by the original program in the comparison below. The exception is T6JOBCHK +3, which only the negative TIME6 the routines refuse reaches.
- **Independent assembly comparison.** `make lm-t6rupt-differential` runs the unmodified Luminary099 rope in yaAGC (host-only, `tools/lm_t6rupt_reference.c`) on 3,840 cases:
  - the vector, 256;
  - DOT6RUPT, 1,536;
  - T6JOBCHK called directly, 512;
  - the six jet writer entries, 768;
  - RESUME, NOQRSM and NOQBRSM, 768. A quarter of these return into rope banks 030-033 and 040-043 under SUPERBNK 060 or 0100, with BRUPT the rope word at the return address (one time in eight it differs, which the routine refuses), so the superbank RESUME uses is checked.

  TIME6 is mostly -0, and the schedule times sit below, at and above the PRIO37 overflow (01000) and at POSMAX. The native routine runs once. The original then runs from the same entry until it first reaches the continuation the native run left in Z (and FB in banked memory), or until the interrupt ends after a RESUME. Every case equals the original in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channels 5, 6 and 13. Per-case digests are frozen into `tests/lm_t6rupt_vectors.h`; see `evidence/lm-t6rupt/lm-t6rupt-proof.json`.
- **Host and sanitizer results.** `make test-lm-t6rupt sanitize-lm-t6rupt` (`evidence/lm-t6rupt/lm-t6rupt-host.log`) checks:
  - the digests;
  - that all twelve entries are reached, and every path: return, CCSHOLE, clock left stopped, restarted, next time dropped, and P, U and V jets;
  - five refusals with the image unchanged, and that only the rope-return RESUME cases whose BRUPT differs from the rope word are refused (26).
- **C64 execution results.** `make vice-lm-t6rupt` (`evidence/lm-t6rupt/lm-t6rupt-vice.log`) runs every sixteenth case (240) plus 4 cover rows.
