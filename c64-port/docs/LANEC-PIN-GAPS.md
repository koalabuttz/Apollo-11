# Pinball verb 07 and CHANLOAD

Lane C. Milestone `lanec-pin-gaps`.

## Scope

Two `PINBALL_GAME_BUTTONS_AND_LIGHTS` paths that no earlier milestone
translated, in both Luminary099 and Comanche055:

- Verb 07, display double-precision decimal: DSPDPDEC through ENDDPDEC
  (LM lines 1716-1730, CM lines 1697-1712, bank 40).
- The channel load used by the load verbs for channel nouns: CHANLOAD
  (LM lines 2004-2014, CM lines 1992-2002, bank 41).

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

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-pin-gaps/SHA256.json`.

- Independent assembly comparison: `lanec-pin-gaps-diff.json` (5,120 cases:
  both MIXBR branches, NOUNADD over unswitched, switched, MPAC-overlapping and
  editing-register pairs, both return points, every channel 0-077 with and
  without high NOUNCADR bits, both missions) and `lanec-pin-gaps-oracle.crc`.
- Host and sanitizer results: `make test-lanec-pin-gaps sanitize-lanec-pin-gaps`
  (`lanec-pin-gaps-sanitize.txt`).
- C64 execution results: `make vice-lanec-pin-gaps`, 214 frozen cases on a
  VICE PAL C64 (`lanec-pin-gaps-vice.txt`).
