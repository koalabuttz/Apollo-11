# Translate AGC_BLOCK_TWO_SELF-CHECK (Luminary099 and Comanche055, bank 43) as transfer-ABI routines

Lane A2 (restart, telemetry and support). Milestone `self-check`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routine tables only, no ABI change).

AGC_BLOCK_TWO_SELF-CHECK is the Executive's idle job (SELFCHK) and the rope and erasable tests behind it, selected by SMODE (verb 21 noun 27). SMODECHK lets a waiting job run through CHECKNJ (POSTJUMP ADVAN, back through SELFRET), then acts on SMODE: +0 idles, an option 1-8 picks one test through SOPTIONS (ERASCHK, ROPECHK, or back to the caller in SKEEP1 for the rest), anything else goes back to SIDLOOP. ERASCHK writes each erasable word's own address into it, two words at a time with interrupts inhibited, checks it and its complement through -1CHK, and puts the saved words back; PRERRORS repeats that repair after a restart (ERESTORE). CNTRCHK reads every counter, CYCLSHFT the edited registers. ROPECHK sums every word of every bank through SUPDACAL and ADSUM, bank by bank (NXTBNK, with the bank 17-20, 27-30 and superbank steps), and checks each bank's sum against its number (BNKCHK); STSHOSUM is the same walk for verb 91's show-banksum (SKEEP6 +1 shows each sum through SDISPLAY). A failing check goes to ERRORS, which saves Q in SFAIL and raises alarm 01102 through ALARM2.

The two missions assemble the same source: Luminary099 sits two words lower (43,3232-3723) than Comanche055 (43,3234-3725), and the routines it calls are at other addresses. `src/self_check_body.inc` names every word by its Comanche055 address; `src/lm_self_check.c` and `src/cm_self_check.c` build it with the mission's offset, outside entries and constants.

## Scope

- `Luminary099/AGC_BLOCK_TWO_SELF_CHECK.agc` and `Comanche055/AGC_BLOCK_TWO_SELF-CHECK.agc`: every instruction of bank 43 but FXADRS (Comanche055 3561-3564).
- Routines (`lm_self_check_routines`, `cm_self_check_routines`, 11 each): SELFCHK, STSHOSUM and NXTBNK (SHOWSUM in EXTENDED_VERBS), and the return point after each TC out of the file: ALARM2's (+0 SMODE entry at 3270), CHECKNJ's POSTJUMP through SELFRET (3306, 3443, 3627), SUPDACAL's (3542), CHANG1's (3625), LEFT5's (3675) and SDISPLAY's (3713).
- One `agc_image_ops` call per original instruction, so overflow, editing (CYR, SR, CYL, EDOP), the EB window and ones' complement behave as in the AGC. Constants are compiled from the source listing (`src/{lm,cm}_self_check_operands.inc`, `tools/file_operands_generate.py`); nothing is read from code memory.
- TC Q (-1CHK, ADSUM) and TC SFAIL / TC SKEEP1 execute the word in that register as an instruction, which leaves Q at the register +1 (yaAGC's behavior). The word must be a TC to a word the file continues at (the entries, the returns of TC -1CHK and TC ADSUM, ERASCHK, ROPECHK) or out of the file; anything else is refused.
- A run goes through the trampoline until it reaches a routine the file calls (ALARM2, POSTJUMP, SUPDACAL, CHANG1, LEFT5, SDISPLAY), ENDSUMS, FXADRS or a TC SFAIL / TC SKEEP1 target outside the file.
- Not translated: FXADRS (INDEX SKEEP3; CA 0), ROPECHK's read of the fixed-fixed rope. The native image holds only certified fixed data spans, so the run stops there with Z = FXADRS.
- Refused: a missing io record, FB other than 33, a SUPERBNK that does not select bank 43, EDOP nonzero, an ERASCHK or ERESTORE pair that reaches below 061 after the EB window (the central registers), and a TC through a register to erasable memory or into the middle of the file.
- Devices: io is a `p66_bias_io`; its interrupt-enable state takes the file's INHINT and RELINT.

## Source map

| AGC (Comanche055 addresses) | Native (`src/self_check_body.inc`) |
|---|---|
| PRERRORS, ERRORS (3250-3267) | `L3250`-`L3261`, `OUT` to ALARM2 |
| SMODE entry, SIDLOOP, -1CHK (3270-3302) | `L3270`-`L3275` |
| SMODECHK, SOPTIONS, SOPTION1-SOPTON10, CHECKNJ, SELFCHK (3303-3340) | `L3303`-`L3340` |
| ERASCHK, ERASLOOP, ELOOPFIN and the EB steps (3341-3467) | `L3341`-`L3467` |
| CNTRCHK, CYCLSHFT (3470-3521) | `L3470`-`L3520` |
| ROPECHK, STSHOSUM, COMMFX, COMADRS, FXFX (3522-3560) | `L3522`-`L3554` |
| FXADRS (3561-3564) | `L3561`: the run stops |
| ADSUM, ADRSCHK, CONTINU, ADRS+1 (3565-3634) | `L3565`-`L3627` |
| NXTBNK, 17TO20, CHKSUPR, NXTSUPR, GONXTBNK (3635-3671) | `L3635`-`L3664` |
| SOPTION, SOPT, BNKCHK (3672-3724) | `L3672`-`L3724` |

## Evidence

Hashes: `evidence/self-check/SHA256.json`. Bulk files are assets of the `evidence-self-check` release (`python3 tools/evidence.py fetch self-check`).

- Independent assembly comparison: `evidence/self-check/self-check-proof.json`. `make self-check-differential` runs the unmodified Luminary099 and Comanche055 ropes in yaAGC (host-only), 4,096 cases per mission.
  - Case n enters entry n % 11 with FB 33 and SUPERBNK selecting bank 43.
  - The image is pseudo-random within the program's domain: SMODE +0, -0, an option 1-8 (either sign) or out of range; SKEEP1 and SFAIL a TC to where the file continues there or out of the file; ERASCHK's address an erasable pair at or above 061 after the EB window it will run under, its limit often the next word, and ERESTORE +0 or that address; ROPECHK's address (often a bank's last), bank word (often the last bank or one before a numbering step), and running sums +-1, +-0; at SUPDACAL's return, the fetched word often a TC to itself; at SDISPLAY's return, a sum that passes or fails BNKCHK, or +-0; NEWJOB mostly +0; EB, the interrupt-enable state and the rest pseudo-random.
  - Each case runs until the original reaches, at an instruction boundary, a point outside bank 43 3234-3725 (shifted for Luminary099) or FXADRS. Z in erasable memory (the word a TC Q or TC SFAIL executes) counts as inside.
  - Compared: the interrupt-enable state, SUPERBNK and all 2,048 erasable words. The digests are frozen into `tests/lm_self_check_vectors.h` and `tests/cm_self_check_vectors.h`.
  - The freeze fails if an instruction of the file is never executed, other than FXADRS and two words no state reaches: -1CHK's TCF PRERRORS on A = +0 (3277) and ADRSCHK's TC CONTINU on L = +0 (3612). Each of those CCS tests a ones'-complement sum whose last addend is never +0 (an ERASCHK address, S+1, S-1, CONC+S2, a complemented address, or SBIT11), and the AGC adder only makes +0 from two +0 addends.
- Host and sanitizer results: `make test-self-check sanitize-self-check` (`evidence/self-check/self-check-host.log`). Beyond the oracle digests, the test checks that:
  - all 11 entries are reached in each mission;
  - each case ends at ALARM2, POSTJUMP, SUPDACAL, CHANG1, LEFT5 or SDISPLAY with Q at the return point, at ENDSUMS, at FXADRS, or at a TC SFAIL / TC SKEEP1 target outside the file, and each named end is reached;
  - a missing io, FB 32, a SUPERBNK selecting bank 33, a TC SFAIL to erasable memory or into the middle of the file and an ERASCHK pair over the central registers are refused.
- C64 execution results: `make vice-self-check` (`evidence/self-check/self-check-vice.log`): 77 rows per mission, every 64th case plus a greedy cover so the rows execute every word the host cases execute (`tools/c64_rows.py`).
