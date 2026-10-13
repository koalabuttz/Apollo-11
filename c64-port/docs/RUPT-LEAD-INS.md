# Run the interrupt lead-ins of both missions as native routines

Lane A. Milestone `rupt-lead-ins`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

INTERRUPT_LEAD_INS.agc is the fixed-fixed code at 04000-04077 that the AGC enters on a restart (GOPROG) and on every interrupt. Each lead-in saves A and L in ARUPT, sets BB for its handler and jumps there. Every lead-in of both missions is now native: `lm_lead_in_routines` and `cm_lead_in_routines` (`src/rupt_lead_ins.c`), with the T6RUPT (LM) and T3RUPT vectors already native in other milestones. `cm_lead_in_routines` also carries Comanche WAITLIST.agc's RESUME family, which HANDRUPT jumps into and which every Comanche interrupt ends with (Luminary's are in `lm_t6rupt_routines`).

| AGC | Native |
|---|---|
| 04000 GOPROG lead-in (INHINT; CAF GOBB; XCH BBANK; TCF GOPROG) | `goprog`: interrupts inhibited, BB from GOBB (LM 012103, CM 012063), Z at GOPROG (LM 02667, CM 02526) |
| Banked vectors (DXCH ARUPT; CAF xxRPTBB; XCH BBANK; TCF handler): T4RUPT 04020, KEYRUPT1 04024, KEYRUPT2 04030, UPRUPT 04034, DOWNRUPT 04040, RADAR 04044, LM RUPT10 04050, CM HANDRUPT 04050 | `vector`: A and L to ARUPT, BB from the BBCON, A the old BB, Z at the handler |
| LM T5RUPT 04010, CM T6RUPT 04004 (DXCH ARUPT; EXTEND; DCA 2CADR; DTCB) | `lm_t5`, `cm_t6`: Z and BB from T5ADR (01274) or T6LOC (01310), A the DTCB address +1, L the old BB |
| CM T5RUPT 04010 (DXCH ARUPT; CS TIME5; AD .5SEC; TCF T5RUPT) and T5RUPT 04065 (EXTEND; BZMF NOQBRSM; EXTEND; DCA T5LOC; DTCB) | `cm_t5`: NOQBRSM 05337 when TIME5 is within half a second of overflow, else T5LOC (01312) |
| CM RESUME 05333, NOQRSM 05335, NOQRSM +1 05336, NOQBRSM 05337 | `cm_resume`: Q from QRUPT (RESUME), BB from BANKRUPT (to NOQRSM), A and L from ARUPT, RELINT, end the interrupt at ZRUPT-1 |

The instructions run in order on the image through `agc_image_ops`. The BBCON constants and handler addresses built into the tables are checked against the assembled words by the source map below.

### Input domain

- Every entry but GOPROG runs inside an interrupt (`program->hardware->in_interrupt`), as the hardware leaves it.
- The 2CADRs at T5ADR, T6LOC and T5LOC name fixed memory (02000-07777), as every producer stores them (the DAP, T6 and T5 schedulers, and the restart).
- The Comanche RESUME family needs ZRUPT to be an address (1-010000), and BANKRUPT a BB value (only FB, superbank and EB bits) where NOQRSM reads it.

Anything else is refused (`EXEC_OUT_OF_SCOPE`) with the image and io unchanged.

## Evidence

Hashes: `evidence/rupt-lead-ins/SHA256.json`.

- **Source maps.** `tools/rupt_lead_ins_source_map.py` (`evidence/rupt-lead-ins/rupt-lead-ins-source-map-{lm,cm}.json`) lists all 54 Luminary099 and 58 Comanche055 assembled words with their native routine. The original program executed every instruction of both files in the oracle run below. The T6RUPT (LM) and T3RUPT vectors are run there only to record coverage; their equivalence is compared in `lm-t6rupt`, `lm-exec-jobs` and `cm-exec-services`. The script also checks that the native BBCONs and handlers equal the assembled words. Comanche's T5RPTBB is assembled but nothing reads it.
- **Independent assembly comparison.** `make rupt-lead-ins-differential` runs the unmodified Luminary099 and Comanche055 ropes in yaAGC (host-only, `tools/rupt_lead_ins_reference.c`) on 128 cases per entry, 1,152 LM and 1,920 CM. The images are random apart from coherent bank registers, 2CADRs in fixed memory and TIME5 on both sides of half a second from overflow. The native routine runs once. The original then runs from the same vector until it first reaches the continuation the native run left in Z (and FB in banked memory), or until the interrupt ends after a RESUME. A quarter of the CM RESUME, NOQRSM and NOQBRSM cases return into rope banks 030-033 and 040-043 under SUPERBNK 060 or 0100, with BRUPT the rope word at the return address (one time in eight it differs, which the routine refuses). Every case equals the original in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit and InIsr. Per-case digests are frozen into `tests/rupt_lead_ins_vectors.h`; see `evidence/rupt-lead-ins/rupt-lead-ins-proof.json`.
- **Host and sanitizer results.** `make test-rupt-lead-ins sanitize-rupt-lead-ins` (`evidence/rupt-lead-ins/rupt-lead-ins-host.log`) checks the digests, that every entry is reached, both CM T5RUPT continuations, that the interrupt stays in progress until a RESUME ends it, and seven refusals with the image unchanged, and that only the rope-return RESUME cases whose BRUPT differs from the rope word are refused (16).
- **C64 execution results.** `make vice-rupt-lead-ins` (`evidence/rupt-lead-ins/rupt-lead-ins-vice.log`) runs every sixteenth case of both missions (192).
