# Translate Luminary time-radius and apsides subroutines (TIMERAD, APSIDES)

Lane B. Milestone `lm-conic-timerad`.

## Scope

The time-radius and apsides subroutines of
`Luminary099/CONIC_SUBROUTINES.agc`: bank 12 `03547`-`03741`, 123 assembled
words on 123 source lines. That is TIMERAD with TERMNVEC, CIRCULAR and BADR2
(`03547`-`03673`) and APSIDES with GETECC and INFINAPO (`03674`-`03741`).
Both call PARAM/GEOM, and TIMERAD also calls GETX and goes on through
COMMNOUT and DELTIME. Those shared steps belong to the `lm-conic-timethet` and
`lm-conic-kepler` milestones and are reused here unchanged. ITERATOR and
LAMBERT are not yet ported, so the file stays `partial`.

`lm_conic_timerad(state, io)` runs from the original interpretive entry
frontier (NEWOPS, FBANK 12, LOC `03547` for TIMERAD or `03674` for APSIDES)
to the caller's NEWOPS frontier at C(RTNTR) or C(RTNAPSE). It returns 1
there, 3 at ABTCONIC's POODOO frontier (alarm 607), 2 at the UNIT/SQRT abort
frontier (alarm 1302), or 0 with the image unchanged when the entry is
outside the contract. Admitted entries:

- canonical 2048-word image, coherent EB/FB/BB with BANKSET = BB, INTBIT15 and
  EDOP `+0`, all words 15-bit, Z and NEWJOB zero, one of the five VACs, and
  X1 `-2` or `-10` (RTX1);
- TIMERAD with QPRET `072106`, the return of P32/P72 CSI's
  `STCALL RDESIRED TIMERAD` at `35,2104`, which sets RVSW first (so TIMERAD
  never reaches NEWSTATE);
- APSIDES with QPRET `046333`, the return of PERIAPO's
  `STCALL XXXALT APSIDES` at `23,2331`.

CSI passes RDESIRED = DPPOSMAX, which always takes BADR2 (the apocenter
time). The cases also sample radii inside, at the edges of and outside the
orbit's range, both SGNRDOT signs, and circular orbits. That way every TIMERAD
path is exercised, not only the one CSI uses.

### How it is translated

Every interpretive pair is compiled with the general Luminary interpretive
bodies (`src/lm_interp_orders.c`) and the dispatch paths of
`src/lm_interp_steps.c`. No new bodies were needed (VSU, at `06747`, was
already covered). Each call passes its source line's order and operand words
as literals. LODPHALF, KEPZERO and LDPOSMAX come from the certified
fixed-data table. The D1/n constants and DP1/4 (ORBITAL_INTEGRATION
`12,3755`, not yet certified) are native literals.

The routines are split into 9 new steps between original NEWOPS frontiers
(`enum lm_conic_timerad_phase`, `ltr_*`). They run with the time-theta
milestone's shared steps, which keep their `lm_conic_timethet_phase` numbers.
`ltr_remap` follows PARAM's `GOTO RTNPRM` and GETX's `RVQ` to the return
address in LOC: TIMERAD `03561`/`03655`, APSIDES `03706`. The image carries
every continuation. The longest sampled entry runs 27 steps.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_conic_timerad_source.py`,
`evidence/lm-conic-timerad/lm-conic-timerad-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| 1597 | 12,3547 | `TIMERAD STQ SETPD` | 40220 | `ltr_timerad` |
| 1598 | 12,3550 | `RTNTR` | 02710 | `ltr_timerad` |
| 1599 | 12,3551 | `0` | 00001 | `ltr_timerad` |
| 1600 | 12,3552 | `BOV` | 77600 | `ltr_timerad` |
| 1601 | 12,3553 | `+1` | 25554 | `ltr_timerad` |
| 1602 | 12,3554 | `VLOAD PDVL` | 63375 | `ltr_timerad` |
| 1603 | 12,3555 | `RVEC` | 02655 | `ltr_timerad` |
| 1604 | 12,3556 | `VVEC` | 02744 | `ltr_timerad` |
| 1605 | 12,3557 | `CALL` | 77624 | `ltr_timerad` |
| 1606 | 12,3560 | `PARAM` | 11064 | `ltr_timerad` |
| 1607 | 12,3561 | `BOV DLOAD` | 71200 | `ltr_afterparam` |
| 1608 | 12,3562 | `COGAOVFL` | 24764 | `ltr_afterparam` |
| 1609 | 12,3563 | `D1/32` | 11043 | `ltr_afterparam` |
| 1610 | 12,3564 | `DSU DMP` | 41225 | `ltr_afterparam` |
| 1611 | 12,3565 | `R1A` | 02742 | `ltr_afterparam` |
| 1612 | 12,3566 | `P` | 02740 | `ltr_afterparam` |
| 1613 | 12,3567 | `SQRT DMP` | 41366 | `ltr_afterparam` |
| 1614 | 12,3570 | `COGA` | 02766 | `ltr_afterparam` |
| 1615 | 12,3571 | `SL4 VXSC` | 74212 | `ltr_afterparam` |
| 1616 | 12,3572 | `U2` | 02712 | `ltr_afterparam` |
| 1617 | 12,3573 | `PDDL DSU` | 45325 | `ltr_afterparam` |
| 1618 | 12,3574 | `D1/64` | 11035 | `ltr_afterparam` |
| 1619 | 12,3575 | `R1A` | 02742 | `ltr_afterparam` |
| 1620 | 12,3576 | `VXSC VSU` | 52361 | `ltr_afterparam` |
| 1621 | 12,3577 | `UR1` | 02722 | `ltr_afterparam` |
| 1622 | 12,3600 | `VSL4 UNIT` | 53512 | `ltr_afterparam` |
| 1623 | 12,3601 | `BOV` | 77600 | `ltr_afterparam` |
| 1624 | 12,3602 | `CIRCULAR` | 25660 | `ltr_afterparam` |
| 1625 | 12,3603 | `PDDL NORM` | 60325 | `ltr_afterparam` |
| 1626 | 12,3604 | `RDESIRED` | 02756 | `ltr_afterparam` |
| 1627 | 12,3605 | `X1` | 00047 | `ltr_afterparam` |
| 1628 | 12,3606 | `PDDL DMP` | 41325 | `ltr_afterparam` |
| 1629 | 12,3607 | `R1` | 00041 | `ltr_afterparam` |
| 1630 | 12,3610 | `P` | 02740 | `ltr_afterparam` |
| 1631 | 12,3611 | `SL* DDV` | 56257 | `ltr_afterparam` |
| 1632 | 12,3612 | `0,1` | 20201 | `ltr_afterparam` |
| 1633 | 12,3613 | `DSU DDV` | 56225 | `ltr_afterparam` |
| 1634 | 12,3614 | `D1/16` | 11041 | `ltr_afterparam` |
| 1635 | 12,3615 | `36D` | 00045 | `ltr_afterparam` |
| 1636 | 12,3616 | `STORE COSF` | 00031 | `ltr_afterparam` |
| 1637 | 12,3617 | `BOV DSQ` | 63400 | `ltr_afterparam` |
| 1638 | 12,3620 | `BADR2` | 25664 | `ltr_afterparam` |
| 1639 | 12,3621 | `BDSU BMN` | 50021 | `ltr_afterparam` |
| 1640 | 12,3622 | `D1/4` | 11037 | `ltr_afterparam` |
| 1641 | 12,3623 | `BADR2` | 25664 | `ltr_afterparam` |
| 1642 | 12,3624 | `SQRT SIGN` | 75366 | `ltr_afterparam` |
| 1643 | 12,3625 | `SGNRDOT` | 02755 | `ltr_afterparam` |
| 1644 | 12,3626 | `CLEAR` | 77614 | `ltr_afterparam` |
| 1645 | 12,3627 | `APSESW` | 04272 | `ltr_afterparam` |
| 1647 | 12,3630 | `TERMNVEC VXSC VSL1` | 76561 | `ltr_termnvec` |
| 1648 | 12,3631 | `UN` | 02674 | `ltr_termnvec` |
| 1649 | 12,3632 | `VXV PDVL` | 63235 | `ltr_termnvec` |
| 1650 | 12,3633 | `0D` | 00001 | `ltr_termnvec` |
| 1651 | 12,3634 | `VXSC VAD` | 53361 | `ltr_termnvec` |
| 1652 | 12,3635 | `COSF` | 00031 | `ltr_termnvec` |
| 1653 | 12,3636 | `VSL1 PUSH` | 41572 | `ltr_termnvec` |
| 1655 | 12,3637 | `DOT DDV` | 56241 | `ltr_termnvec` |
| 1656 | 12,3640 | `UR1` | 02722 | `ltr_termnvec` |
| 1657 | 12,3641 | `DP1/4` | 25756 | `ltr_termnvec` |
| 1658 | 12,3642 | `SR1 BOV` | 40142 | `ltr_termnvec` |
| 1659 | 12,3643 | `+1` | 25644 | `ltr_termnvec` |
| 1660 | 12,3644 | `STOVL CSTH` | 26732 | `ltr_termnvec` |
| 1661 | 12,3645 | `UR1` | 02722 | `ltr_termnvec` |
| 1662 | 12,3646 | `VXV VSL1` | 76435 | `ltr_termnvec` |
| 1663 | 12,3647 | `DOT SL1` | 72441 | `ltr_termnvec` |
| 1664 | 12,3650 | `UN` | 02674 | `ltr_termnvec` |
| 1665 | 12,3651 | `STODL SNTH` | 16730 | `ltr_termnvec` |
| 1666 | 12,3652 | `P` | 02740 | `ltr_termnvec` |
| 1667 | 12,3653 | `CALL` | 77624 | `ltr_termnvec` |
| 1668 | 12,3654 | `GETX` | 24767 | `ltr_termnvec` |
| 1669 | 12,3655 | `CLRGO` | 77614 | `ltr_soln` |
| 1670 | 12,3656 | `SOLNSW` | 02634 | `ltr_soln` |
| 1671 | 12,3657 | `COMMNOUT` | 24747 | `ltr_soln` |
| 1673 | 12,3660 | `CIRCULAR SETPD SETGO` | 43001 | `ltr_circular` |
| 1674 | 12,3661 | `0` | 00001 | `ltr_circular` |
| 1675 | 12,3662 | `SOLNSW` | 02434 | `ltr_circular` |
| 1676 | 12,3663 | `ABTCONIC` | 25742 | `ltr_circular` |
| 1678 | 12,3664 | `BADR2 DLOAD SIGN` | 75345 | `ltr_badr2` |
| 1679 | 12,3665 | `LODPHALF` | 24005 | `ltr_badr2` |
| 1680 | 12,3666 | `COSF` | 00031 | `ltr_badr2` |
| 1681 | 12,3667 | `STODL COSF` | 14031 | `ltr_badr2` |
| 1682 | 12,3670 | `KEPZERO` | 24007 | `ltr_badr2` |
| 1683 | 12,3671 | `SETGO` | 77614 | `ltr_badr2` |
| 1684 | 12,3672 | `APSESW` | 04032 | `ltr_badr2` |
| 1685 | 12,3673 | `TERMNVEC` | 25630 | `ltr_badr2` |
| 1688 | 12,3674 | `APSIDES STQ SETPD` | 40220 | `ltr_apsides` |
| 1689 | 12,3675 | `RTNAPSE` | 02710 | `ltr_apsides` |
| 1690 | 12,3676 | `0D` | 00001 | `ltr_apsides` |
| 1691 | 12,3677 | `BOV` | 77600 | `ltr_apsides` |
| 1692 | 12,3700 | `+1` | 25701 | `ltr_apsides` |
| 1693 | 12,3701 | `VLOAD PDVL` | 63375 | `ltr_apsides` |
| 1694 | 12,3702 | `RVEC` | 02655 | `ltr_apsides` |
| 1695 | 12,3703 | `VVEC` | 02744 | `ltr_apsides` |
| 1696 | 12,3704 | `CALL` | 77624 | `ltr_apsides` |
| 1697 | 12,3705 | `PARAM` | 11064 | `ltr_apsides` |
| 1698 | 12,3706 | `BOV` | 77600 | `ltr_getecc` |
| 1699 | 12,3707 | `GETECC` | 25710 | `ltr_getecc` |
| 1700 | 12,3710 | `GETECC DMP SL4` | 42405 | `ltr_getecc` |
| 1701 | 12,3711 | `R1A` | 02742 | `ltr_getecc` |
| 1702 | 12,3712 | `BDSU SQRT` | 75421 | `ltr_getecc` |
| 1703 | 12,3713 | `D1/64` | 11035 | `ltr_getecc` |
| 1704 | 12,3714 | `STORE ECC` | 02752 | `ltr_getecc` |
| 1705 | 12,3715 | `DAD PDDL` | 65215 | `ltr_getecc` |
| 1706 | 12,3716 | `D1/8` | 11031 | `ltr_getecc` |
| 1707 | 12,3717 | `R1` | 00041 | `ltr_getecc` |
| 1708 | 12,3720 | `DMP SL1` | 72405 | `ltr_getecc` |
| 1709 | 12,3721 | `P` | 02740 | `ltr_getecc` |
| 1710 | 12,3722 | `DDV` | 77671 | `ltr_getecc` |
| 1711 | 12,3723 | `PDDL NORM` | 60325 | `ltr_getecc` |
| 1712 | 12,3724 | `R1A` | 02742 | `ltr_getecc` |
| 1713 | 12,3725 | `X1` | 00047 | `ltr_getecc` |
| 1714 | 12,3726 | `PDDL SL*` | 53725 | `ltr_getecc` |
| 1715 | 12,3727 | `R1` | 00041 | `ltr_getecc` |
| 1716 | 12,3730 | `0 -5,1` | 20174 | `ltr_getecc` |
| 1717 | 12,3731 | `DDV DSU` | 45271 | `ltr_getecc` |
| 1718 | 12,3732 | `BOV BMN` | 50000 | `ltr_getecc` |
| 1719 | 12,3733 | `INFINAPO` | 25737 | `ltr_getecc` |
| 1720 | 12,3734 | `INFINAPO` | 25737 | `ltr_getecc` |
| 1721 | 12,3735 | `GOTO` | 77650 | `ltr_getecc` |
| 1722 | 12,3736 | `RTNAPSE` | 02710 | `ltr_getecc` |
| 1723 | 12,3737 | `INFINAPO DLOAD GOTO` | 52145 | `ltr_infinapo` |
| 1724 | 12,3740 | `LDPOSMAX` | 24020 | `ltr_infinapo` |
| 1725 | 12,3741 | `RTNAPSE` | 02710 | `ltr_infinapo` |

## Evidence

Hashes: `evidence/lm-conic-timerad/SHA256.json`. Bulk files are assets of the
`evidence-lm-conic-timerad` release
(`python3 tools/evidence.py fetch lm-conic-timerad`).

- Source-to-native map and independent assembly comparison:
  `python3 tools/lm_conic_timerad_source.py` reassembles the unchanged file
  with the pinned yaYUL (rope equal to the certified fixed-data rope) and maps
  all 123 words of the range to native literals in 9 functions.
- Oracle: `tools/lm_conic_timerad_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant over 65536
  sampled entries (`tests/lm_conic_timerad_cases.c`, TIMERAD for even and
  APSIDES for odd cases). The samples cover Earth and Moon state vectors with
  speeds from well below circular to hyperbolic, near-circular orbits, random
  planes and flight-path angles, and RDESIRED at DPPOSMAX or at sampled radii.
  One case in eight uses raw edge words. They also cover both IRQ settings,
  SUPER, EBANK, all five VACs, PUSHLOC, MODE and OVFIND.
  `tools/lm_conic_timerad_differential.py` checks the native producer
  against every entry and pins the stream identity
  (`lm-conic-timerad-source-stream.json`, SHA256 `6207be44...4bf266`,
  269,090,816 bytes). TIMERAD returns on 25615 entries and ends at ABTCONIC
  on 7153. APSIDES returns on 32197 and reaches the 1302 abort at GETECC's
  SQRT on 571. All 46 phase transitions taken are frozen.
- Host and sanitizer results: `make test-lm-conic-timerad
  sanitize-lm-conic-timerad`. Strict and ASAN/UBSAN builds each pass 12
  frozen entries, 25 admission guards, 108 frozen phase steps and the full
  65536-entry stream (`lm-conic-timerad-checks.log`).
- Phase frontiers: `tools/freeze_lm_conic_timerad_phases.py` requires each of
  the 180 native frontiers of the 12 frozen entries (which together take all
  46 transitions) to equal, in order, an original NEWOPS image of the oracle,
  then freezes 108 steps in six groups.
- C64 execution results: `make vice-lm-conic-timerad`. Six PRGs each link one
  phase group and keep both images in the RAM under KERNAL ROM (`$01=$35`),
  starting each step from a debris-only base state. In VICE (PAL, stock ROMs)
  they pass 28, 9, 13, 17, 21 and 20 steps with no failures
  (`lm-conic-timerad-results.json`).

## Limits

- The C64 check runs each phase separately; the whole chains run on the host.
- Qualified from the one TIMERAD and one APSIDES call site. NEWSTATE is not
  reached from TIMERAD, because CSI sets RVSW.
- Numerical validity is the original's; no widened claim.
