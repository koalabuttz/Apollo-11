# Translate Luminary Kepler subroutine (KEPLERN, DELTIME, CHECKCTR)

Lane B. Milestone `lm-conic-kepler`.

## Scope

The Kepler subroutine of `Luminary099/CONIC_SUBROUTINES.agc`: bank 12
`02023`-`02535` and `02644`-`02652`, 338 assembled words on
318 source lines. That is KEPLERN `02023` through KEPCONVG, the
time routine DELTIME `02421` (with the 21-word S(XI) and C(XI) POLY tables it
runs in basic code) and the iteration counter CHECKCTR `02644`. The rest of the
file (ITERATOR, NEWSTATE/LAMENTER, TIMETHET, TIMERAD, APSIDES, LAMBERT, GETX
and the conic constants in bank 4) is not yet ported, so the file stays
`partial`.

`lm_conic_kepler(state, io)` runs from the original interpretive entry frontier
that KEPPREP (`ORBITAL_INTEGRATION`, `11,2376`) leaves with its
`GOTO KEPLERN` (NEWOPS, LOC `02023`, FBANK 12) to the caller's NEWOPS frontier at C(KEPRTN). It returns 1, or 0 with
the image unchanged when the entry is outside the contract. Admitted entries:

- canonical 2048-word image, coherent EB/FB/BB with BANKSET = BB, INTBIT15 and
  EDOP `+0`, all words 15-bit, Z and NEWJOB zero, one of the five VACs;
- X1 `-2` (Earth) or `-10` (Moon, MOONFLAG), as KEPPREP's `AXC,1` sets it;
- KEPRTN (`E4,1512`) one of the returns KEPPREP's `STQ` stores for its three
  callers: `027252` (`CALL KEPPREP` at `13,3250`, INTEGRATION_INITIALIZATION),
  `022516` and `023012` (`STCALL TET KEPPREP` at `11,2514` and `11,3010`,
  ORBITAL_INTEGRATION). The cases sample all three.

The routine also has an original UNIT/SQRT abort frontier (alarm 1302), and the
native step returns 2 there, but no admitted entry reaches it: UNIT's SQRT gets
a VSQ result, and the two SQRT orders only see `-50SC/ALPHA` with ALPHA
negative or ALPHA itself after `BPL`. All 65536 sampled entries return.

### How it is translated

Every interpretive pair is compiled. The routine uses the general Luminary
interpretive bodies (`src/lm_interp_orders.c`) and the dispatch paths of
`src/lm_interp_steps.c` (Lane A). It needed 18 general bodies the TFF
milestone did not: VAD, VSU, BVSU, TAD, ROUND, SSP, XAD, XSU, LXA, SXA, BHIZ,
CALL, indirect GOTO, EXIT, INTPRET, TPMODE, 15BITADR (STCALL) and BZE/GOTO.
Each is checked against yaAGC with the earlier bodies by
`tools/lmi_body_differential.py` (56 bodies, 2000 cases each when added).
Each call passes the order and operand words of its own source line as
literals; nothing is fetched from code memory. MUTABLE (bank 4), KEPZERO and
the fixed-fixed ONE come from the certified fixed-data table; D1/64, D1/128,
D1/256, 2PISC, -50SC, 0.9, BEE22 and the two POLY tables are native literals.

The routine is split into 23 steps between original NEWOPS frontiers
(`enum lm_conic_kepler_phase`, `lm_conic_kepler_step`, and the `lck_*` entry
points). The image carries every continuation (LOC, X1/X2, ITERCTR, QPRET and
the push-down list), so each step can become its own overlay module. The
longest sampled entry runs 1076 steps (PERIODCH reductions, then the full 20
iterations); the driver's guard is 4000.

Original details the translation reproduces, found by stepping the oracle:
`STCALL XI DELTIME` stores through STOTC and 15BITADR into CALL (CYR edited to
`015`); DELTIME leaves the interpreter with EXIT, runs POLY in basic code and
re-enters with `TC INTPRET` twice, so its steps restore the IRQ enable that
INTPRET sets; CHECKCTR counts ITERCTR down through `INDEX FIXLOC` and returns
to the RTB BHIZ through DANZIG; the time overflow paths (TIMEOVFL, NEGTOVFL,
CMNTOVFL) can return directly through CMNTOVFL's `BZE KEPRTN`, which the cases
reach.

### Interpreter ABI

The 18 bodies above are new shared interpreter code in `lm_interp_orders`.
Lane A owns the interpreter ABI and should review them; they add no runtime
entry points.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_conic_kepler_source.py`,
`evidence/lm-conic-kepler/lm-conic-kepler-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| 597 | 12,2023 | `KEPLERN SETPD BOV` | 40001 | `lck_entry` |
| 598 | 12,2024 | `0` | 00001 | `lck_entry` |
| 599 | 12,2025 | `+1` | 24026 | `lck_entry` |
| 600 | 12,2026 | `VLOAD*` | 77773 | `lck_entry` |
| 601 | 12,2027 | `MUTABLE,1` | 10005 | `lck_entry` |
| 602 | 12,2030 | `STOVL 14D` | 24017 | `lck_entry` |
| 603 | 12,2031 | `RRECT` | 01503 | `lck_entry` |
| 604 | 12,2032 | `UNIT SSP` | 66256 | `lck_entry` |
| 605 | 12,2033 | `ITERCTR` | 00027 | `lck_entry` |
| 606 | 12,2034 | `20D` | 00024 | `lck_entry` |
| 607 | 12,2035 | `STODL URRECT` | 16647 | `lck_entry` |
| 608 | 12,2036 | `36D` | 00045 | `lck_entry` |
| 609 | 12,2037 | `STOVL R1` | 24041 | `lck_entry` |
| 610 | 12,2040 | `RRECT` | 01503 | `lck_entry` |
| 611 | 12,2041 | `DOT SL1R` | 76441 | `lck_entry` |
| 612 | 12,2042 | `VRECT` | 01511 | `lck_entry` |
| 613 | 12,2043 | `DMP SL1R` | 76405 | `lck_entry` |
| 614 | 12,2044 | `1/ROOTMU` | 00023 | `lck_entry` |
| 615 | 12,2045 | `STOVL KEPC1` | 24043 | `lck_entry` |
| 616 | 12,2046 | `VRECT` | 01511 | `lck_entry` |
| 617 | 12,2047 | `VSQ DMPR` | 57236 | `lck_entry` |
| 618 | 12,2050 | `1/MU` | 00017 | `lck_entry` |
| 619 | 12,2051 | `DMP SL3` | 52405 | `lck_entry` |
| 620 | 12,2052 | `R1` | 00041 | `lck_entry` |
| 621 | 12,2053 | `DSU ROUND` | 61425 | `lck_entry` |
| 622 | 12,2054 | `D1/64` | 11035 | `lck_entry` |
| 623 | 12,2055 | `STORE KEPC2` | 00045 | `lck_entry` |
| 625 | 12,2056 | `BDSU SR1R` | 74421 | `lck_entry` |
| 626 | 12,2057 | `D1/64` | 11035 | `lck_entry` |
| 627 | 12,2060 | `DDV` | 77671 | `lck_entry` |
| 628 | 12,2061 | `R1` | 00041 | `lck_entry` |
| 629 | 12,2062 | `STORE ALPHA` | 00011 | `lck_entry` |
| 631 | 12,2063 | `BPL DLOAD` | 71244 | `lck_entry` |
| 632 | 12,2064 | `1REV` | 24073 | `lck_entry` |
| 633 | 12,2065 | `-50SC` | 11053 | `lck_entry` |
| 634 | 12,2066 | `DDV BOV` | 40071 | `lck_entry` |
| 635 | 12,2067 | `ALPHA` | 00011 | `lck_entry` |
| 636 | 12,2070 | `STOREMAX` | 24077 | `lck_entry` |
| 637 | 12,2071 | `SQRT GOTO` | 52166 | `lck_entry` |
| 638 | 12,2072 | `STOREMAX` | 24077 | `lck_entry` |
| 640 | 12,2073 | `1REV SQRT BDDV` | 55366 | `lck_1rev` |
| 642 | 12,2074 | `2PISC` | 11055 | `lck_1rev` |
| 643 | 12,2075 | `BOV` | 77600 | `lck_1rev` |
| 644 | 12,2076 | `STOREMAX` | 24077 | `lck_1rev` |
| 645 | 12,2077 | `STOREMAX STORE XMAX` | 00013 | `lck_storemax` |
| 646 | 12,2100 | `DMP PDDL` | 65205 | `lck_storemax` |
| 647 | 12,2101 | `1/ROOTMU` | 00023 | `lck_storemax` |
| 648 | 12,2102 | `ALPHA` | 00011 | `lck_storemax` |
| 649 | 12,2103 | `NORM PDDL` | 65301 | `lck_storemax` |
| 650 | 12,2104 | `X1` | 00047 | `lck_storemax` |
| 651 | 12,2105 | `SL* DDV` | 56257 | `lck_storemax` |
| 652 | 12,2106 | `0 -6,1` | 20173 | `lck_storemax` |
| 653 | 12,2107 | `BOV BMN` | 50000 | `lck_storemax` |
| 654 | 12,2110 | `MODDONE` | 24124 | `lck_storemax` |
| 655 | 12,2111 | `MODDONE` | 24124 | `lck_storemax` |
| 656 | 12,2112 | `PERIODCH PDDL ABS` | 51525 | `lck_periodch` |
| 657 | 12,2113 | `TAU.` | 02074 | `lck_periodch` |
| 658 | 12,2114 | `DSU BMN` | 50025 | `lck_periodch` |
| 659 | 12,2115 | `0D` | 00001 | `lck_periodch` |
| 660 | 12,2116 | `MODDONE` | 24124 | `lck_periodch` |
| 661 | 12,2117 | `SIGN` | 77765 | `lck_periodch` |
| 662 | 12,2120 | `TAU.` | 02074 | `lck_periodch` |
| 663 | 12,2121 | `STODL TAU.` | 16074 | `lck_periodch` |
| 664 | 12,2122 | `GOTO` | 77650 | `lck_periodch` |
| 665 | 12,2123 | `PERIODCH` | 24112 | `lck_periodch` |
| 666 | 12,2124 | `MODDONE SETPD DLOAD` | 71201 | `lck_moddone` |
| 667 | 12,2125 | `0` | 00001 | `lck_moddone` |
| 668 | 12,2126 | `XKEPNEW` | 02130 | `lck_moddone` |
| 669 | 12,2127 | `STORE X` | 00025 | `lck_moddone` |
| 670 | 12,2130 | `SIGN BZE` | 53165 | `lck_moddone` |
| 671 | 12,2131 | `TAU.` | 02074 | `lck_moddone` |
| 672 | 12,2132 | `BADX` | 24302 | `lck_moddone` |
| 673 | 12,2133 | `BMN ABS` | 51440 | `lck_moddone` |
| 674 | 12,2134 | `BADX` | 24302 | `lck_moddone` |
| 675 | 12,2135 | `DSU BPL` | 51025 | `lck_moddone` |
| 676 | 12,2136 | `XMAX` | 00013 | `lck_moddone` |
| 677 | 12,2137 | `BADX` | 24302 | `lck_moddone` |
| 678 | 12,2140 | `STORBNDS DLOAD BPL` | 51145 | `lck_storbnds` |
| 679 | 12,2141 | `TAU.` | 02074 | `lck_storbnds` |
| 680 | 12,2142 | `STOREMIN` | 24152 | `lck_storbnds` |
| 681 | 12,2143 | `DLOAD DCOMP` | 57545 | `lck_storbnds` |
| 682 | 12,2144 | `XMAX` | 00013 | `lck_storbnds` |
| 683 | 12,2145 | `STODL XMIN` | 14015 | `lck_storbnds` |
| 684 | 12,2146 | `KEPZERO` | 24007 | `lck_storbnds` |
| 685 | 12,2147 | `STORE XMAX` | 00013 | `lck_storbnds` |
| 686 | 12,2150 | `GOTO` | 77650 | `lck_storbnds` |
| 687 | 12,2151 | `DXCOMP` | 24155 | `lck_storbnds` |
| 688 | 12,2152 | `STOREMIN DLOAD` | 77745 | `lck_storemin` |
| 689 | 12,2153 | `KEPZERO` | 24007 | `lck_storemin` |
| 690 | 12,2154 | `STORE XMIN` | 00015 | `lck_storemin` |
| 691 | 12,2155 | `DXCOMP DLOAD DMPR` | 57345 | `lck_dxcomp` |
| 693 | 12,2156 | `TAU.` | 02074 | `lck_dxcomp` |
| 694 | 12,2157 | `BEE22` | 11046 | `lck_dxcomp` |
| 695 | 12,2160 | `ABS` | 77646 | `lck_dxcomp` |
| 696 | 12,2161 | `STODL EPSILONT` | 16177 | `lck_dxcomp` |
| 697 | 12,2162 | `XPREV` | 01553 | `lck_dxcomp` |
| 698 | 12,2163 | `XDIFF BDSU` | 77621 | `lck_dxcomp` |
| 699 | 12,2164 | `X` | 00025 | `lck_dxcomp` |
| 700 | 12,2165 | `STORE DELX` | 02643 | `lck_dxcomp` |
| 702 | 12,2166 | `KEPLOOP DLOAD DSQ` | 63545 | `lck_keploop` |
| 703 | 12,2167 | `X` | 00025 | `lck_keploop` |
| 704 | 12,2170 | `NORM PUSH` | 41501 | `lck_keploop` |
| 705 | 12,2171 | `X1` | 00047 | `lck_keploop` |
| 706 | 12,2172 | `DMP SRR*` | 53605 | `lck_keploop` |
| 707 | 12,2173 | `ALPHA` | 00011 | `lck_keploop` |
| 708 | 12,2174 | `0 -6,1` | 21573 | `lck_keploop` |
| 709 | 12,2175 | `STCALL XI` | 34031 | `lck_keploop` |
| 710 | 12,2176 | `DELTIME` | 24421 | `lck_keploop` |
| 711 | 12,2177 | `BOV BDSU` | 44200 | `lck_kepret` |
| 712 | 12,2200 | `TIMEOVFL` | 24311 | `lck_kepret` |
| 713 | 12,2201 | `TAU.` | 02074 | `lck_kepret` |
| 714 | 12,2202 | `STORE DELT` | 02645 | `lck_kepret` |
| 715 | 12,2203 | `ABS BDSU` | 44246 | `lck_kepret` |
| 716 | 12,2204 | `EPSILONT` | 02177 | `lck_kepret` |
| 717 | 12,2205 | `BPL DLOAD` | 71244 | `lck_kepret` |
| 718 | 12,2206 | `KEPCONVG` | 24333 | `lck_kepret` |
| 719 | 12,2207 | `T` | 00037 | `lck_kepret` |
| 720 | 12,2210 | `DSU NORM` | 60225 | `lck_kepret` |
| 721 | 12,2211 | `TC` | 01551 | `lck_kepret` |
| 722 | 12,2212 | `X1` | 00047 | `lck_kepret` |
| 723 | 12,2213 | `PDDL NORM` | 60325 | `lck_kepret` |
| 724 | 12,2214 | `DELX` | 02643 | `lck_kepret` |
| 725 | 12,2215 | `X2` | 00050 | `lck_kepret` |
| 726 | 12,2216 | `XSU,1 DMP` | 41260 | `lck_kepret` |
| 727 | 12,2217 | `X2` | 00047 | `lck_kepret` |
| 728 | 12,2220 | `DELT` | 02645 | `lck_kepret` |
| 729 | 12,2221 | `SLR* DDV` | 56257 | `lck_kepret` |
| 730 | 12,2222 | `1,1` | 21202 | `lck_kepret` |
| 731 | 12,2223 | `SR1 PUSH` | 41542 | `lck_kepret` |
| 732 | 12,2224 | `BPL DLOAD` | 71244 | `lck_kepret` |
| 733 | 12,2225 | `POSDELX` | 24246 | `lck_kepret` |
| 734 | 12,2226 | `X` | 00025 | `lck_kepret` |
| 735 | 12,2227 | `STORE XMAX` | 00013 | `lck_kepret` |
| 736 | 12,2230 | `BDSU DSU` | 45221 | `lck_kepret` |
| 737 | 12,2231 | `XMIN` | 00015 | `lck_kepret` |
| 738 | 12,2232 | `BOV BPL` | 51000 | `lck_kepret` |
| 739 | 12,2233 | `NDXCHNGE` | 24240 | `lck_kepret` |
| 740 | 12,2234 | `NDXCHNGE` | 24240 | `lck_kepret` |
| 741 | 12,2235 | `DLOAD GOTO` | 52145 | `lck_kepret` |
| 743 | 12,2236 | `0D` | 00001 | `lck_kepret` |
| 744 | 12,2237 | `NEWDELX` | 24260 | `lck_kepret` |
| 746 | 12,2240 | `NDXCHNGE DLOAD DSU` | 45345 | `lck_ndxchnge` |
| 747 | 12,2241 | `XMIN` | 00015 | `lck_ndxchnge` |
| 748 | 12,2242 | `X` | 00025 | `lck_ndxchnge` |
| 749 | 12,2243 | `DMPR GOTO` | 52075 | `lck_ndxchnge` |
| 750 | 12,2244 | `DP9/10` | 11051 | `lck_ndxchnge` |
| 751 | 12,2245 | `NEWDELX` | 24260 | `lck_ndxchnge` |
| 753 | 12,2246 | `POSDELX DLOAD` | 77745 | `lck_posdelx` |
| 754 | 12,2247 | `X` | 00025 | `lck_posdelx` |
| 755 | 12,2250 | `STORE XMIN` | 00015 | `lck_posdelx` |
| 756 | 12,2251 | `BDSU DSU` | 45221 | `lck_posdelx` |
| 757 | 12,2252 | `XMAX` | 00013 | `lck_posdelx` |
| 758 | 12,2253 | `BOV BMN` | 50000 | `lck_posdelx` |
| 759 | 12,2254 | `PDXCHNGE` | 24274 | `lck_posdelx` |
| 760 | 12,2255 | `PDXCHNGE` | 24274 | `lck_posdelx` |
| 761 | 12,2256 | `DLOAD` | 77745 | `lck_posdelx` |
| 762 | 12,2257 | `0D` | 00001 | `lck_posdelx` |
| 763 | 12,2260 | `NEWDELX STORE DELX` | 02643 | `lck_newdelx` |
| 764 | 12,2261 | `BZE DAD` | 43254 | `lck_newdelx` |
| 765 | 12,2262 | `KEPCONVG` | 24333 | `lck_newdelx` |
| 766 | 12,2263 | `X` | 00025 | `lck_newdelx` |
| 767 | 12,2264 | `STODL X` | 14025 | `lck_newdelx` |
| 768 | 12,2265 | `T` | 00037 | `lck_newdelx` |
| 769 | 12,2266 | `STORE TC` | 01551 | `lck_newdelx` |
| 770 | 12,2267 | `BRNCHCTR RTB BHIZ` | 46034 | `lck_brnchctr` |
| 771 | 12,2270 | `CHECKCTR` | 24644 | `lck_brnchctr` |
| 772 | 12,2271 | `KEPCONVG` | 24333 | `lck_brnchctr` |
| 773 | 12,2272 | `GOTO` | 77650 | `lck_brnchctr` |
| 774 | 12,2273 | `KEPLOOP` | 24166 | `lck_brnchctr` |
| 776 | 12,2274 | `PDXCHNGE DLOAD DSU` | 45345 | `lck_pdxchnge` |
| 777 | 12,2275 | `XMAX` | 00013 | `lck_pdxchnge` |
| 778 | 12,2276 | `X` | 00025 | `lck_pdxchnge` |
| 779 | 12,2277 | `DMPR GOTO` | 52075 | `lck_pdxchnge` |
| 780 | 12,2300 | `DP9/10` | 11051 | `lck_pdxchnge` |
| 781 | 12,2301 | `NEWDELX` | 24260 | `lck_pdxchnge` |
| 783 | 12,2302 | `BADX DLOAD SR1` | 70545 | `lck_badx` |
| 784 | 12,2303 | `XMAX` | 00013 | `lck_badx` |
| 785 | 12,2304 | `SIGN` | 77765 | `lck_badx` |
| 786 | 12,2305 | `TAU.` | 02074 | `lck_badx` |
| 787 | 12,2306 | `STORE X` | 00025 | `lck_badx` |
| 788 | 12,2307 | `GOTO` | 77650 | `lck_badx` |
| 790 | 12,2310 | `STORBNDS` | 24140 | `lck_badx` |
| 791 | 12,2311 | `TIMEOVFL DLOAD BMN` | 50145 | `lck_timeovfl` |
| 792 | 12,2312 | `X` | 00025 | `lck_timeovfl` |
| 793 | 12,2313 | `NEGTOVFL` | 24330 | `lck_timeovfl` |
| 794 | 12,2314 | `STORE XMAX` | 00013 | `lck_timeovfl` |
| 795 | 12,2315 | `CMNTOVFL DLOAD SR1` | 70545 | `lck_cmntovfl` |
| 796 | 12,2316 | `DELX` | 02643 | `lck_cmntovfl` |
| 797 | 12,2317 | `STORE DELX` | 02643 | `lck_cmntovfl` |
| 798 | 12,2320 | `BZE BDSU` | 44254 | `lck_cmntovfl` |
| 799 | 12,2321 | `KEPRTN` | 02112 | `lck_cmntovfl` |
| 800 | 12,2322 | `X` | 00025 | `lck_cmntovfl` |
| 801 | 12,2323 | `STODL X` | 14025 | `lck_cmntovfl` |
| 802 | 12,2324 | `TC` | 01551 | `lck_cmntovfl` |
| 803 | 12,2325 | `STORE T` | 00037 | `lck_cmntovfl` |
| 804 | 12,2326 | `GOTO` | 77650 | `lck_cmntovfl` |
| 805 | 12,2327 | `BRNCHCTR` | 24267 | `lck_cmntovfl` |
| 806 | 12,2330 | `NEGTOVFL STORE XMIN` | 00015 | `lck_negtovfl` |
| 807 | 12,2331 | `GOTO` | 77650 | `lck_negtovfl` |
| 808 | 12,2332 | `CMNTOVFL` | 24315 | `lck_negtovfl` |
| 809 | 12,2333 | `KEPCONVG DLOAD SR4R` | 44545 | `lck_kepconvg` |
| 810 | 12,2334 | `R1` | 00041 | `lck_kepconvg` |
| 811 | 12,2335 | `DSU VXSC` | 74225 | `lck_kepconvg` |
| 812 | 12,2336 | `XSQC(XI)` | 00035 | `lck_kepconvg` |
| 813 | 12,2337 | `URRECT` | 02647 | `lck_kepconvg` |
| 814 | 12,2340 | `VSL1 PDDL` | 65372 | `lck_kepconvg` |
| 815 | 12,2341 | `X` | 00025 | `lck_kepconvg` |
| 816 | 12,2342 | `DSQ NORM` | 60316 | `lck_kepconvg` |
| 817 | 12,2343 | `X1` | 00047 | `lck_kepconvg` |
| 818 | 12,2344 | `DMPR DMPR` | 57275 | `lck_kepconvg` |
| 819 | 12,2345 | `1/ROOTMU` | 00023 | `lck_kepconvg` |
| 820 | 12,2346 | `X` | 00025 | `lck_kepconvg` |
| 821 | 12,2347 | `DMP SRR*` | 53605 | `lck_kepconvg` |
| 822 | 12,2350 | `S(XI)` | 00033 | `lck_kepconvg` |
| 823 | 12,2351 | `0 -7,1` | 21572 | `lck_kepconvg` |
| 824 | 12,2352 | `BDSU` | 77621 | `lck_kepconvg` |
| 825 | 12,2353 | `T` | 00037 | `lck_kepconvg` |
| 826 | 12,2354 | `SL1 VXSC` | 74352 | `lck_kepconvg` |
| 827 | 12,2355 | `VRECT` | 01511 | `lck_kepconvg` |
| 828 | 12,2356 | `VSL1 VAD` | 53372 | `lck_kepconvg` |
| 829 | 12,2357 | `VSL4` | 77712 | `lck_kepconvg` |
| 830 | 12,2360 | `STORE RCV` | 01535 | `lck_kepconvg` |
| 832 | 12,2361 | `ABVAL NORM` | 60246 | `lck_kepconvg` |
| 833 | 12,2362 | `X2` | 00050 | `lck_kepconvg` |
| 834 | 12,2363 | `STODL RCNORM` | 14043 | `lck_kepconvg` |
| 835 | 12,2364 | `XI` | 00031 | `lck_kepconvg` |
| 836 | 12,2365 | `DMPR DSU` | 45275 | `lck_kepconvg` |
| 837 | 12,2366 | `S(XI)` | 00033 | `lck_kepconvg` |
| 838 | 12,2367 | `D1/128` | 11033 | `lck_kepconvg` |
| 840 | 12,2370 | `DMP SL1R` | 76405 | `lck_kepconvg` |
| 841 | 12,2371 | `ROOTMU` | 00021 | `lck_kepconvg` |
| 842 | 12,2372 | `DMP SLR*` | 53605 | `lck_kepconvg` |
| 843 | 12,2373 | `X` | 00025 | `lck_kepconvg` |
| 844 | 12,2374 | `0 -3,2` | 56601 | `lck_kepconvg` |
| 845 | 12,2375 | `DDV VXSC` | 74271 | `lck_kepconvg` |
| 846 | 12,2376 | `RCNORM` | 00043 | `lck_kepconvg` |
| 847 | 12,2377 | `URRECT` | 02647 | `lck_kepconvg` |
| 848 | 12,2400 | `VSL1 PDDL` | 65372 | `lck_kepconvg` |
| 849 | 12,2401 | `XSQC(XI)` | 00035 | `lck_kepconvg` |
| 850 | 12,2402 | `SLR* DDV` | 56257 | `lck_kepconvg` |
| 851 | 12,2403 | `0 -4,2` | 56602 | `lck_kepconvg` |
| 852 | 12,2404 | `RCNORM` | 00043 | `lck_kepconvg` |
| 853 | 12,2405 | `BDSU VXSC` | 74221 | `lck_kepconvg` |
| 854 | 12,2406 | `D1/256` | 11047 | `lck_kepconvg` |
| 855 | 12,2407 | `VRECT` | 01511 | `lck_kepconvg` |
| 856 | 12,2410 | `VAD VSL8` | 42455 | `lck_kepconvg` |
| 857 | 12,2411 | `STADR` | 77626 | `lck_kepconvg` |
| 858 | 12,2412 | `STODL VCV` | 62234 | `lck_kepconvg` |
| 859 | 12,2413 | `T` | 00037 | `lck_kepconvg` |
| 860 | 12,2414 | `STODL TC` | 15551 | `lck_kepconvg` |
| 861 | 12,2415 | `X` | 00025 | `lck_kepconvg` |
| 862 | 12,2416 | `STORE XPREV` | 01553 | `lck_kepconvg` |
| 863 | 12,2417 | `GOTO` | 77650 | `lck_kepconvg` |
| 864 | 12,2420 | `KEPRTN` | 02112 | `lck_kepconvg` |
| 867 | 12,2421 | `DELTIME EXIT` | 77776 | `lck_deltime` |
| 868 | 12,2422 | `TC POLY` | 07222 | `lck_deltime` |
| 869 | 12,2423 | `DEC 8` | 00010 | `lck_deltime` |
| 870 | 12,2424 | `2DEC .083333334` | 02525 12526 | `lck_deltime` |
| 871 | 12,2426 | `2DEC -.266666684` | 67356 75666 | `lck_deltime` |
| 872 | 12,2430 | `2DEC .406349155` | 15001 23771 | `lck_deltime` |
| 873 | 12,2432 | `2DEC -.361198675` | 64342 43674 | `lck_deltime` |
| 874 | 12,2434 | `2DEC .210153242` | 06563 04645 | `lck_deltime` |
| 875 | 12,2436 | `2DEC -.086221951` | 75173 52672 | `lck_deltime` |
| 876 | 12,2440 | `2DEC .026268812` | 00656 14331 | `lck_deltime` |
| 877 | 12,2442 | `2DEC -.006163316` | 77633 40512 | `lck_deltime` |
| 878 | 12,2444 | `2DEC .001177342` | 00023 11210 | `lck_deltime` |
| 879 | 12,2446 | `2DEC -.000199055` | 77774 67506 | `lck_deltime` |
| 881 | 12,2450 | `TC INTPRET` | 06037 | `lck_deltime` |
| 882 | 12,2451 | `STODL S(XI)` | 14033 | `lck_deltime` |
| 883 | 12,2452 | `XI` | 00031 | `lck_deltime` |
| 884 | 12,2453 | `EXIT` | 77776 | `lck_deltime2` |
| 885 | 12,2454 | `TC POLY` | 07222 | `lck_deltime2` |
| 886 | 12,2455 | `DEC 8` | 00010 | `lck_deltime2` |
| 887 | 12,2456 | `2DEC .031250001` | 01000 00000 | `lck_deltime2` |
| 888 | 12,2460 | `2DEC -.166666719` | 72525 52506 | `lck_deltime2` |
| 889 | 12,2462 | `2DEC .355555413` | 13301 15337 | `lck_deltime2` |
| 890 | 12,2464 | `2DEC -.406347410` | 62776 54733 | `lck_deltime2` |
| 891 | 12,2466 | `2DEC .288962094` | 11176 13267 | `lck_deltime2` |
| 892 | 12,2470 | `2DEC -.140117894` | 73410 51674 | `lck_deltime2` |
| 893 | 12,2472 | `2DEC .049247387` | 01446 33641 | `lck_deltime2` |
| 894 | 12,2474 | `2DEC -.013081923` | 77451 65233 | `lck_deltime2` |
| 895 | 12,2476 | `2DEC .002806389` | 00055 37266 | `lck_deltime2` |
| 896 | 12,2500 | `2DEC -.000529414` | 77767 52336 | `lck_deltime2` |
| 897 | 12,2502 | `TC INTPRET` | 06037 | `lck_deltime2` |
| 899 | 12,2503 | `DMP SRR*` | 53605 | `lck_deltail` |
| 900 | 12,2504 | `0D` | 00001 | `lck_deltail` |
| 901 | 12,2505 | `0 -5,1` | 21574 | `lck_deltail` |
| 902 | 12,2506 | `STORE XSQC(XI)` | 00035 | `lck_deltail` |
| 903 | 12,2507 | `DMP SL1` | 72405 | `lck_deltail` |
| 904 | 12,2510 | `KEPC1` | 00043 | `lck_deltail` |
| 905 | 12,2511 | `RTB PDDL` | 65234 | `lck_deltail` |
| 906 | 12,2512 | `TPMODE` | 21633 | `lck_deltail` |
| 907 | 12,2513 | `DMP SRR*` | 53605 | `lck_deltail` |
| 908 | 12,2514 | `S(XI)` | 00033 | `lck_deltail` |
| 909 | 12,2515 | `0 -5,1` | 21574 | `lck_deltail` |
| 910 | 12,2516 | `DMP SL1` | 72405 | `lck_deltail` |
| 911 | 12,2517 | `KEPC2` | 00045 | `lck_deltail` |
| 912 | 12,2520 | `RTB PDDL` | 65234 | `lck_deltail` |
| 913 | 12,2521 | `TPMODE` | 21633 | `lck_deltail` |
| 914 | 12,2522 | `R1` | 00041 | `lck_deltail` |
| 915 | 12,2523 | `SR TAD` | 76261 | `lck_deltail` |
| 916 | 12,2524 | `6` | 20607 | `lck_deltail` |
| 917 | 12,2525 | `NORM DMP` | 41301 | `lck_deltail` |
| 918 | 12,2526 | `X1` | 00047 | `lck_deltail` |
| 919 | 12,2527 | `X` | 00025 | `lck_deltail` |
| 920 | 12,2530 | `SR* TAD` | 76257 | `lck_deltail` |
| 921 | 12,2531 | `0 -3,1` | 20576 | `lck_deltail` |
| 922 | 12,2532 | `SL4R DMPR` | 57232 | `lck_deltail` |
| 923 | 12,2533 | `1/ROOTMU` | 00023 | `lck_deltail` |
| 924 | 12,2534 | `STORE T` | 00037 | `lck_deltail` |
| 925 | 12,2535 | `RVQ` | 77616 | `lck_deltail` |
| 1007 | 12,2644 | `CHECKCTR CS ONE` | 44753 | `lck_brnchctr` |
| 1008 | 12,2645 | `INDEX FIXLOC` | 50120 | `lck_brnchctr` |
| 1009 | 12,2646 | `AD ITERCTR` | 60026 | `lck_brnchctr` |
| 1010 | 12,2647 | `INDEX FIXLOC` | 50120 | `lck_brnchctr` |
| 1011 | 12,2650 | `TS ITERCTR` | 54026 | `lck_brnchctr` |
| 1012 | 12,2651 | `TS MPAC` | 54154 | `lck_brnchctr` |
| 1013 | 12,2652 | `TC DANZIG` | 06061 | `lck_brnchctr` |

## Evidence

Hashes: `evidence/lm-conic-kepler/SHA256.json`. Bulk files are assets of the `evidence-lm-conic-kepler` release
(`python3 tools/evidence.py fetch lm-conic-kepler`).

- Source-to-native map and independent assembly comparison:
  `python3 tools/lm_conic_kepler_source.py` reassembles the unchanged file
  with the pinned yaYUL (rope equal to the certified fixed-data rope) and maps
  all 338 words of the range to native literals in 23 functions.
- Oracle: `tools/lm_conic_kepler_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant over 65536
  sampled KEPPREP entries (`tests/lm_conic_kepler_cases.c`): Earth and Moon
  state vectors with speeds from well below circular to hyperbolic, random
  planes and flight-path angles, transfer times from a fraction of a period to
  several periods (PERIODCH) and hyperbolic hours, initial guesses near the
  solution, poor, zero, of the wrong sign or beyond the bound (BADX), XPREV/TC,
  raw edge words in one case in eight, all three KEPRTN returns, both IRQ settings, SUPER, EBANK, all
  five VACs, PUSHLOC, MODE and OVFIND. `tools/lm_conic_kepler_differential.py`
  checks the native producer against every entry and pins the stream identity
  (`lm-conic-kepler-source-stream.json`, SHA256 `50152139...5120e0`,
  269,090,816 bytes; all 65536 return). Every phase and all 37 phase
  transitions are reached; the abort arm is not (above).
- Host and sanitizer results: `make test-lm-conic-kepler
  sanitize-lm-conic-kepler`. Strict and ASAN/UBSAN builds each pass 8 frozen
  entries, 21 admission guards, 90 frozen phase steps and the full
  65536-entry stream (`lm-conic-kepler-checks.log`).
- Phase frontiers: `tools/freeze_lm_conic_kepler_phases.py` requires each of
  the 1731 native frontiers of the 8 frozen entries (which together take all
  37 transitions) to equal, in order, an original NEWOPS image of the oracle,
  then freezes 90 steps in four groups.
- C64 execution results: `make vice-lm-conic-kepler`. Four PRGs each link one
  phase group and keep both images in the RAM under KERNAL ROM (`$01=$35`),
  starting each step from a debris-only base state. In VICE (PAL, stock ROMs):
  group 1 (KEPLERN to STORBNDS) 24 steps, group 2 (KEPLOOP and DELTIME) 20
  steps, group 3 (STOREMIN, DXCOMP and the update through BRNCHCTR) 28 steps,
  group 4 (BADX, time overflow, KEPCONVG) 18 steps, no failures
  (`lm-conic-kepler-results.json`).

## Limits

- The C64 check runs each phase separately; the whole chain runs on the host.
  Running it as one program needs overlay ABI 2 modules (the 23 phases are the
  module entry points).
- KEPLERN is qualified from KEPPREP's entry only (its sole caller in
  Luminary099), with the returns of KEPPREP's three callers.
- Numerical validity of the Kepler iteration is the original's; no widened
  claim.
