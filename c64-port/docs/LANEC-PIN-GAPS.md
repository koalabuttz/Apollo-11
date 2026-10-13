# Pinball verb 07 and CHANLOAD

Lane C. Milestone `lanec-pin-gaps`.

## Scope

The `PINBALL_GAME_BUTTONS_AND_LIGHTS` paths that no earlier milestone
translated, in both Luminary099 and Comanche055:

- Verb 07, display double-precision decimal: DSPDPDEC through ENDDPDEC
  (LM lines 1716-1730, CM lines 1697-1712, bank 40).
- The channel load used by the load verbs for channel nouns: CHANLOAD
  (LM lines 2004-2014, CM lines 1992-2002, bank 41).
- DSP2DEC through END2DEC, the sign and ten-digit display that verb 07 calls
  (LM lines 2559-2569, CM lines 2557-2567, bank 40).
- The bank-41 lead-ins NVSUBB (`TC NVSUB1`), LOADLV1 (`TC LOADLV`) and GOALMCYC
  (`TC ALMCYCLE`, the octal-only entry of the PUTDCSF2 scale-factor table).
- DSPDCWD1, the per-digit loop of the decimal display (bank 40): its
  `CAF FOUR` lead-in, the loop head and the point where DSPIN returns.
- The fixed-fixed LODSAMPT, FLASHON and FLASHOFF, and the lamp test's
  WAITLIST and NOVAC return points.
- The words the file reaches through POSTJUMP and BANKJUMP: GOVNUPDT (the
  verb display of UPDATVB) and its return from DSPDECVN, UPDAT1 +2, DSPOCTIN
  and its return from DSPIN (bank 40), and OCTBACK, the octal display's
  per-character loop (bank 41).

Each native entry runs one source path from its entry to the first boundary
and leaves the image exactly as the original program leaves it there (A/L/Q
debris, the stores, Z at the boundary, FB/BB, superbank, channels).

## Source map

| Source | Native | Boundary |
|---|---|---|
| DSPDPDEC `INDEX MIXBR`, `TC +0`, `TC +2`, `TC DSPALARM` | `lanec_pin_dspdpdec` | DSPALARM for a mixed noun (MIXBR 2) |
| DSPDPDEC +4..+12: `DCA` of the noun's word pair, `DXCH MPAC`, DSPCOUNT = R1D1, MPAC +2 = 0 | `lanec_pin_dspdpdec` | TPAGREE |
| DSPDPDEC +13 (TPAGREE returns) | `lanec_pin_dspdpdec` | DSP2DEC |
| ENDDPDEC (DSP2DEC returns) | `lanec_pin_dspdpdec` | ENTEXIT, the saved ENTRET word |
| CHANLOAD `CS SEVEN`, `AD NOUNCADR`, `BZF LOADLV` | `lanec_pin_chanload` | LOADLV, nothing written, for channel 7 (the superbank) |
| CHANLOAD +4..+10: `MASK LOW9`, `XCH MPAC`, `INDEX MPAC`, `WRITE 0` | `lanec_pin_chanload` | LOADLV |
| DSP2DEC `XCH Q`, `TS WDRET`, `CAF ZERO`, `TS CODE`, `CAF THREE`, `TC 11DSPIN` | `lanec_pin_dsp2dec` | 11DSPIN (-R2 off). A Q that overflows makes `TS WDRET` skip `CAF ZERO`, so CODE gets +1 or -1, as on the AGC |
| DSP2DEC +6 `CAF FOUR`, `TC 11DSPIN` (11DSPIN returns) | `lanec_pin_dsp2dec` | 11DSPIN (+R2 off) |
| DSP2DEC +8 `TC DSPSIGN` | `lanec_pin_dsp2dec` | DSPSIGN |
| DSP2DEC +9 `CAF R2D1`, END2DEC `TC DSPDCWD1` | `lanec_pin_dsp2dec` | DSPDCWD1, which returns through WDRET |
| NVSUBB `TC NVSUB1`, LOADLV1 `TC LOADLV`, GOALMCYC `TC ALMCYCLE` | `lanec_pin_leadin` | NVSUB1, LOADLV, ALMCYCLE |
| DSPDCWD1 -1 `CAF FOUR`; DSPDCWD1 `TS WDCNT`, `CAF BINCON`, `TC SHORTMP`, the relay code from RELTAB, `XCH CODE`/`XCH DSPCOUNT`/`TS COUNT`, `CCS A` | `lanec_pin_dspdcwd1` | DSPIN (Q = DSPDCWD1 +16); a negative count goes on as at +16, -0 to the next digit |
| DSPDCWD1 +16 (DSPIN returns): `CCS WDCNT` and its four branches, `CS VD1`/`TS DSPCOUNT` | `lanec_pin_dspdcwd1` | the next digit (Q = DSPDCWD1 +18), or WDRET |
| LODSAMPT `DCA TIME2`, `DXCH SAMPTIME`; FLASHON/FLASHOFF on channel 11 | `lanec_pin_small` | the caller in Q (switched or fixed-fixed) |
| the lamp test's `TC WAITLIST` and `TC NOVAC` returns | `lanec_pin_small` | ENDOFJOB, TASKOVER |
| GOVNUPDT `TC DSPDECVN`; +1 (DSPDECVN returns through WDRET) `TC POSTJUMP` | `lanec_pin_jumps` | DSPDECVN; POSTJUMP (CADR UPDAT1 +2) |
| UPDAT1 +2 `TC UPDATRET` | `lanec_pin_jumps` | the return word UPDATVB saved |
| DSPOCTIN `TC DSPIN`; +1 (DSPIN returns) `CAF +2`, `TC BANKJUMP` | `lanec_pin_jumps` | DSPIN; BANKJUMP with A = ENDSPOCT (CADR OCTBACK) |
| OCTBACK `CCS WDCNT`: WDAGAIN (`TS WDCNT`, `CS CYL` three times, `CS A`, `MASK DSPMSK`, the relay code from RELTAB, `TS CODE`, `XCH DSPCOUNT`, `TS COUNT`, `CCS A`, `TS DSPCOUNT`) | `lanec_pin_jumps` | POSTJUMP (CADR DSPOCTIN), the next character |
| OCTBACK's other branches, DSPLV `CS VD1`, `TS DSPCOUNT` | `lanec_pin_jumps` | WDRET |
| dispatch by Z and bank | `lanec_pin_gaps_dispatch` | |

`NOUNADD` may name any erasable pair from 010 up to 01376 or inside the
current EB bank (01400-01776). A pair that overlaps MPAC is read before
`DXCH` writes, as the source does. The double-word read rewrites its words,
so a pair touching CYR, SR, CYL or EDOP (020-023) edits them again
(`lanec_rewrite`), exactly as the AGC does. A pair that straddles 01377 and
01400, or starts below 010, is out of scope.

CHANLOAD writes channels 0-077 through `lanec_write`, which keeps the AGC
channel rules for L (1), Q (2), 013, 033 and 077. Writes to channels with
external latches (013 HANDRUPT trap enables, 010 relay rows, 011 bit 10
RESTART reset, 034/035 downlink timing) store the program-visible channel
word here; the latch effects belong to the C64 I/O layer that consumes
channel writes and are not claimed by this milestone. A NOUNCADR whose low
nine bits name a channel above 077, or name channel 7 with other bits set,
is out of scope.

DSPDCWD1 runs SHORTMP (MPAC times BINCON) on a copy and refuses unless the
product's top word is a digit 0-9 (or -0), as it is for every magnitude
DSPDECWD leaves in MPAC; an A that overflowed into `TS WDCNT` is out of scope.
SHORTMP's and DMPSUB2's words are part of the oracle's range and its
`TC Q` runs through Q.

OCTBACK runs WDAGAIN inline. Each `CS CYL` reads CYL and edits it again,
so three reads leave it cycled left three times. When WDCNT is positive the
next character's `CCS A` must find the old DSPCOUNT positive or +0 (DSPOCTWD
leaves the display position there, plus BIT14 to blank the signs); a negative
DSPCOUNT would run the `CADR DSPOCTIN` word as an instruction, and is out of
scope.

`DCA` reads the pair's second word into L before the first into A, so a
pair whose first word is L (NOUNADD 1) gives A the new L; a central register
read through its address gives its low 15 bits, sign-extended. Verb 07
follows both rules.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-pin-gaps/SHA256.json`.

- Independent assembly comparison: `lanec-pin-gaps-diff.json` (14,848 cases:
  both MIXBR branches, NOUNADD over unswitched, switched, MPAC-overlapping and
  editing-register pairs, both return points, every channel 0-077 with and
  without high NOUNCADR bits, DSP2DEC from its entry with and without an
  overflowed Q and from each return, the lead-ins, LODSAMPT/FLASHON/FLASHOFF
  with fixed and erasable callers, the lamp-test returns, DSPDCWD1 from its
  three points with every digit, every CCS branch and out-of-range A or MPAC,
  the POSTJUMP and BANKJUMP targets with OCTBACK over every WDCNT branch,
  positive, blanking and negative DSPCOUNT and any CYL, both missions;
  14,545 identical, 303 refused unchanged) and
  `lanec-pin-gaps-oracle.crc`.
- Host and sanitizer results: `make test-lanec-pin-gaps sanitize-lanec-pin-gaps`
  (`lanec-pin-gaps-sanitize.txt`).
- C64 execution results: `make vice-lanec-pin-gaps`, @ROWS@ frozen cases that `tools/lanec/cover.py pick`
  chose to take every host branch arc of the suite, on a VICE PAL C64 (`lanec-pin-gaps-vice.txt`).
