# Interpretive words of the LM tracking verbs (EXTENDED_VERBS)

Lane C. Milestone `lanec-ev-lmi`.

## Scope

The interpretive words of Luminary099 EXTENDED_VERBS around the basic code
already in `lanec-ev-track`:

- TRMTRAK1 (bank 42): `CALL INTSTALL`, then `EXIT`.
- ATTACHIT (bank 43): `CALL INTSTALL`; `SET MOONOTH`, `BON MOONTHIS`,
  `CLEAR MOONOTH`, `EXIT`; then `CALL PTOALEM`, `LXA,2 PBODY`,
  `CALL SVDWN1`, `EXIT`.
- TCPIN (bank 43): `RTB PINBRNCH`.

`src/lanec_ev_lmi.c` compiles these words onto the native interpreter paths
of `lm_interp_steps` and `lm_interp_orders` (INTPRET, NEWOPS, the MISCJUMP
bodies of CALL, SWITCH, LXA and RTB, DANZIG, EXIT), the same way the other
compiled Luminary routines do. Each order and operand word appears only as
the literal argument those paths take for the interpreter's bookkeeping.

## Entries and boundaries

| Entry | Ends at |
|---|---|
| INTPRET with Q at TRMTRAK1 +7, ATTACHIT +1 or ATTACHIT +16 | NEWOPS of INTSTALL or PTOALEM (Q at the word after the call: QPRET) |
| NEWOPS with LOC at ATTACHIT +3 (the switch orders) | the basic word after EXIT, or NEWOPS at the BON target |
| NEWOPS with LOC at TRMTRAK1 +9, ATTACHIT +9 or ATTACHIT +21 (the CALL returns) | the basic word after EXIT |
| NEWOPS with LOC at ATTACHIT +18 | NEWOPS of SVDWN1 |
| NEWOPS with LOC at TCPIN | PINBRNCH, through RTB (Q at SWRETURN) |

Domain, the interpreter's own contract: NEWJOB is +0 (no job change
pending), FIXLOC names a VAC area in unswitched erasable, and at NEWOPS EDOP
is +0 and BANKSET names the current banks. States outside it are out of
scope.

## Oracle stop at NEWOPS

A case can end at NEWOPS of a called routine, which is also the address the
interpreter passes through between the words translated here. The suite
names the LOC ranges of its own interpretive words (`lanec_meta.newops` and
`loc[]` in `include/lanec_case.h`), and `tools/lanec/oracle.c` stops at
NEWOPS only when LOC is outside them. Suites that leave `newops` at 0 are
unaffected, and every earlier Lane C suite produces the same oracle CRCs.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-ev-lmi/SHA256.json`.

- Independent assembly comparison: `lanec-ev-lmi-diff.json` (576 cases:
  all 9 entries, 64 variants each, random erasable with the
  MOONTHIS/MOONOTH bits, a random VAC area in FIXLOC and random registers) and `lanec-ev-lmi-oracle.crc`. gcov: every line and every
  branch outcome of the compiled words is taken; the only untaken
  outcomes are the out-of-scope returns.
- Host and sanitizer results: `make test-lanec-ev-lmi sanitize-lanec-ev-lmi`
  (`lanec-ev-lmi-sanitize.txt`).
- C64 execution results: `make vice-lanec-ev-lmi`, 116 frozen cases (every
  entry) on a VICE PAL C64 (`lanec-ev-lmi-vice.txt`).
