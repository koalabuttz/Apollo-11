# Pinball heads for the display consumers

Pinball module split lane. Milestone `lanec-pin-heads-display`; the head mechanism in general is docs/LANEC-PIN-HEADS.md (Lane C).

## Why

Six composite modules run whole display paths of `PINBALL_GAME_BUTTONS_AND_LIGHTS`
as one routine: `cm_dynamic_display`, `cm_normal_display`, `cm_nv85` (CM),
and `decimal_time_consumer`, `decimal_consumer`, `monitor_consumer` (both
missions). Each links the component modules it dispatches over and is far
bigger than a 16 KB flight slot (`cm_dynamic_display` about 70 KB).

Everything they do is already done by smaller modules that sit at the same
Pinball points, except at the points below. With the six composites dropped
from a try-list, the transfer engine stops at these points because no
remaining module accepts them. A head is a small routine that runs the
original program from one of those points to the next registered point.
With the four head families in the try-lists, the composites are no longer
needed in flight: each display path runs as a chain of per-label modules.

The heads do not change the composites. The composites stay as host test
oracles until the routine tables drop them (Lane C, `tools/lanec/routines.py`).

## Source map

| Source | Native | Boundary |
|---|---|---|
| DSPDCEND +2 (41,2570) `CCS DECOUNT`, `TC +2`, `TC ENTEXIT`, `TS DECOUNT`, `TC DSPDCPUT` | `lanec_pin_head_decount_run` | DSPDCPUT (Q = DSPDCEND +7) for a positive or negative count, ENTEXIT (Q = DSPDCEND +5) for +0, DSPDCPUT for -0 |
| DP1OUTSF, DP2OUTSF, DP3OUTSF (bank 40) through DPOUT, READLO, TPAGREE, DMP and SCOUTEND | `lanec_pin_head_dpout_run` | SCOUTEND's POSTJUMP to DSPDCEND |
| 2INTOUT +1 `TC +ON`, +2 `CA MPAC`, +4 `CS THREE`, +013 `TC POSTJUMP` (bank 40) | `lanec_pin_head_twoint_run` | the next stage of the two-integer display, or its POSTJUMP |
| M/SOUT (the fresh READLO), M/SNORM +1 `CAF HISECON`, the seconds display's BANKCALL, `CAF ZERO` after it, `TC SEPMIN`, the minutes display's BANKCALL (bank 42) | `lanec_pin_head_minsec_run` | the next stage of the minutes-seconds display |

The points, as physical words:

| Family | LM | CM |
|---|---|---|
| decount | 41,2570 | 41,2570 |
| dpout | 40,2710 2715 2717 | 40,2614 2621 2623 |
| twoint | 40,2733 2734 2736 2745 | 40,2637 2640 2642 2651 |
| minsec | 42,3303 3343 3347 3351 3361 3371 | 42,3216 3256 3262 3264 3274 3304 |

`dpout`, `twoint` and `minsec` run the stage code the composites already
use (`dsky_scale_output`, `cm_scale_output`, `dsky_two_integer`,
`dsky_minutes_output`), entered at the stage that starts at the point.
`dpout` refuses a superbank that does not select bank 40 with 0100 (bits 020
and 040 may be in any state; the code reads no other bit, and the superbank
is left as it was). `twoint` and `minsec` refuse unless every erasable word
is in its 15-bit memory form (`minsec` through `dsky_minutes_output`'s own
check). `minsec` also claims M/SOUT itself, so neither `dsky_minutes_output`
nor `cm_minutes_seconds` need to link in flight. Every head refuses outside its bank.

## Sizes

C64, one mission, LTO (`big.ld`; an empty program is 56 bytes):

| Head | LM | CM |
|---|---|---|
| decount | 701 | 701 |
| dpout | 5,721 | 6,007 |
| twoint | 8,599 | 8,616 |
| minsec | 14,900 | 14,911 |

`minsec` at -Oz, the flight store's setting: 13,656 bytes (LM) with M/SOUT
claimed, after dropping the head's duplicate 15-bit check (14,111 before).

All fit in a 16 KB flight slot.

## Chains

The chain check ran every accepted case of the six composites' suites through
the routine tables with the composites dropped and the heads in front. Every
Pinball-internal break closes. What is left is not a Pinball split gap:

- ENTEXIT's `TC` through the saved return word goes to the caller's return,
  outside the suite's range.
- `dsky_minutes_output` and `dsky_two_integer` run every stage, so they may
  run past a composite's stop; the image at the composite's stop is the same.
- Decimal display through LRPOSOUT (40,2635) reads channel 33, which the
  trampoline's device record (`lm_dntm_io`) does not carry yet.
- CM composites that stop outside Pinball (the Executive, the interpreter,
  DYNDISP in bank 24, REGODSP, DOWNFLAG): those points belong to other
  routine tables.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).

- Independent assembly comparison: `make test-lanec-pin-head`, one suite per
  family (`pin-head-decount`, `pin-head-dpout`, `pin-head-twoint`,
  `pin-head-minsec`, 8,192 cases each; `minsec` also takes the
  `pw-cm-minutes-seconds` cases at M/SOUT). Each family's cases are the accepted
  cases of the display consumers' suites, and of the suites of the modules
  it stands in for, that enter at one of its points. `decount` also varies
  DECOUNT (+0, 1, 2, -1, -0) and `dpout` the unused superbank bits. The
  oracle stops where the head stopped, with the same Q.
- Host and sanitizer results: `make test-lanec-pin-head sanitize-lanec-pin-head`.
- C64 execution results: the REU module harness (`tools/module_test.py`) once
  the routine tables carry the heads.
