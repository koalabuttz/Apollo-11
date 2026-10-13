# Translate Comanche ALARM_AND_ABORT as transfer-ABI routines

Lane A. Milestone `cm-alarm`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and an operand table only, no ABI change). The Comanche counterpart of `docs/LM-ALARM.md`.

ALARM_AND_ABORT records an alarm code in the FAILREG words, lights PROG, and either returns to its caller or abandons the job. BAILOUT (= ABORT), POODOO and CCSHOLE first save the erasables with BANKCALL VAC5STOR. BAILOUT then goes through WHIMPER's RESUME and POSTJUMP to ENEMA. POODOO clears STATEFLG, REINTFLG and NODOFLAG and goes through MR.KLEAN to WHIMPER, unless V37FLBIT is set. The flight image reached ALARM 02,3650 with no routine table (the steward's flight-gap list); the older `cm_alarm_*`/`cm_abort_*` files implement pieces of this file for the legacy harnesses and stay as they are. Here all of it runs natively, one `agc_image_ops` call per original instruction.

## Scope

- `Comanche055/ALARM_AND_ABORT.agc`, every instruction:
  - Fixed-fixed 5650-5720: ALARM, ALARM2, BORTENT, PRIOENT, LARMENT (with Comanche's LOC/BANKSET copy to LOCALARM/BANKALRM), CHKFAIL1, CHKFAIL2, FAIL3, PROGLARM, MULTEXIT, MULTFAIL.
  - Fixed-fixed 5721-5772: POODOO, ABORT2, CCSHOLE, CURTAINS, VARALARM.
  - Fixed-fixed 7755-7774: BAILOUT = ABORT, WHIMPER.
  - PRIOLARM in bank 10 (3766-3776).
- Entries (`cm_alarm_routines`, 21):
  - The labels other code reaches: ALARM, ALARM2 (also the self-check's TCALARM2), BAILOUT (and TC ABORT), POODOO, CCSHOLE, CURTAINS, VARALARM, PRIOLARM.
  - MULTEXIT's `INDEX A; TC 1` targets inside the file: BAILOUT +11 (WHIMPER -1), POODOO +11, CURTAINS +4, VARALARM +6, PRIOLARM +7. MULTEXIT forms the instruction as the hardware does; it takes TC (to anything but a register) and TCF, and anything else refuses.
  - The returns from TC BANKCALL (CADR VAC5STOR in BAILOUT, POODOO and CCSHOLE; CADR MR.KLEAN) and from the three TC DOWNFLAGs.
  - ALMCADR (erasable 1363). CURTAINS's and VARALARM's `TC ALMCADR` executes the return address stored there. It accepts a word in 2000-7777 as TC; anything else refuses.
- Alarm codes:
  - ALARM and ALARM2 read the word after the caller's TC with `INDEX Q; CA 0`.
  - BAILOUT and POODOO read it after VAC5STOR with `INDEX ALMCADR; CAF 0`, where ALMCADR holds the caller's Q.
  - `tools/cm_alarm_generate.py` compiles that word for all 62 genuine TC ALARM, ALARM2, BAILOUT and POODOO sites (the same 62 as `src/cm_alarm_sites.inc`) into `src/cm_alarm_operands.inc`. It also compiles the CADRs after this file's own TC BANKCALL and TC POSTJUMP and the fixed constants the file reads, and checks each against the listing and the rope.
  - A program running these routines uses `cm_alarm_operands` as its operand table. A Q or ALMCADR at no compiled site refuses; an erasable one reads erasable memory.
- WHIMPER's `CA TWO; AD Z; TS BRUPT; RESUME` runs as yaAGC runs it. RESUME clears the in-interrupt state (`agc_hardware`) and executes BRUPT (TC 7773) at ZRUPT-1, which reaches `TC POSTJUMP; CADR ENEMA`. Without `program->hardware`, WHIMPER refuses.
- Interrupt state: INHINT and RELINT act on `cm_native_io.interrupt_enabled`; ROR SUPERBNK reads `cm_native_io.superbank`.
- External continuations: the callers' returns, PRIODSPR (PINBALL), DOWNFLAG (SERVICE_ROUTINES), and BANKCALL/POSTJUMP (`cm_interbank_routines`) with, through them, VAC5STOR, MR.KLEAN and ENEMA.

Paths with no producer:
- A TS that overflows refuses. In MULTFAIL that needs a negative alarm code. Every compiled code is positive, and so are the codes PRIOLARM's and VARALARM's callers pass in A.
- The CCS after `MASK FLAGWRD7` can't be negative, and refuses if it is.

## Source map

| AGC | Native (`src/cm_alarm.c`) |
|---|---|
| ALARM 5650 ... VARALARM 5772, BAILOUT/WHIMPER 7755-7774, PRIOLARM 10,3766-3776 | `body`, labels `L<address>`, one entry per routine |
| `TC ALMCADR` (erasable 1363) | `almcadr` |
| alarm codes, own CADRs, constants | `cm_alarm_operands` (`src/cm_alarm_operands.inc`) |

## Evidence

Hashes: `evidence/cm-alarm/SHA256.json`. Bulk files are assets of the `evidence-cm-alarm` release (`python3 tools/evidence.py fetch cm-alarm`).

- Independent assembly comparison: `evidence/cm-alarm/cm-alarm-proof.json`. `make cm-alarm-differential` runs the unmodified Comanche055 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters kind n % 79: each of the 62 TC sites (Q at its alarm code, FB of its bank), then CCSHOLE, CURTAINS, VARALARM, PRIOLARM, the twelve return points, and ALMCADR. The VAC5STOR returns of BAILOUT and POODOO have ALMCADR at a random BAILOUT or POODOO site.
  - Images, FB/EB, SUPERBNK, the interrupt-enable and in-interrupt states are pseudo-random. Each FAILREG word is +0, -0, positive or negative a quarter of the time, so every CHKFAIL/FAIL3 branch, PROGLARM and MULTFAIL run.
  - Each case runs to the first point neither this file nor INTER-BANK_COMMUNICATION owns. The interrupt-enable state, SUPERBNK, the in-interrupt state and all 2,048 erasable words are compared.
  - The digests are frozen into `tests/cm_alarm_vectors.h`. `evidence/cm-alarm/cm-alarm-data.json` lists every compiled word with its source line.
- Host and sanitizer results: `make test-cm-alarm sanitize-cm-alarm` (`evidence/cm-alarm/cm-alarm-host.log`). Beyond the oracle digests, the test checks that:
  - every compiled site code is a positive word in the operand table;
  - all 21 entries are reached;
  - each case ends at a caller's return, PRIODSPR, ENEMA, MR.KLEAN, VAC5STOR or DOWNFLAG, and each of those ends is reached;
  - a Q or ALMCADR at no compiled site, an ALMCADR word that is no return address, a negative code that overflows MULTFAIL, a missing io and WHIMPER without the hardware record are refused.
- C64 execution results: `make vice-cm-alarm` (`evidence/cm-alarm/cm-alarm-vice.log`): every 16th case (256).
- C64 branch outcomes: `evidence/cm-alarm/cm-alarm-c64-cover.json` (`python3 tools/lane_a_c64_cover.py --check cm-alarm`). The 256 stride rows already take all 287 branch arcs the 4,096 host cases take in every linked source, so the C64 program runs no extra cover rows.
