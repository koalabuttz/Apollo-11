# Translate the Luminary apsides display subroutines (PERIAPO, SHIFTR1, MAXCHK)

Lane E. Milestone `lm-periapo`.

## Scope

The shared apsides display subroutines of the Luminary targeting programs:
from `Luminary099/P34-35_P74-75.agc`, RPAD, PERIAPO1, PERIAPO,
SETRAD/SETRADX, LEMSTORE/ACTIVE, CSMSTORE/PASSIVE, VECSHIFT and SHIFTR1
(bank 23 `02314`-`02357` and `02376`-`02431`), and from
`Luminary099/R30.agc`, MAXCHK and its constant MAXNM (`22,3636`-`3646`).
That is 73 assembled words on 71 source lines. PERIAPO's
`STCALL XXXALT APSIDES` runs the `lm-conic-timerad` milestone's APSIDES
steps (and through them PARAM/GEOM from `lm-conic-timethet`) unchanged.
PRECSET (`23,2360`-`2375`) is not translated here: it calls LEMPREC and
CSMPREC, which belong to INTEGRATION_INITIALIZATION (Lane B). Its callees
LEMSTORE and CSMSTORE are qualified from its two CALL returns.

`lm_periapo(state, io)` runs from the original interpretive entry frontier
(NEWOPS at the routine) to the caller's NEWOPS frontier at C(QPRET). It
returns 1 there, 2 at the UNIT/SQRT abort frontier (alarm 1302, from
SETRAD's ABVAL or APSIDES), 3 at ABTCONIC's POODOO frontier, or 0 with the
image unchanged when the entry is outside the contract. Admitted entries:

- canonical 2048-word image, coherent EB/FB/BB with BANKSET = BB, INTBIT15
  the FBANK's bit 15 (`040000` for banks 22 and 23), EDOP `+0`, all words 15
  bits, ZERO and NEWJOB zero, one of the five VACs and PUSHLOC within it;
- NEWOPS at the routine with QPRET one of its Luminary call returns:
  PERIAPO1 `070043`, `070444`, `071473`, `072517`, `072706`; PERIAPO
  `070254`; LEMSTORE `046366` and CSMSTORE `046374` (PRECSET); VECSHIFT
  `070241`, `070356`, `071164`; SHIFTR1 at its eleven call returns; MAXCHK
  `045600`, `045612` (R30), `070047`, `070055`, `072523`, `072531`. The
  callers are in P30_P37, GENERAL_LAMBERT_AIMPOINT_GUIDANCE, P32-P35_P72-P75,
  P34-35_P74-75, STABLE_ORBIT and R30 (each return is the word after the
  caller's CADR in the assembly listing);
- MODE vector where the routine starts on a vector in MPAC (PERIAPO1,
  VECSHIFT) and double precision where it starts on a scalar (SHIFTR1,
  MAXCHK); PUSHLOC at least six words into the VAC where the routine pops
  the position vector;
- RTX1/RTX2 the Earth (`-2`, `0`) or Moon (`-10`, `2`) pair where PERIAPO1
  or VECSHIFT loads them, RTX2 0 or 2 for SHIFTR1, and X1 `-2` or `-10` on
  entry to PERIAPO (APSIDES' contract).

The cases (`tests/lm_periapo_cases.c`) give each routine Earth or Moon
conics (speeds from well below circular to hyperbolic, sometimes exactly
circular, random planes), a lunar landing site in RLS, altitudes around
MAXNM for MAXCHK, AVFLAG either way, and raw edge words on one case in
eight, over poisoned erasable debris with every VAC, PUSHLOC, OVFIND,
IRQ setting and SUPERBNK 0 or 3.

### How it is translated

Every interpretive pair is compiled with the general Luminary interpretive
bodies (`src/lm_interp_orders.c`) and the dispatch paths of
`src/lm_interp_steps.c`; no new bodies were needed. LXA, SXA and INCR are
used from compiled code for the first time (they are qualified by
`tools/lmi_body_differential.py`). RPAD and MAXNM appear as literal words.
The routines are ten steps between original NEWOPS frontiers
(`enum lm_periapo_phase`, `lpa_*`); APSIDES keeps its own phase numbers and
`lpa_remap` follows its `GOTO RTNAPSE` back to `23,2333`.

INTBIT15 matters here for the first time in the lane's images: the fixed
operands of banks 22 and 23 (RPAD, MAXNM) are read through INTBIT15, and an
entry image with it clear would read bank 0 or 2. The admission requires it
to match the FBANK, as the interpreter's INTPRET entry leaves it.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_periapo_source.py`,
`evidence/lm-periapo/lm-periapo-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P34-35_P74-75.agc:1527 | 23,2314 | `RPAD 2DEC 6373338 B-29` | 00302 17755 | `lpa_setrad` |
| P34-35_P74-75.agc:1530 | 23,2316 | `PERIAPO1 LXA,2 VSR*` | 53754 | `lpa_periapo1` |
| P34-35_P74-75.agc:1531 | 23,2317 | `RTX2` | 02777 | `lpa_periapo1` |
| P34-35_P74-75.agc:1532 | 23,2320 | `0,2` | 57176 | `lpa_periapo1` |
| P34-35_P74-75.agc:1533 | 23,2321 | `STOVL VVEC` | 26744 | `lpa_periapo1` |
| P34-35_P74-75.agc:1534 | 23,2322 | `LXA,1 VSR*` | 53750 | `lpa_periapo1` |
| P34-35_P74-75.agc:1535 | 23,2323 | `RTX1` | 02776 | `lpa_periapo1` |
| P34-35_P74-75.agc:1536 | 23,2324 | `0,2` | 57176 | `lpa_periapo1` |
| P34-35_P74-75.agc:1537 | 23,2325 | `STORE RVEC` | 02655 | `lpa_periapo1` |
| P34-35_P74-75.agc:1538 | 23,2326 | `PERIAPO STQ CALL` | 45020 | `lpa_periapo` |
| P34-35_P74-75.agc:1539 | 23,2327 | `NORMEX` | 03463 | `lpa_periapo` |
| P34-35_P74-75.agc:1540 | 23,2330 | `SETRAD` | 46344 | `lpa_periapo` |
| P34-35_P74-75.agc:1541 | 23,2331 | `STCALL XXXALT` | 37671 | `lpa_aftersetrad` |
| P34-35_P74-75.agc:1542 | 23,2332 | `APSIDES` | 25674 | `lpa_aftersetrad` |
| P34-35_P74-75.agc:1543 | 23,2333 | `SETPD PUSH` | 41401 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1544 | 23,2334 | `2D` | 00003 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1545 | 23,2335 | `DSU PDDL` | 65225 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1546 | 23,2336 | `XXXALT` | 03671 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1547 | 23,2337 | `0D` | 00001 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1548 | 23,2340 | `PUSH DSU` | 45206 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1549 | 23,2341 | `XXXALT` | 03671 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1550 | 23,2342 | `PUSH GOTO` | 52006 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1551 | 23,2343 | `NORMEX` | 03463 | `lpa_afterapsides` |
| P34-35_P74-75.agc:1554 | 23,2344 | `SETRAD DLOAD PUSH` | 41545 | `lpa_setrad` |
| P34-35_P74-75.agc:1555 | 23,2345 | `RPAD` | 06315 | `lpa_setrad` |
| P34-35_P74-75.agc:1556 | 23,2346 | `SXA,1 INCR,2` | 63130 | `lpa_setrad` |
| P34-35_P74-75.agc:1557 | 23,2347 | `X2` | 00047 | `lpa_setrad` |
| P34-35_P74-75.agc:1558 | 23,2350 | `2D` | 00002 | `lpa_setrad` |
| P34-35_P74-75.agc:1559 | 23,2351 | `SLOAD BHIZ` | 46135 | `lpa_setrad` |
| P34-35_P74-75.agc:1560 | 23,2352 | `X2` | 00050 | `lpa_setrad` |
| P34-35_P74-75.agc:1561 | 23,2353 | `SETRADX` | 46357 | `lpa_setrad` |
| P34-35_P74-75.agc:1562 | 23,2354 | `VLOAD ABVAL` | 51575 | `lpa_setrad` |
| P34-35_P74-75.agc:1563 | 23,2355 | `RLS` | 02023 | `lpa_setrad` |
| P34-35_P74-75.agc:1564 | 23,2356 | `PDDL` | 77725 | `lpa_setrad` |
| P34-35_P74-75.agc:1565 | 23,2357 | `SETRADX DLOAD RVQ` | 43545 | `lpa_setrad` |
| P34-35_P74-75.agc:1582 | 23,2376 | `LEMSTORE VLOAD BOFF` | 43175 | `lpa_lemstore` |
| P34-35_P74-75.agc:1583 | 23,2377 | `RATT` | 00001 | `lpa_lemstore` |
| P34-35_P74-75.agc:1584 | 23,2400 | `AVFLAG` | 01352 | `lpa_lemstore` |
| P34-35_P74-75.agc:1585 | 23,2401 | `PASSIVE` | 46412 | `lpa_lemstore` |
| P34-35_P74-75.agc:1586 | 23,2402 | `ACTIVE STOVL RACT3` | 27536 | `lpa_lemstore` |
| P34-35_P74-75.agc:1587 | 23,2403 | `VATT` | 00007 | `lpa_lemstore` |
| P34-35_P74-75.agc:1588 | 23,2404 | `STORE VACT3` | 03544 | `lpa_lemstore` |
| P34-35_P74-75.agc:1589 | 23,2405 | `RVQ` | 77616 | `lpa_lemstore` |
| P34-35_P74-75.agc:1590 | 23,2406 | `CSMSTORE VLOAD BOFF` | 43175 | `lpa_csmstore` |
| P34-35_P74-75.agc:1591 | 23,2407 | `RATT` | 00001 | `lpa_csmstore` |
| P34-35_P74-75.agc:1592 | 23,2410 | `AVFLAG` | 01352 | `lpa_csmstore` |
| P34-35_P74-75.agc:1593 | 23,2411 | `ACTIVE` | 46402 | `lpa_csmstore` |
| P34-35_P74-75.agc:1594 | 23,2412 | `PASSIVE STOVL RPASS3` | 27552 | `lpa_csmstore` |
| P34-35_P74-75.agc:1595 | 23,2413 | `VATT` | 00007 | `lpa_csmstore` |
| P34-35_P74-75.agc:1596 | 23,2414 | `STORE VPASS3` | 03560 | `lpa_csmstore` |
| P34-35_P74-75.agc:1597 | 23,2415 | `RVQ` | 77616 | `lpa_csmstore` |
| P34-35_P74-75.agc:1600 | 23,2416 | `VECSHIFT LXA,2 VSR*` | 53754 | `lpa_vecshift` |
| P34-35_P74-75.agc:1601 | 23,2417 | `RTX2` | 02777 | `lpa_vecshift` |
| P34-35_P74-75.agc:1602 | 23,2420 | `0,2` | 57176 | `lpa_vecshift` |
| P34-35_P74-75.agc:1603 | 23,2421 | `LXA,1 PDVL` | 63350 | `lpa_vecshift` |
| P34-35_P74-75.agc:1604 | 23,2422 | `RTX1` | 02776 | `lpa_vecshift` |
| P34-35_P74-75.agc:1605 | 23,2423 | `VSR* PDVL` | 63257 | `lpa_vecshift` |
| P34-35_P74-75.agc:1606 | 23,2424 | `0,2` | 57176 | `lpa_vecshift` |
| P34-35_P74-75.agc:1607 | 23,2425 | `RVQ` | 77616 | `lpa_vecshift` |
| P34-35_P74-75.agc:1610 | 23,2426 | `SHIFTR1 LXA,2 SL*` | 53754 | `lpa_shiftr1` |
| P34-35_P74-75.agc:1611 | 23,2427 | `RTX2` | 02777 | `lpa_shiftr1` |
| P34-35_P74-75.agc:1612 | 23,2430 | `0,2` | 57576 | `lpa_shiftr1` |
| P34-35_P74-75.agc:1613 | 23,2431 | `RVQ` | 77616 | `lpa_shiftr1` |
| R30.agc:454 | 22,3636 | `MAXCHK DSU BPL` | 51025 | `lpa_maxchk` |
| R30.agc:455 | 22,3637 | `MAXNM` | 05646 | `lpa_maxchk` |
| R30.agc:456 | 22,3640 | `+3` | 45643 | `lpa_maxchk` |
| R30.agc:457 | 22,3641 | `DAD RVQ` | 43415 | `lpa_maxchk` |
| R30.agc:458 | 22,3642 | `MAXNM` | 05646 | `lpa_maxchk` |
| R30.agc:459 | 22,3643 | `+3 DLOAD RVQ` | 43545 | `lpa_maxchk` |
| R30.agc:460 | 22,3644 | `MAXNM` | 05646 | `lpa_maxchk` |
| R30.agc:462 | 22,3645 | `MAXNM 2OCT 0106505603` | 01065 05603 | `lpa_maxchk` |

## Evidence

Hashes: `evidence/lm-periapo/SHA256.json`. Bulk files are assets of the
`evidence-lm-periapo` release (`python3 tools/evidence.py fetch lm-periapo`).

- Source-to-native map and independent assembly comparison:
  `python3 tools/lm_periapo_source.py` reassembles the unchanged files with
  the pinned yaYUL (rope equal to the certified fixed-data rope) and maps all
  73 words of the ranges to native literals in 10 functions.
- Oracle: `tools/lm_periapo_reference.c` (GPL, host only) runs the unchanged
  Luminary099 in the pinned yaAGC hardware-DV variant over 65536 sampled
  entries, with both IRQ settings, SUPERBNK, EBANK, all five VACs, PUSHLOC,
  MODE and OVFIND, until the caller's NEWOPS frontier named by the entry
  QPRET. `tools/lm_periapo_differential.py` checks the native producer
  against every entry and pins the stream identity
  (`lm-periapo-source-stream.json`, SHA256 `36536a94...585305`, 269,090,816
  bytes). The routines return on 63819 entries; 1717 PERIAPO1/PERIAPO
  entries reach APSIDES' SQRT abort (alarm 1302). Every phase transition and
  both sides of each runtime branch (SETRAD's BHIZ, MAXCHK's BPL, LEMSTORE's
  and CSMSTORE's BOFF, GETECC's INFINAPO) are taken.
- Host and sanitizer results: `make test-lm-periapo sanitize-lm-periapo`.
  Strict and ASAN/UBSAN builds each pass 20 frozen entries, 30 admission
  guards, 63 frozen phase steps and the full 65536-entry stream
  (`lm-periapo-checks.log`).
- Phase frontiers: `tools/freeze_lm_periapo_phases.py` requires each of the
  118 native frontiers of the 20 frozen entries to equal, in order, an
  original NEWOPS image of the oracle, then freezes 63 steps in four groups.
- C64 execution results: `make vice-lm-periapo`. Four PRGs each link one
  phase group and keep both images in the RAM under KERNAL ROM (`$01=$35`),
  starting each step from a debris-only base state. In VICE (PAL, stock
  ROMs) they pass 31, 8, 8 and 16 steps with no failures
  (`lm-periapo-results.json`).

## Limits

- The C64 check runs each phase separately; the whole chains run on the host.
- PRECSET is not translated (it needs LEMPREC/CSMPREC); the rest of both
  files is not covered by this milestone.
- Numerical validity is the original's; no widened claim.
