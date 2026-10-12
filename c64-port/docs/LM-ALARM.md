# Translate Luminary ALARM_AND_ABORT as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `lm-alarm`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and an operand table only, no ABI change).

ALARM_AND_ABORT records an alarm code in the FAILREG words, lights PROG, and either returns to its caller or abandons the job: BAILOUT and POODOO go through WHIMPER's RESUME and POSTJUMP to ENEMA, and POODOO also points restart group 4 at GOPOODOO, which clears flags and terminates or idles the jobs it owns. All of it is native here, one `agc_image_ops` call per original instruction.

## Scope

- `Luminary099/ALARM_AND_ABORT.agc`, every instruction: fixed-fixed 5567-5752 (ALARM, ALARM2, BORTENT, PRIOENT, LARMENT, CHKFAIL1, CHKFAIL2, FAIL3, PROGLARM, MULTEXIT, MULTFAIL, BAILOUT, WHIMPER = ABORT, POODOO, ABORT2, GOPOODOO, STRTIDLE, CCSHOLE, CURTAINS, BAILOUT1, BOTHABRT, POODOO1, ALARM1, ALMNCADR, VARALARM), PRIOLARM in bank 10 (3562-3572) and FLAGS in bank 13 (3755-3766).
- Entries (`lm_alarm_routines`, 26): the labels other code reaches (ALARM, ALARM2, BAILOUT, WHIMPER, POODOO, ABORT2, GOPOODOO from restart group 4, CCSHOLE, CURTAINS, BAILOUT1, POODOO1, ALARM1, ALMNCADR, VARALARM, PRIOLARM, FLAGS) and the points where control comes back into the file:
  - MULTEXIT's `INDEX A; TC 1` targets inside it (BAILOUT +7, POODOO +7, CURTAINS +4, VARALARM +6, PRIOLARM +7). MULTEXIT forms the instruction as the hardware does; TC (to anything but a register) and TCF are taken, anything else refuses.
  - The returns from TC BANKCALL (CADR FLAGS, V37KLEAN, MR.KLEAN) and TC SUPERSW.
  - ALMCADR (erasable 1363): CURTAINS's and VARALARM's `TC ALMCADR` executes the return address stored there. It accepts a word in 2000-7777 as TC; anything else refuses.
- Alarm codes: `INDEX Q; CA 0` reads the word after the caller's TC. `tools/lm_alarm_generate.py` compiles that word for all 73 genuine TC ALARM, ALARM2, BAILOUT, POODOO, ABORT2, BAILOUT1, POODOO1 and ALARM1 sites (ALMNCADR has none), plus the CADRs after this file's own TC BANKCALL and TC POSTJUMP and the fixed constants it reads, into `src/lm_alarm_sites.inc`. Each is checked against the listing and the rope. A program running these routines uses `lm_alarm_operands` as its operand table. A Q at no compiled site refuses (an erasable Q reads erasable memory).
- WHIMPER's `CA TWO; AD Z; TS BRUPT; RESUME` is run as yaAGC runs it: RESUME clears the in-interrupt state (`agc_hardware`) and executes BRUPT (TC 5650) at ZRUPT-1, which reaches `TC POSTJUMP; CADR ENEMA`. Without `program->hardware`, WHIMPER refuses.
- FLAGS ends in `TC Q`, which executes the word in Q as TC and leaves Q = 3.
- Interrupt state: INHINT and RELINT act on `p66_bias_io.interrupt_enabled`; ROR SUPERBNK reads `p66_bias_io.superbank`.
- External continuations: the callers' returns, PRIODSPR (PINBALL), BANKCALL/POSTJUMP/SUPERSW (`lm_interbank_routines`) and through them ENEMA, V37KLEAN, MR.KLEAN and SERVIDLE.

Paths with no producer: a TS that overflows refuses. In MULTFAIL that needs a negative alarm code, and every compiled code is positive, as are the codes PRIOLARM's and VARALARM's callers pass in A. The CCS after `MASK V37FLBIT` can't be negative and refuses if it is.

Note on SWRETURN: `lm_interbank_routines` takes SWRETURN only with Q in 0100-7775. FLAGS's `TC Q` leaves Q = 3, so a run of BANKCALL FLAGS stops at SWRETURN, and the code after it (GOPOODOO +3) is entered directly in the tests. Lane A has been told.

## Source map

| AGC | Native (`src/lm_alarm.c`) |
|---|---|
| ALARM 5567 ... VARALARM 5752, PRIOLARM 10,3562-3572 | `body`, labels `L<address>`, one entry per routine |
| FLAGS 13,3755-3766 | `flags` |
| `TC ALMCADR` (erasable 1363) | `almcadr` |
| alarm codes, own CADRs, constants | `lm_alarm_operands` (`src/lm_alarm_sites.inc`) |

## Evidence

Hashes: `evidence/lm-alarm/SHA256.json`. Bulk files are assets of the `evidence-lm-alarm` release (`python3 tools/evidence.py fetch lm-alarm`).

- Independent assembly comparison: `evidence/lm-alarm/lm-alarm-proof.json`. `make lm-alarm-differential` runs the unmodified Luminary099 rope in yaAGC (host-only), 4,096 cases. Case n enters kind n % 91: each of the 73 TC sites (Q at its alarm code, FB of its bank), then CCSHOLE, CURTAINS, VARALARM, WHIMPER, GOPOODOO, PRIOLARM, FLAGS, the nine return points, ALMCADR, and ALMNCADR at a random site's code. Images, FB/EB, SUPERBNK, the interrupt-enable and in-interrupt states are pseudo-random. Each FAILREG word is +0, -0, positive or negative a quarter of the time, so every CHKFAIL/FAIL3 branch, PROGLARM and MULTFAIL run. Each case runs to the first point neither this file nor INTER-BANK_COMMUNICATION owns, or SWRETURN. The interrupt-enable state, SUPERBNK, the in-interrupt state and all 2,048 erasable words are compared. The digests are frozen into `tests/lm_alarm_vectors.h`. `evidence/lm-alarm/lm-alarm-data.json` lists every compiled word with its source line.
- Host and sanitizer results: `make test-lm-alarm sanitize-lm-alarm` (`evidence/lm-alarm/lm-alarm-host.log`). Beyond the oracle digests, the test checks that:
  - every compiled site code is a positive word in the operand table;
  - all 26 entries are reached;
  - each case ends at a caller's return, PRIODSPR, ENEMA, V37KLEAN/MR.KLEAN, SERVIDLE or SWRETURN (with Q = 3);
  - a Q at no compiled site, an ALMCADR word that is no return address, a negative code that overflows MULTFAIL, a missing io and WHIMPER without the hardware record are refused.
- C64 execution results: `make vice-lm-alarm` (`evidence/lm-alarm/lm-alarm-vice.log`): every 16th case (256).
