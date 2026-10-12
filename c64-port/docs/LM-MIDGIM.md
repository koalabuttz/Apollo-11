# Translate Luminary MIDGIM and SELECTMU

Lane E. Milestone `lm-midgim`.

## Scope

From `Luminary099/P34-35_P74-75.agc`:

- MIDGIM (bank 06 `03715`-`03752`, 30 words): HALFREV, GET+MGA, SETMGA
  and GET.LVC. GET+MGA computes the positive middle gimbal angle of the
  delta V pushed at 0D (DOT with REFSMMAT's Y row, ARCSIN, a revolution
  added when negative) into +MGA and clears MGLVFLAG. GET.LVC builds the
  local-vertical matrix from RINIT and VINIT in 6D, converts the delta V
  with MXV into DELVLVC and sets MGLVFLAG.
- SELECTMU (bank 10 `02000`-`02027`, 24 words): picks the Earth or Moon
  index pair on CMOONFLG, loads RTSR1/MU and RTMU from MUTABLE, stores
  RTX1/RTX2, clears FINALFLG and ends at its `GOTO VN1645`.

That is 54 words on 53 source lines. VN1645 (bank 35) and the rest of the
file are not covered here.

`lm_midgim(state, io)` returns one of these values:

- 1 at the caller's NEWOPS frontier at C(QPRET), or for SELECTMU at the
  NEWOPS of VN1645 (`35,3606`).
- 3 at ARCSIN's argument-too-large frontier (`03723` in bank 0, alarm
  1301), reached by GET+MGA on raw edge words.
- 0 with the image unchanged, when the entry is outside the contract.

The UNIT abort (2) is part of the contract but no sampled entry reaches it.

Admitted entries must meet all of these conditions:

- The image is a canonical 2048-word image at the entry NEWOPS frontier
  (Z `06070`). EB, FB and BB are coherent, BANKSET = BB, INTBIT15, EDOP,
  ZERO and NEWJOB are zero, every word is 15 bits, MODE is `+0`, `+1` or
  vector, and the VAC is one of the five with PUSHLOC inside it.
- GET+MGA (`06,3717`): QPRET `073634` (its one Luminary call, `35,3632`)
  and PUSHLOC at least 6D past the VAC, above the pushed delta V.
- GET.LVC (`06,3733`): QPRET `072512`, `056334` or `056403` (`35,2510`,
  `27,2332`, `27,2401`).
- SELECTMU (`10,2000`): any QPRET (its P39/P79 callers branch to it and
  leave QPRET as it was).

The cases (`tests/lm_midgim_cases.c`) give GET+MGA a unit REFSMMAT
rotation and a delta V, sometimes along the stable-member Y axis (+-90
degrees); GET.LVC Earth and Moon RINIT/VINIT and a delta V; and SELECTMU
the flag words of the debris. Raw edge words appear on one case in eight.
All of this sits over poisoned erasable debris with every VAC, MODE,
OVFIND, IRQ setting and SUPERBNK value.

### How it is translated

Each routine is one native step (`lmg_getmga`, `lmg_getlvc`,
`lmg_selectmu`) compiled from the general Luminary interpretive bodies
(`src/lm_interp_orders.c`) and the dispatch paths of
`src/lm_interp_steps.c`. ARCSIN uses `inverse_trig`, UNIT `guidance_norm`,
and MXV runs the four `matrix_native_phase` phases on the 18 work-area
words at 6D. `matrix_native_phase` admits ADDRWD from `01000`; the 6D
matrix sits lower, so `lmg_getlvc` runs the phases on ADDRWD+`01000` and
takes that bias back out of ADDRWD (and of A after phases 1 and 2, which
leave the address sum there). The address arithmetic is small positive
additions, so the result is the original's, as the oracle stream confirms.
The milestone adds one shared body:

- `lmi_axc`: AXC `02351` (TAGSUB, CS POLISH, XSTORE), used by SELECTMU.

It passes `tools/lmi_body_differential.py` (body 61, 60000 cases), which
now has 62 bodies. HALFREV and MUTABLE are fixed operands read from the
certified fixed-data table.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_midgim_source.py`,
`evidence/lm-midgim/lm-midgim-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P34-35_P74-75.agc:1404 | 06,3715 | `HALFREV 2DEC 1 B-1` | 20000 00000 | `lmg_getmga` |
| P34-35_P74-75.agc:1406 | 06,3717 | `GET+MGA VLOAD UNIT` | 53575 | `lmg_getmga` |
| P34-35_P74-75.agc:1407 | 06,3720 | `UNIT` | 77656 | `lmg_getmga` |
| P34-35_P74-75.agc:1408 | 06,3721 | `DOT SL1` | 72441 | `lmg_getmga` |
| P34-35_P74-75.agc:1409 | 06,3722 | `REFSMMAT +6` | 01742 | `lmg_getmga` |
| P34-35_P74-75.agc:1410 | 06,3723 | `ARCSIN BPL` | 51136 | `lmg_getmga` |
| P34-35_P74-75.agc:1411 | 06,3724 | `SETMGA` | 15730 | `lmg_getmga` |
| P34-35_P74-75.agc:1412 | 06,3725 | `DAD DAD` | 43215 | `lmg_getmga` |
| P34-35_P74-75.agc:1413 | 06,3726 | `HALFREV` | 15716 | `lmg_getmga` |
| P34-35_P74-75.agc:1414 | 06,3727 | `HALFREV` | 15716 | `lmg_getmga` |
| P34-35_P74-75.agc:1415 | 06,3730 | `SETMGA STORE +MGA` | 02253 | `lmg_getmga` |
| P34-35_P74-75.agc:1416 | 06,3731 | `CLR RVQ` | 43414 | `lmg_getmga` |
| P34-35_P74-75.agc:1417 | 06,3732 | `MGLVFLAG` | 02675 | `lmg_getmga` |
| P34-35_P74-75.agc:1418 | 06,3733 | `GET.LVC VLOAD UNIT` | 53575 | `lmg_getlvc` |
| P34-35_P74-75.agc:1419 | 06,3734 | `RINIT` | 02323 | `lmg_getlvc` |
| P34-35_P74-75.agc:1420 | 06,3735 | `VCOMP` | 77676 | `lmg_getlvc` |
| P34-35_P74-75.agc:1421 | 06,3736 | `STORE 18D` | 00023 | `lmg_getlvc` |
| P34-35_P74-75.agc:1422 | 06,3737 | `VXV UNIT` | 53435 | `lmg_getlvc` |
| P34-35_P74-75.agc:1423 | 06,3740 | `VINIT` | 02331 | `lmg_getlvc` |
| P34-35_P74-75.agc:1424 | 06,3741 | `STORE 12D` | 00015 | `lmg_getlvc` |
| P34-35_P74-75.agc:1425 | 06,3742 | `VXV UNIT` | 53435 | `lmg_getlvc` |
| P34-35_P74-75.agc:1426 | 06,3743 | `18D` | 00023 | `lmg_getlvc` |
| P34-35_P74-75.agc:1427 | 06,3744 | `STOVL 6D` | 24007 | `lmg_getlvc` |
| P34-35_P74-75.agc:1428 | 06,3745 | `0D` | 00001 | `lmg_getlvc` |
| P34-35_P74-75.agc:1429 | 06,3746 | `MXV VSL1` | 76521 | `lmg_getlvc` |
| P34-35_P74-75.agc:1430 | 06,3747 | `6D` | 00007 | `lmg_getlvc` |
| P34-35_P74-75.agc:1431 | 06,3750 | `STORE DELVLVC` | 03434 | `lmg_getlvc` |
| P34-35_P74-75.agc:1432 | 06,3751 | `SET RVQ` | 43414 | `lmg_getlvc` |
| P34-35_P74-75.agc:1433 | 06,3752 | `MGLVFLAG` | 02475 | `lmg_getlvc` |
| P34-35_P74-75.agc:1443 | 10,2000 | `SELECTMU AXC,1 AXT,2` | 77160 | `lmg_selectmu` |
| P34-35_P74-75.agc:1444 | 10,2001 | `2D` | 00002 | `lmg_selectmu` |
| P34-35_P74-75.agc:1445 | 10,2002 | `0D` | 00000 | `lmg_selectmu` |
| P34-35_P74-75.agc:1446 | 10,2003 | `BOFF` | 77614 | `lmg_selectmu` |
| P34-35_P74-75.agc:1447 | 10,2004 | `CMOONFLG` | 04343 | `lmg_selectmu` |
| P34-35_P74-75.agc:1448 | 10,2005 | `SETMUER` | 20011 | `lmg_selectmu` |
| P34-35_P74-75.agc:1449 | 10,2006 | `AXC,1 AXT,2` | 77160 | `lmg_selectmu` |
| P34-35_P74-75.agc:1450 | 10,2007 | `10D` | 00012 | `lmg_selectmu` |
| P34-35_P74-75.agc:1451 | 10,2010 | `2D` | 00002 | `lmg_selectmu` |
| P34-35_P74-75.agc:1452 | 10,2011 | `SETMUER DLOAD* SXA,1` | 66143 | `lmg_selectmu` |
| P34-35_P74-75.agc:1453 | 10,2012 | `MUTABLE +4,1` | 10011 | `lmg_selectmu` |
| P34-35_P74-75.agc:1454 | 10,2013 | `RTX1` | 02776 | `lmg_selectmu` |
| P34-35_P74-75.agc:1455 | 10,2014 | `STODL* RTSR1/MU` | 22317 | `lmg_selectmu` |
| P34-35_P74-75.agc:1456 | 10,2015 | `MUTABLE -2,1` | 10003 | `lmg_selectmu` |
| P34-35_P74-75.agc:1457 | 10,2016 | `BOFF SR` | 54214 | `lmg_selectmu` |
| P34-35_P74-75.agc:1458 | 10,2017 | `CMOONFLG` | 04343 | `lmg_selectmu` |
| P34-35_P74-75.agc:1459 | 10,2020 | `RTRNMU` | 20022 | `lmg_selectmu` |
| P34-35_P74-75.agc:1460 | 10,2021 | `6D` | 20607 | `lmg_selectmu` |
| P34-35_P74-75.agc:1461 | 10,2022 | `RTRNMU STORE RTMU` | 02321 | `lmg_selectmu` |
| P34-35_P74-75.agc:1462 | 10,2023 | `SXA,2 CLEAR` | 43134 | `lmg_selectmu` |
| P34-35_P74-75.agc:1463 | 10,2024 | `RTX2` | 02777 | `lmg_selectmu` |
| P34-35_P74-75.agc:1464 | 10,2025 | `FINALFLG` | 01271 | `lmg_selectmu` |
| P34-35_P74-75.agc:1465 | 10,2026 | `GOTO` | 77650 | `lmg_selectmu` |
| P34-35_P74-75.agc:1466 | 10,2027 | `VN1645` | 73606 | `lmg_selectmu` |

## Evidence

Hashes are in `evidence/lm-midgim/SHA256.json`. Bulk files are assets of
the `evidence-lm-midgim` release (`python3 tools/evidence.py fetch
lm-midgim`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_midgim_source.py` reassembles the unchanged file with
  the pinned yaYUL. The rope equals the certified fixed-data rope. All 54
  words map to native literals in 3 functions.
- **Oracle.** `tools/lm_midgim_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant over 65536
  sampled entries. It stops at the caller's NEWOPS frontier (or VN1645's),
  the abort frontier or ARCSIN's alarm frontier.
  `tools/lm_midgim_differential.py` checks the native producer against
  every entry and pins the stream identity (`lm-midgim-source-stream.json`,
  SHA256 `76e33e3d...20b96d`, 269,090,816 bytes).
  - 63691 entries return and 1845 reach the ARCSIN 1301 frontier.
  - Every path is taken: GET+MGA's BPL both ways and the alarm, GET.LVC to
    each caller, SELECTMU with CMOONFLG clear and set.
- **Host and sanitizer results.** Run
  `make test-lm-midgim sanitize-lm-midgim`. The strict build and the
  ASAN/UBSAN build each pass 18 frozen entries, 26 admission guards, 18
  frozen phase steps and the full 65536-entry stream
  (`lm-midgim-checks.log`).
- **Phase frontiers.** `tools/freeze_lm_midgim_phases.py` requires each of
  the 36 native frontiers of the 18 frozen entries to equal, in order, an
  original NEWOPS image of the oracle. It then freezes 18 steps in three
  groups.
- **C64 execution results.** Run `make vice-lm-midgim`. Three PRGs each
  link one routine and keep both images in the RAM under KERNAL ROM
  (`$01=$35`). In VICE (PAL, stock ROMs) they pass 5, 5 and 8 steps with
  no failures (`lm-midgim-results.json`).

## Limits

- SELECTMU stops at VN1645, which is not translated. The rest of
  P34-35_P74-75 is not covered by this milestone.
- Numerical validity is the original's. No wider claim is made.
