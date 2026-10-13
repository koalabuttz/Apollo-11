# Translate RTB_OP_CODES (RT8_OP_CODES)

Lane D1b (alignment and maneuvers), issue #57. Milestone `rtb-op-codes`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/RTB_OP_CODES.agc` (`src/lm_rtb_op_codes.c`, 130 words, 10,3573-3777) and `Comanche055/RT8_OP_CODES.agc` (`src/cm_rtb_op_codes.c`, 206 words; the listing calls it `RTB_OP_CODES.agc`) are native C, every word. These are the basic routines the interpreter's RTB order and BOVB reach, compiled into routine tables of the control-transfer ABI (24 Luminary and 44 Comanche routines), keyed at each entry the source uses:

- Both: LOADTIME, CDULOGIC, SGNAGREE, 1STO2S, V1STO2S, TPMODE, 2V1STO2S, CDUINC, PULSEIMU, SIGNMPAC, DPMODE, NORMUNX1, NORMUNIT and VECSGNAG.
- Comanche also: READPIPS, VECMODE, INCRCDUS, TRANSP1, TRANSP2 and QUALITY1 (with QUALITY2, the interpretive J2 term in MODCHG3/MODCHG2).

TPMODE, DPMODE and VECMODE, which other code only fell into before, are entries of their own. Every return from a call (TPAGREE, 1TO2SUB, CDUINC, BANKCALL VECAGREE) and every word after a DANZIG is a routine too.

Transfers out of the file go through the trampoline with Z set, as the original leaves it: TCF DANZIG, SLOAD2, SLOAD2 +2, NEWMODE and VMODE back to the interpreter; BANKCALL IMUPULSE and BANKCALL VECAGREE with their CADRs; POSTJUMP UNIT +1. On the flight image those land on the interpreter's tables; here the run stops there (`EXEC_UNMAPPED`), and yaAGC stops at the same Z.

The earlier hand-written bodies (`src/rtb_angles.c`, `src/rtb_agreement.c`, `src/rtb_vector.c`, `src/rtb_unit.c`, `src/rtb_pulse_return.c`) stay: compiled interpretive files still call them through spec `rtb` entries.

## Evidence

Hashes: `evidence/rtb-op-codes/SHA256.json`. Bulk files are assets of the `evidence-rtb-op-codes` release.

- **Source map.** `rtb-op-codes-source-map.json` maps all 130 and 206 words to their routines; none untranslated.
- **Independent assembly comparison.** `make rtb-op-codes-differential` runs 512 states per routine and program: 34,816 cases. They equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. The cases take any EB, and a quarter put edge values into MPAC and the operands. TIME1, TIME2 and the CDUs vary in every case. CDUINC's BUF names THETAD, +1 or +2; after NORMUNIT's VECAGREE, MPAC +2 is FIXLOC or FIXLOC +1. The routines that never name FIXLOC take any FIXLOC in half their cases (spec `any_fixloc`), since RTB runs in NOVAC jobs too.
- **Host and sanitizer results.** `make test-rtb-op-codes sanitize-rtb-op-codes` runs the 1,132 frozen cases: every 32nd case, plus a greedy cover of every branch outcome any case reaches.
- **C64 execution results.** `make vice-rtb-op-codes` runs three diagnostics with 144 cases. They reach all 828 branch outcomes the host cases reach (`rtb-op-codes-c64-outcomes.json`). The log records each PRG's sha256. The `INDEX LIMITS -2`/`-1` sums take the end-around carry in 16-bit arithmetic (`index_carry16`), as the host build does. Before, the C64 build dropped it, and the 18 rows with a negative index passed only because LIMITS -2 and -1 both hold 37777.
