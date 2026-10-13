# Display-start entries (DISPLAY_INTERFACE_ROUTINES)

Lane C. Milestone `lanec-di-dsp`.

## Scope

The display-start entries of `DISPLAY_INTERFACE_ROUTINES.agc` in both
Luminary099 (lines 609-700, 803-940, 1168-1169) and Comanche055
(lines 644-729, 832-965, 1188-1189), all in bank 10: the entries
user programs call to start a normal, priority, flashing, perform or
"R" (immediate return) display, the code they share up to the scheduling
of the display job, and the copy of a display request into its permanent
cells. Comanche055 has no REFLASHR; otherwise the two files are the same
code at different addresses.

Each native entry runs one source path from its entry to the first boundary
and leaves the image exactly as the original program leaves it there.

## Source map

| Source | Native | Boundary |
|---|---|---|
| GODSP, GODSP2, GODSPRET, CLEANDSP, REFLASH, REGODSP, CLOCPLAY, GOFLASH, PRIODSP, SETPRIO, EXDSPRET, GOPERF1, GOPERFS, GOPERF2, GOPERF4 (via PURRS4) | `lanec_di_dsp_dispatch` | GOFLASH2 |
| GOFLASH2 `TS PLAYTEM4`, `TC SAVELOCS` | `goflash2` | MAKECADR (via SAVELOCS); MAKEPLAY when the TS overflows |
| GODSPR, GODSPR1, GODSPR2, REFLASHR (LM), REGODSPR, PRIODSPR, GOFLASHR, GODSPRS, GOPERF1R, GOPERFRS, GOPERF2R, GOPERF4R (via PURRS4) | `godsprs`, `godsprs1` | NOVAC (2CADR MAKEPLAY) for a non-flashing display; SPVAC for a flashing one (VACDSP) |
| PURRS4 (`TC Q` executes the return held in Q) | `purrs4` | back to GOPERF4 +1 / GOPERF4R +1 |
| GODSPRS1 +13 (NOVAC returns), BOTHJOBS (SPVAC returns) | `lanec_di_dsp_dispatch` | MAKECADR (via SAVELOCS) |
| SAVELOCS, SAVELOCR | `savelocs`, `savelocr` | MAKECADR |
| SAVELOCS +7 (MAKECADR returns), `TC L` | `saved_return` | MAKEPLAY (return GOFLASH2 +2); MAKECADR again after BOTHJOBS copies the user's MPAC +1..+4 into the job's register set (return BOTHJOBS +1); BANKJUMP (return BOTHJOBS +12) |
| COPYNORM, COPIES, COPIES2, SKIPADD, RELINTQ | `copies2` | the caller's return (`TC Q`) |
| CHKPRIO | `lanec_di_dsp_dispatch` | MARSLEEP, or SETNORM after setting the mark-over-normal flag |
| MARKPERF | `lanec_di_dsp_dispatch` | NV50DSP |

`TC Q` and `TC L` run the return address held in the register as a TC; the
native code does the same and leaves Q = 3 or 2, as the AGC does. Every
`TS` keeps its overflow skip, including the ones the source can never
overflow (they are kept so the code reads like the source).

Domain: the return held in L at SAVELOCS +7 is one of the three SAVELOCR
saves; LOCCTR is a register-set offset 0-01215, so both BOTHJOBS copies land
in unswitched erasable below 01377 (copies that overlap MPAC are done in
source order); COPINDEX is 0-0300 and the caller's return is in fixed-fixed
memory (04000-07777). Anything else is out of scope.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`). Constants are the
assembled words read from the pinned listings by label (`symbols.txt`
`WORD` rows), not copied by hand. Hashes: `evidence/lanec-di-dsp/SHA256.json`.

- Independent assembly comparison: `lanec-di-dsp-diff.json` (3,264 cases:
  all 34 entries and return points, both missions, A with and without
  overflow, flashing and non-flashing displays, all three MAKECADR returns,
  BOTHJOBS copies into any register set, COPIES indices and returns, CHKPRIO
  with and without a priority display) and `lanec-di-dsp-oracle.crc`.
- Host and sanitizer results: `make test-lanec-di-dsp sanitize-lanec-di-dsp`
  (`lanec-di-dsp-sanitize.txt`).
- C64 execution results: `make vice-lanec-di-dsp`, 37 frozen cases on a VICE PAL C64,
  picked by `tools/lanec/cover.py` so that together they take every
  branch arc the 3,264 host cases take (`lanec-di-dsp-vice.txt`).
