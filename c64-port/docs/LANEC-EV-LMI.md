# Interpretive words of the LM tracking verbs (EXTENDED_VERBS)

Lane C. Milestone `lanec-ev-lmi`.

## Scope

The interpretive words of Luminary099 EXTENDED_VERBS around the basic code
already in `lanec-ev-track`:

- RRLOSDSP (bank 40, the V83 radar display job): `CALL RRNBMPAC`; the
  vector math from the body-axis line of sight to RR-ELEV and RR-AZ (STORE,
  STODL, STOVL, UNIT, DOT, STCALL ARCTRIG, BPL/DAD DPPOSMAX); `EXIT`.
- TRMTRAK1 (bank 42): `CALL INTSTALL`, then `EXIT`.
- ATTACHIT (bank 43): `CALL INTSTALL`; `SET MOONOTH`, `BON MOONTHIS`,
  `CLEAR MOONOTH`, `EXIT`; then `CALL PTOALEM`, `LXA,2 PBODY`,
  `CALL SVDWN1`, `EXIT`.
- TCPIN (bank 43): `RTB PINBRNCH`.

`src/lanec_ev_lmi.c` compiles these words onto the native interpreter paths
of `lm_interp_steps` and `lm_interp_orders` (INTPRET, NEWOPS, the MISCJUMP
bodies of CALL, SWITCH, LXA and RTB, DANZIG, EXIT), the same way the other
compiled Luminary routines do. RRLOSDSP's fixed operands (UNITX, UNITY,
UNITZ, HI6ZEROS, DPPOSMAX, at 23,2513-2530) are passed as literal words. Each order and operand word appears only as
the literal argument those paths take for the interpreter's bookkeeping.

## Entries and boundaries

| Entry | Ends at |
|---|---|
| INTPRET with Q at RRLOSDSP +4, TRMTRAK1 +7, ATTACHIT +1 or ATTACHIT +16 | NEWOPS of RRNBMPAC, INTSTALL or PTOALEM (QPRET at the word after the call) |
| NEWOPS with LOC at RRLOSDSP +6 (the CALL return) | NEWOPS of ARCTRIG, or POODOO1 when UNIT's argument is zero |
| NEWOPS with LOC at RRLOSDSP +21 or +24 (the ARCTRIG return and its BPL target) | NEWOPS of ARCTRIG |
| NEWOPS with LOC at RRLOSDSP +34 or +37 | the basic word after EXIT |
| NEWOPS with LOC at ATTACHIT +3 (the switch orders) | the basic word after EXIT, or NEWOPS at the BON target |
| NEWOPS with LOC at TRMTRAK1 +9, ATTACHIT +9 or ATTACHIT +21 (the CALL returns) | the basic word after EXIT |
| NEWOPS with LOC at ATTACHIT +18 | NEWOPS of SVDWN1 |
| NEWOPS with LOC at TCPIN | PINBRNCH, through RTB (Q at SWRETURN) |

Domain, the interpreter's own contract: NEWJOB is +0 (no job change
pending), FIXLOC names a VAC area in unswitched erasable above the
interpreter's work area (0200 up to 01400 less a VAC area), and at NEWOPS
EDOP is +0, BANKSET names the current banks and INTBIT15 holds BANKSET's
bit 15, as INTPRET leaves them. States outside it are out of
scope.

## Oracle stop at NEWOPS

A case can end at NEWOPS of a called routine, which is also the address the
interpreter passes through between the words translated here. The suite
names the LOC ranges of its own interpretive words (`lanec_meta.newops` and
`loc[]` in `include/lanec_case.h`), and `tools/lanec/oracle.c` stops at
NEWOPS only when LOC is outside them. Suites that leave `newops` at 0 are
unaffected, and every earlier Lane C suite produces the same oracle CRCs.

## Oracle notes

- UNIT, DOT and ARCTRIG run thousands of instructions, so the oracle allows
  40,000 per case (it was 4,000). The limit only reports a case that never
  reaches a boundary.
- The interpreter executes A (TC A in the unary dispatch) and short code it
  builds in erasable 0100-0177, so those addresses count as the
  interpreter's code.
- The Lane C oracle now runs every DV through the pinned engine's own
  hardware model, like the other interpreter oracles
  (`tools/prepare_hardware_dv.py`, `docs/DIVIDE.md`). The engine's fast DV
  path reads a 16-bit L of -0 as a nonzero word; UNIT's small-vector path
  reaches that DV when only a low-order component is left. Every earlier
  Lane C suite produces the same oracle CRCs with this engine.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-ev-lmi/SHA256.json`.

- Independent assembly comparison: `lanec-ev-lmi-diff.json` (960 cases:
  all 15 entries, 64 variants each, lines of sight along Y for UNIT's alarm
  and with only a low-order Z for its small-vector path, random erasable with the
  MOONTHIS/MOONOTH bits, a random VAC area in FIXLOC and random registers) and `lanec-ev-lmi-oracle.crc`. gcov: every line and every
  branch outcome of the compiled words is taken; the only untaken
  outcomes are the out-of-scope returns.
- Host and sanitizer results: `make test-lanec-ev-lmi sanitize-lanec-ev-lmi`
  (`lanec-ev-lmi-sanitize.txt`).
- C64 execution results: `make vice-lanec-ev-lmi`, 138 frozen cases (every
  entry) on a VICE PAL C64 (`lanec-ev-lmi-vice.txt`).
