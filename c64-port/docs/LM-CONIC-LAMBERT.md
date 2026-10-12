# Translate the Luminary Lambert subroutine (LAMBERT, ITERATOR)

Lane B. Milestone `lm-conic-lambert`.

## Scope

The Lambert subroutine of `Luminary099/CONIC_SUBROUTINES.agc`: LAMBERT
through TARGETV (bank 12 `03212`-`03546`), its slope iterator ITERATOR
(`12,2536`-`2643`) and LAMENTER (`12,2701`-`2731`), which TARGETV calls.
That is 316 assembled words on 316 source lines. LAMBERT calls GEOM (with
NORMSW either way, so HAVENORM is reached here for the first time), GETX and
DELTIME. Those shared steps belong to the `lm-conic-timethet` and
`lm-conic-kepler` milestones and are reused unchanged. CHECKCTR is compiled
inline at its RTB from the Kepler milestone's words. LAMENTER is compiled
again here (the time-theta milestone compiles it inside its NEWSTATE step),
so the merged time-theta evidence stays valid.

With this milestone every assembled word of the file has a native owner
(`whole_file` in the source map): Kepler 338, time-theta 265 (plus HAVENORM's
RVQ at `04,3164`), time-radius 123, Lambert 316 and the 29 CONICS1 constant
words, which the native files carry as literal tables. The coverage entry
carries a drafted complete certificate and stays `partial` until the auditor
reviews it.

`lm_conic_lambert(state, io)` runs from the original interpretive entry
frontier (NEWOPS, FBANK 12, LOC `03212`) to the caller's NEWOPS frontier at
C(RTNLAMB). It returns 1 there, 2 at the UNIT/SQRT abort frontier (alarm
1302), or 0 with the image unchanged when the entry is outside the contract.
Admitted entries:

- canonical 2048-word image, coherent EB/FB/BB with BANKSET = BB, INTBIT15 and
  EDOP `+0`, all words 15-bit, Z and NEWJOB zero, one of the five VACs, and
  X1 `-2` or `-10` (RTX1);
- QPRET `022145`, the return of INITVEL's `CALL SETITCTR` at `11,2143`
  (Luminary's only LAMBERT call), with ITERCTR 20, or 5 when AVEGFLAG is set,
  as SETITCTR (`11,3765`) leaves it before its BOFF or GOTO LAMBERT.

The cases put two positions on an Earth or Moon conic (speeds from well below
circular to hyperbolic, random planes) and the Kepler time between them:
short, near half a revolution, near a whole one, or anywhere. Some cases
perturb the time or put the target along the first position (360LAMB).
GEOMSGN follows the sense of travel or is random. GUESSW is set or clear
(with a COGA guess near the truth or anywhere in range), NORMSW is set with
the orbit normal in UN on a quarter of the cases, and VTARGTAG is zero half
the time. One case in eight uses raw edge words.

### How it is translated

Every interpretive pair is compiled with the general Luminary interpretive
bodies (`src/lm_interp_orders.c`) and the dispatch paths of
`src/lm_interp_steps.c`. No new bodies were needed. Each call passes its
source line's order and operand words as literals. MUTABLE (bank 4) and ONE
(bank 2) come from the certified fixed-data table. BEE17, BEE19, D1/n,
DP9/10, ONEBIT, COGUPLIM and COGLOLIM are native literals.

The routine is split into 30 new steps between original NEWOPS frontiers
(`enum lm_conic_lambert_phase`, `lla_*`). They run with the time-theta
milestone's shared steps, which keep their `lm_conic_timethet_phase`
numbers. `lla_remap` follows GEOM's, GETX's and DELTIME's `RVQ` to the return
address in LOC (`03235`, `03363`, `03373`); ITERATOR's `RVQ` returns to
`03407`. The longest sampled entry runs 4542 steps: CHECKCTR bounds the
ITERATOR passes to 20, and between them the HIENERGY/LOENERGY passes halve
DCOGA until it is zero.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_conic_lambert_source.py`,
`evidence/lm-conic-lambert/lm-conic-lambert-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| 928 | 12,2536 | `ITERATOR BONCLR DLOAD` | 71214 | `lla_iterator` |
| 929 | 12,2537 | `SLOPESW` | 00614 | `lla_iterator` |
| 930 | 12,2540 | `FIRSTIME` | 24606 | `lla_iterator` |
| 931 | 12,2541 | `DEP` | 00037 | `lla_iterator` |
| 932 | 12,2542 | `DSU NORM` | 60225 | `lla_iterator` |
| 933 | 12,2543 | `DEPREV` | 02762 | `lla_iterator` |
| 934 | 12,2544 | `X1` | 00047 | `lla_iterator` |
| 935 | 12,2545 | `PDDL NORM` | 60325 | `lla_iterator` |
| 936 | 12,2546 | `DELINDEP` | 00015 | `lla_iterator` |
| 937 | 12,2547 | `X2` | 00050 | `lla_iterator` |
| 938 | 12,2550 | `XSU,1 DMP` | 41260 | `lla_iterator` |
| 939 | 12,2551 | `X2` | 00047 | `lla_iterator` |
| 940 | 12,2552 | `DELDEP` | 02760 | `lla_iterator` |
| 941 | 12,2553 | `SLR* DDV` | 56257 | `lla_iterator` |
| 942 | 12,2554 | `1,1` | 21202 | `lla_iterator` |
| 943 | 12,2555 | `SR1 BOFF` | 43142 | `lla_iterator` |
| 944 | 12,2556 | `ORDERSW` | 04351 | `lla_iterator` |
| 945 | 12,2557 | `SGNCHECK` | 24562 | `lla_iterator` |
| 946 | 12,2560 | `ABS SIGN` | 75246 | `lla_iterator` |
| 947 | 12,2561 | `DELDEP` | 02760 | `lla_iterator` |
| 949 | 12,2562 | `SGNCHECK PUSH BPL` | 51006 | `lla_sgncheck` |
| 950 | 12,2563 | `POSDEL` | 24620 | `lla_sgncheck` |
| 951 | 12,2564 | `DLOAD BON` | 43145 | `lla_sgncheck` |
| 952 | 12,2565 | `INDEP` | 02766 | `lla_sgncheck` |
| 953 | 12,2566 | `ORDERSW` | 04311 | `lla_sgncheck` |
| 954 | 12,2567 | `MINCHECK` | 24571 | `lla_sgncheck` |
| 955 | 12,2570 | `STORE MAX` | 00017 | `lla_sgncheck` |
| 957 | 12,2571 | `MINCHECK BDSU DSU` | 45221 | `lla_mincheck` |
| 958 | 12,2572 | `MIN` | 00011 | `lla_mincheck` |
| 959 | 12,2573 | `BOV BPL` | 51000 | `lla_mincheck` |
| 960 | 12,2574 | `MODNGDEL` | 24600 | `lla_mincheck` |
| 961 | 12,2575 | `MODNGDEL` | 24600 | `lla_mincheck` |
| 962 | 12,2576 | `GOTO` | 77650 | `lla_mincheck` |
| 963 | 12,2577 | `DELOK` | 24632 | `lla_mincheck` |
| 965 | 12,2600 | `MODNGDEL DLOAD DSU` | 45345 | `lla_modngdel` |
| 966 | 12,2601 | `MIN` | 00011 | `lla_modngdel` |
| 967 | 12,2602 | `INDEP` | 02766 | `lla_modngdel` |
| 968 | 12,2603 | `DMP GOTO` | 52005 | `lla_modngdel` |
| 969 | 12,2604 | `DP9/10` | 11051 | `lla_modngdel` |
| 970 | 12,2605 | `NEWDEL` | 24634 | `lla_modngdel` |
| 972 | 12,2606 | `FIRSTIME DLOAD DMP` | 41345 | `lla_firstime` |
| 973 | 12,2607 | `MIN` | 00011 | `lla_firstime` |
| 974 | 12,2610 | `TWEEKIT` | 00051 | `lla_firstime` |
| 975 | 12,2611 | `PDDL DMP` | 41325 | `lla_firstime` |
| 977 | 12,2612 | `MAX` | 00017 | `lla_firstime` |
| 978 | 12,2613 | `TWEEKIT` | 00051 | `lla_firstime` |
| 979 | 12,2614 | `DSU` | 77625 | `lla_firstime` |
| 980 | 12,2615 | `SIGN GOTO` | 52165 | `lla_firstime` |
| 981 | 12,2616 | `DELDEP` | 02760 | `lla_firstime` |
| 982 | 12,2617 | `SGNCHECK` | 24562 | `lla_firstime` |
| 984 | 12,2620 | `POSDEL DLOAD BON` | 43145 | `lla_posdel` |
| 985 | 12,2621 | `INDEP` | 02766 | `lla_posdel` |
| 986 | 12,2622 | `ORDERSW` | 04311 | `lla_posdel` |
| 987 | 12,2623 | `MAXCHECK` | 24625 | `lla_posdel` |
| 988 | 12,2624 | `STORE MIN` | 00011 | `lla_posdel` |
| 990 | 12,2625 | `MAXCHECK BDSU DSU` | 45221 | `lla_maxcheck` |
| 991 | 12,2626 | `MAX` | 00017 | `lla_maxcheck` |
| 992 | 12,2627 | `BOV BMN` | 50000 | `lla_maxcheck` |
| 993 | 12,2630 | `MODPSDEL` | 24636 | `lla_maxcheck` |
| 994 | 12,2631 | `MODPSDEL` | 24636 | `lla_maxcheck` |
| 995 | 12,2632 | `DELOK DLOAD` | 77745 | `lla_delok` |
| 996 | 12,2633 | `0D` | 00001 | `lla_delok` |
| 997 | 12,2634 | `NEWDEL STORE DELINDEP` | 00015 | `lla_newdel` |
| 998 | 12,2635 | `RVQ` | 77616 | `lla_newdel` |
| 1000 | 12,2636 | `MODPSDEL DLOAD DSU` | 45345 | `lla_modpsdel` |
| 1001 | 12,2637 | `MAX` | 00017 | `lla_modpsdel` |
| 1002 | 12,2640 | `INDEP` | 02766 | `lla_modpsdel` |
| 1003 | 12,2641 | `DMP GOTO` | 52005 | `lla_modpsdel` |
| 1004 | 12,2642 | `DP9/10` | 11051 | `lla_modpsdel` |
| 1005 | 12,2643 | `NEWDEL` | 24634 | `lla_modpsdel` |
| 1038 | 12,2701 | `LAMENTER NORM` | 77701 | `lla_targetv` |
| 1039 | 12,2702 | `X1` | 00047 | `lla_targetv` |
| 1040 | 12,2703 | `STODL R2` | 16720 | `lla_targetv` |
| 1041 | 12,2704 | `XI` | 00031 | `lla_targetv` |
| 1042 | 12,2705 | `DMP DSU` | 45205 | `lla_targetv` |
| 1043 | 12,2706 | `S(XI)` | 00033 | `lla_targetv` |
| 1044 | 12,2707 | `D1/128` | 11033 | `lla_targetv` |
| 1045 | 12,2710 | `DMP SL1R` | 76405 | `lla_targetv` |
| 1046 | 12,2711 | `ROOTMU` | 00021 | `lla_targetv` |
| 1047 | 12,2712 | `DMP SLR*` | 53605 | `lla_targetv` |
| 1048 | 12,2713 | `X` | 00025 | `lla_targetv` |
| 1049 | 12,2714 | `0 -3,1` | 21176 | `lla_targetv` |
| 1050 | 12,2715 | `DDV VXSC` | 74271 | `lla_targetv` |
| 1051 | 12,2716 | `R2` | 02720 | `lla_targetv` |
| 1052 | 12,2717 | `UR1` | 02722 | `lla_targetv` |
| 1053 | 12,2720 | `VSL1 PDDL` | 65372 | `lla_targetv` |
| 1054 | 12,2721 | `XSQC(XI)` | 00035 | `lla_targetv` |
| 1055 | 12,2722 | `SLR* DDV` | 56257 | `lla_targetv` |
| 1056 | 12,2723 | `0 -4,1` | 21175 | `lla_targetv` |
| 1057 | 12,2724 | `R2` | 02720 | `lla_targetv` |
| 1058 | 12,2725 | `BDSU` | 77621 | `lla_targetv` |
| 1059 | 12,2726 | `D1/256` | 11047 | `lla_targetv` |
| 1060 | 12,2727 | `VXSC VAD` | 53361 | `lla_targetv` |
| 1061 | 12,2730 | `VVEC` | 02744 | `lla_targetv` |
| 1062 | 12,2731 | `VSL8 RVQ` | 43412 | `lla_targetv` |
| 1359 | 12,3212 | `LAMBERT STQ SETPD` | 40220 | `lla_lambert` |
| 1360 | 12,3213 | `RTNLAMB` | 02710 | `lla_lambert` |
| 1361 | 12,3214 | `0D` | 00001 | `lla_lambert` |
| 1362 | 12,3215 | `BOV` | 77600 | `lla_lambert` |
| 1363 | 12,3216 | `+1` | 25217 | `lla_lambert` |
| 1364 | 12,3217 | `CLEAR VLOAD*` | 76614 | `lla_lambert` |
| 1365 | 12,3220 | `SOLNSW` | 02674 | `lla_lambert` |
| 1366 | 12,3221 | `MUTABLE,1` | 10005 | `lla_lambert` |
| 1367 | 12,3222 | `STODL 1/MU` | 14017 | `lla_lambert` |
| 1368 | 12,3223 | `TDESIRED` | 02671 | `lla_lambert` |
| 1369 | 12,3224 | `DMPR` | 77675 | `lla_lambert` |
| 1370 | 12,3225 | `BEE19` | 11042 | `lla_lambert` |
| 1371 | 12,3226 | `STORE EPSILONL` | 02764 | `lla_lambert` |
| 1372 | 12,3227 | `SET VLOAD` | 77214 | `lla_lambert` |
| 1373 | 12,3230 | `SLOPESW` | 00474 | `lla_lambert` |
| 1374 | 12,3231 | `R1VEC` | 02655 | `lla_lambert` |
| 1375 | 12,3232 | `PDVL CALL` | 45115 | `lla_lambert` |
| 1376 | 12,3233 | `R2VEC` | 02663 | `lla_lambert` |
| 1377 | 12,3234 | `GEOM` | 11130 | `lla_lambert` |
| 1378 | 12,3235 | `STODL SNTH` | 16730 | `lla_aftergeom` |
| 1379 | 12,3236 | `MAGVEC2` | 02720 | `lla_aftergeom` |
| 1380 | 12,3237 | `NORM PDDL` | 65301 | `lla_aftergeom` |
| 1381 | 12,3240 | `X1` | 00047 | `lla_aftergeom` |
| 1382 | 12,3241 | `R1` | 00041 | `lla_aftergeom` |
| 1383 | 12,3242 | `SR1 DDV` | 56342 | `lla_aftergeom` |
| 1384 | 12,3243 | `SL* PDDL` | 65257 | `lla_aftergeom` |
| 1385 | 12,3244 | `0 -6,1` | 20173 | `lla_aftergeom` |
| 1386 | 12,3245 | `STADR` | 77626 | `lla_aftergeom` |
| 1387 | 12,3246 | `STORE CSTH` | 75045 | `lla_aftergeom` |
| 1388 | 12,3247 | `SR1 BDSU` | 44342 | `lla_aftergeom` |
| 1389 | 12,3250 | `D1/4` | 11037 | `lla_aftergeom` |
| 1390 | 12,3251 | `STORE 1-CSTH` | 02734 | `lla_aftergeom` |
| 1392 | 12,3252 | `ROUND BZE` | 53106 | `lla_aftergeom` |
| 1393 | 12,3253 | `360LAMB` | 25463 | `lla_aftergeom` |
| 1394 | 12,3254 | `NORM PDDL` | 65301 | `lla_aftergeom` |
| 1395 | 12,3255 | `X1` | 00047 | `lla_aftergeom` |
| 1396 | 12,3256 | `0D` | 00001 | `lla_aftergeom` |
| 1397 | 12,3257 | `SR1 DDV` | 56342 | `lla_aftergeom` |
| 1398 | 12,3260 | `SL* SQRT` | 75457 | `lla_aftergeom` |
| 1399 | 12,3261 | `0 -3,1` | 20176 | `lla_aftergeom` |
| 1400 | 12,3262 | `PDDL SR` | 54325 | `lla_aftergeom` |
| 1401 | 12,3263 | `SNTH` | 02730 | `lla_aftergeom` |
| 1402 | 12,3264 | `6` | 20607 | `lla_aftergeom` |
| 1403 | 12,3265 | `DDV DAD` | 43271 | `lla_aftergeom` |
| 1404 | 12,3266 | `1-CSTH` | 02734 | `lla_aftergeom` |
| 1405 | 12,3267 | `STADR` | 77626 | `lla_aftergeom` |
| 1406 | 12,3270 | `STORE COGAMAX` | 77760 | `lla_aftergeom` |
| 1407 | 12,3271 | `BOV BMN` | 50000 | `lla_aftergeom` |
| 1408 | 12,3272 | `UPLIM` | 25277 | `lla_aftergeom` |
| 1410 | 12,3273 | `MAXCOGA` | 25302 | `lla_aftergeom` |
| 1411 | 12,3274 | `DSU BMN` | 50025 | `lla_aftergeom` |
| 1412 | 12,3275 | `COGUPLIM` | 11061 | `lla_aftergeom` |
| 1413 | 12,3276 | `MAXCOGA` | 25302 | `lla_aftergeom` |
| 1414 | 12,3277 | `UPLIM DLOAD` | 77745 | `lla_uplim` |
| 1415 | 12,3300 | `COGUPLIM` | 11061 | `lla_uplim` |
| 1416 | 12,3301 | `STORE COGAMAX` | 00017 | `lla_uplim` |
| 1417 | 12,3302 | `MAXCOGA DLOAD` | 77745 | `lla_maxcoga` |
| 1418 | 12,3303 | `CSTH` | 02732 | `lla_maxcoga` |
| 1419 | 12,3304 | `SR DSU` | 45261 | `lla_maxcoga` |
| 1420 | 12,3305 | `6` | 20607 | `lla_maxcoga` |
| 1421 | 12,3306 | `STADR` | 77626 | `lla_maxcoga` |
| 1422 | 12,3307 | `STODL CSTH-RHO` | 61041 | `lla_maxcoga` |
| 1423 | 12,3310 | `GEOMSGN` | 02673 | `lla_maxcoga` |
| 1424 | 12,3311 | `BMN DLOAD` | 71240 | `lla_maxcoga` |
| 1425 | 12,3312 | `LOLIM` | 25503 | `lla_maxcoga` |
| 1426 | 12,3313 | `CSTH-RHO` | 02736 | `lla_maxcoga` |
| 1427 | 12,3314 | `SL1 DDV` | 56352 | `lla_maxcoga` |
| 1428 | 12,3315 | `SNTH` | 02730 | `lla_maxcoga` |
| 1429 | 12,3316 | `BOV` | 77600 | `lla_maxcoga` |
| 1430 | 12,3317 | `LOLIM` | 25503 | `lla_maxcoga` |
| 1431 | 12,3320 | `MINCOGA STORE COGAMIN` | 00011 | `lla_mincoga` |
| 1432 | 12,3321 | `BON SSP` | 66214 | `lla_mincoga` |
| 1433 | 12,3322 | `GUESSW` | 00715 | `lla_mincoga` |
| 1434 | 12,3323 | `NOGUESS` | 25467 | `lla_mincoga` |
| 1435 | 12,3324 | `TWEEKIT` | 00051 | `lla_mincoga` |
| 1436 | 12,3325 | `00001` | 00001 | `lla_mincoga` |
| 1437 | 12,3326 | `DLOAD` | 77745 | `lla_mincoga` |
| 1438 | 12,3327 | `COGA` | 02766 | `lla_mincoga` |
| 1440 | 12,3330 | `LAMBLOOP DMP` | 77605 | `lla_lambloop` |
| 1441 | 12,3331 | `SNTH` | 02730 | `lla_lambloop` |
| 1442 | 12,3332 | `SR1 DSU` | 45342 | `lla_lambloop` |
| 1443 | 12,3333 | `CSTH-RHO` | 02736 | `lla_lambloop` |
| 1444 | 12,3334 | `NORM PDDL` | 65301 | `lla_lambloop` |
| 1445 | 12,3335 | `X1` | 00047 | `lla_lambloop` |
| 1446 | 12,3336 | `1-CSTH` | 02734 | `lla_lambloop` |
| 1447 | 12,3337 | `SL* DDV` | 56257 | `lla_lambloop` |
| 1448 | 12,3340 | `0 -9D,1` | 20170 | `lla_lambloop` |
| 1449 | 12,3341 | `BMN BZE` | 53040 | `lla_lambloop` |
| 1450 | 12,3342 | `NEGP` | 25417 | `lla_lambloop` |
| 1451 | 12,3343 | `NEGP` | 25417 | `lla_lambloop` |
| 1452 | 12,3344 | `STODL P` | 16740 | `lla_lambloop` |
| 1453 | 12,3345 | `COGA` | 02766 | `lla_lambloop` |
| 1454 | 12,3346 | `DSQ DAD` | 43316 | `lla_lambloop` |
| 1455 | 12,3347 | `D1/1024` | 11045 | `lla_lambloop` |
| 1456 | 12,3350 | `NORM DMP` | 41301 | `lla_lambloop` |
| 1457 | 12,3351 | `X1` | 00047 | `lla_lambloop` |
| 1458 | 12,3352 | `P` | 02740 | `lla_lambloop` |
| 1460 | 12,3353 | `SR* BDSU` | 44257 | `lla_lambloop` |
| 1461 | 12,3354 | `0 -8D,1` | 20571 | `lla_lambloop` |
| 1462 | 12,3355 | `D1/32` | 11043 | `lla_lambloop` |
| 1463 | 12,3356 | `STODL R1A` | 16742 | `lla_lambloop` |
| 1464 | 12,3357 | `P` | 02740 | `lla_lambloop` |
| 1465 | 12,3360 | `BOV CALL` | 45000 | `lla_lambloop` |
| 1466 | 12,3361 | `HIENERGY` | 25422 | `lla_lambloop` |
| 1467 | 12,3362 | `GETX` | 24767 | `lla_lambloop` |
| 1468 | 12,3363 | `DLOAD` | 77745 | `lla_aftergetx` |
| 1469 | 12,3364 | `T` | 00037 | `lla_aftergetx` |
| 1470 | 12,3365 | `STODL TPREV` | 16762 | `lla_aftergetx` |
| 1471 | 12,3366 | `XI` | 00031 | `lla_aftergetx` |
| 1472 | 12,3367 | `BON CALL` | 45014 | `lla_aftergetx` |
| 1473 | 12,3370 | `INFINFLG` | 04310 | `lla_aftergetx` |
| 1474 | 12,3371 | `NEGP` | 25417 | `lla_aftergetx` |
| 1475 | 12,3372 | `DELTIME` | 24421 | `lla_aftergetx` |
| 1476 | 12,3373 | `BOV BDSU` | 44200 | `lla_afterdt` |
| 1477 | 12,3374 | `BIGTIME` | 25437 | `lla_afterdt` |
| 1478 | 12,3375 | `TDESIRED` | 02671 | `lla_afterdt` |
| 1479 | 12,3376 | `STORE TERRLAMB` | 02760 | `lla_afterdt` |
| 1480 | 12,3377 | `ABS BDSU` | 44246 | `lla_afterdt` |
| 1481 | 12,3400 | `EPSILONL` | 02764 | `lla_afterdt` |
| 1482 | 12,3401 | `BPL RTB` | 47044 | `lla_afterdt` |
| 1483 | 12,3402 | `INITV` | 25506 | `lla_afterdt` |
| 1484 | 12,3403 | `CHECKCTR` | 24644 | `lla_afterdt` |
| 1485 | 12,3404 | `BHIZ CALL` | 45030 | `lla_afterdt` |
| 1486 | 12,3405 | `SUFFCHEK` | 25450 | `lla_afterdt` |
| 1487 | 12,3406 | `ITERATOR` | 24536 | `lla_afterdt` |
| 1488 | 12,3407 | `DLOAD BZE` | 53145 | `lla_afteriter` |
| 1489 | 12,3410 | `MPAC` | 00155 | `lla_afteriter` |
| 1490 | 12,3411 | `SUFFCHEK` | 25450 | `lla_afteriter` |
| 1491 | 12,3412 | `DAD` | 77615 | `lla_afteriter` |
| 1492 | 12,3413 | `COGA` | 02766 | `lla_afteriter` |
| 1493 | 12,3414 | `STORE COGA` | 02766 | `lla_afteriter` |
| 1494 | 12,3415 | `GOTO` | 77650 | `lla_afteriter` |
| 1495 | 12,3416 | `LAMBLOOP` | 25330 | `lla_afteriter` |
| 1497 | 12,3417 | `NEGP DLOAD BPL` | 51145 | `lla_negp` |
| 1498 | 12,3420 | `DCOGA` | 00015 | `lla_negp` |
| 1499 | 12,3421 | `LOENERGY` | 25442 | `lla_negp` |
| 1501 | 12,3422 | `HIENERGY SETPD DLOAD` | 71201 | `lla_hienergy` |
| 1502 | 12,3423 | `0` | 00001 | `lla_hienergy` |
| 1503 | 12,3424 | `COGA` | 02766 | `lla_hienergy` |
| 1504 | 12,3425 | `STORE COGAMIN` | 00011 | `lla_hienergy` |
| 1505 | 12,3426 | `COMMONLM DLOAD SR1` | 70545 | `lla_commonlm` |
| 1506 | 12,3427 | `DCOGA` | 00015 | `lla_commonlm` |
| 1508 | 12,3430 | `STORE DCOGA` | 00015 | `lla_commonlm` |
| 1509 | 12,3431 | `BZE BDSU` | 44254 | `lla_commonlm` |
| 1510 | 12,3432 | `SUFFCHEK` | 25450 | `lla_commonlm` |
| 1511 | 12,3433 | `COGA` | 02766 | `lla_commonlm` |
| 1512 | 12,3434 | `STORE COGA` | 02766 | `lla_commonlm` |
| 1513 | 12,3435 | `GOTO` | 77650 | `lla_commonlm` |
| 1514 | 12,3436 | `LAMBLOOP` | 25330 | `lla_commonlm` |
| 1516 | 12,3437 | `BIGTIME DLOAD` | 77745 | `lla_bigtime` |
| 1517 | 12,3440 | `TPREV` | 02762 | `lla_bigtime` |
| 1518 | 12,3441 | `STORE T` | 00037 | `lla_bigtime` |
| 1520 | 12,3442 | `LOENERGY SETPD DLOAD` | 71201 | `lla_loenergy` |
| 1521 | 12,3443 | `0` | 00001 | `lla_loenergy` |
| 1522 | 12,3444 | `COGA` | 02766 | `lla_loenergy` |
| 1523 | 12,3445 | `STORE COGAMAX` | 00017 | `lla_loenergy` |
| 1524 | 12,3446 | `GOTO` | 77650 | `lla_loenergy` |
| 1525 | 12,3447 | `COMMONLM` | 25426 | `lla_loenergy` |
| 1527 | 12,3450 | `SUFFCHEK DLOAD ABS` | 51545 | `lla_suffchek` |
| 1528 | 12,3451 | `TERRLAMB` | 02760 | `lla_suffchek` |
| 1529 | 12,3452 | `PDDL DMP` | 41325 | `lla_suffchek` |
| 1530 | 12,3453 | `TDESIRED` | 02671 | `lla_suffchek` |
| 1531 | 12,3454 | `BEE17` | 11030 | `lla_suffchek` |
| 1532 | 12,3455 | `DAD DSU` | 45215 | `lla_suffchek` |
| 1533 | 12,3456 | `ONEBIT` | 11057 | `lla_suffchek` |
| 1534 | 12,3457 | `BPL SETGO` | 43044 | `lla_suffchek` |
| 1535 | 12,3460 | `INITV` | 25506 | `lla_suffchek` |
| 1536 | 12,3461 | `SOLNSW` | 02434 | `lla_suffchek` |
| 1537 | 12,3462 | `INITV` | 25506 | `lla_suffchek` |
| 1538 | 12,3463 | `360LAMB SETPD SETGO` | 43001 | `lla_360lamb` |
| 1539 | 12,3464 | `0` | 00001 | `lla_360lamb` |
| 1540 | 12,3465 | `SOLNSW` | 02434 | `lla_360lamb` |
| 1541 | 12,3466 | `RTNLAMB` | 02710 | `lla_360lamb` |
| 1543 | 12,3467 | `NOGUESS SSP DLOAD` | 71331 | `lla_noguess` |
| 1544 | 12,3470 | `TWEEKIT` | 00051 | `lla_noguess` |
| 1545 | 12,3471 | `20000` | 10000 | `lla_noguess` |
| 1546 | 12,3472 | `COGAMIN` | 00011 | `lla_noguess` |
| 1547 | 12,3473 | `SR1 PDDL` | 65342 | `lla_noguess` |
| 1548 | 12,3474 | `COGAMAX` | 00017 | `lla_noguess` |
| 1549 | 12,3475 | `SR1 DAD` | 43342 | `lla_noguess` |
| 1550 | 12,3476 | `STADR` | 77626 | `lla_noguess` |
| 1551 | 12,3477 | `STORE COGA` | 75011 | `lla_noguess` |
| 1552 | 12,3500 | `STORE DCOGA` | 00015 | `lla_noguess` |
| 1553 | 12,3501 | `GOTO` | 77650 | `lla_noguess` |
| 1555 | 12,3502 | `LAMBLOOP` | 25330 | `lla_noguess` |
| 1557 | 12,3503 | `LOLIM DLOAD GOTO` | 52145 | `lla_lolim` |
| 1558 | 12,3504 | `COGLOLIM` | 11063 | `lla_lolim` |
| 1559 | 12,3505 | `MINCOGA` | 25320 | `lla_lolim` |
| 1561 | 12,3506 | `INITV DLOAD NORM` | 60345 | `lla_initv` |
| 1562 | 12,3507 | `R1` | 00041 | `lla_initv` |
| 1563 | 12,3510 | `X1` | 00047 | `lla_initv` |
| 1564 | 12,3511 | `PDDL SR1` | 70525 | `lla_initv` |
| 1565 | 12,3512 | `P` | 02740 | `lla_initv` |
| 1566 | 12,3513 | `DDV` | 77671 | `lla_initv` |
| 1567 | 12,3514 | `SL* SQRT` | 75457 | `lla_initv` |
| 1568 | 12,3515 | `0 -4,1` | 20175 | `lla_initv` |
| 1569 | 12,3516 | `DMP SL1` | 72405 | `lla_initv` |
| 1570 | 12,3517 | `ROOTMU` | 00021 | `lla_initv` |
| 1571 | 12,3520 | `PUSH DMP` | 41206 | `lla_initv` |
| 1572 | 12,3521 | `COGA` | 02766 | `lla_initv` |
| 1573 | 12,3522 | `SL VXSC` | 74261 | `lla_initv` |
| 1574 | 12,3523 | `5` | 20206 | `lla_initv` |
| 1575 | 12,3524 | `UR1` | 02722 | `lla_initv` |
| 1576 | 12,3525 | `PDDL` | 77725 | `lla_initv` |
| 1577 | 12,3526 | `VXSC VSL1` | 76561 | `lla_initv` |
| 1578 | 12,3527 | `UN` | 02674 | `lla_initv` |
| 1579 | 12,3530 | `VXV VAD` | 53235 | `lla_initv` |
| 1580 | 12,3531 | `UR1` | 02722 | `lla_initv` |
| 1581 | 12,3532 | `VSL1` | 77772 | `lla_initv` |
| 1582 | 12,3533 | `STORE VVEC` | 02744 | `lla_initv` |
| 1583 | 12,3534 | `SLOAD BZE` | 53135 | `lla_initv` |
| 1584 | 12,3535 | `VTARGTAG` | 02702 | `lla_initv` |
| 1585 | 12,3536 | `TARGETV` | 25541 | `lla_initv` |
| 1586 | 12,3537 | `GOTO` | 77650 | `lla_initv` |
| 1587 | 12,3540 | `RTNLAMB` | 02710 | `lla_initv` |
| 1589 | 12,3541 | `TARGETV DLOAD CALL` | 45145 | `lla_targetv` |
| 1590 | 12,3542 | `MAGVEC2` | 02720 | `lla_targetv` |
| 1591 | 12,3543 | `LAMENTER` | 24701 | `lla_targetv` |
| 1592 | 12,3544 | `STORE VTARGET` | 02703 | `lla_targetv` |
| 1593 | 12,3545 | `GOTO` | 77650 | `lla_targetv` |
| 1594 | 12,3546 | `RTNLAMB` | 02710 | `lla_targetv` |

## Evidence

Hashes: `evidence/lm-conic-lambert/SHA256.json`. Bulk files are assets of the
`evidence-lm-conic-lambert` release
(`python3 tools/evidence.py fetch lm-conic-lambert`).

- Source-to-native map and independent assembly comparison:
  `python3 tools/lm_conic_lambert_source.py` reassembles the unchanged file
  with the pinned yaYUL (rope equal to the certified fixed-data rope), maps
  all 316 words of the ranges to native literals in 30 functions, and checks
  that every other word of the file is listed by a current Kepler,
  time-theta or time-radius source map or is a CONICS1 constant.
- Oracle: `tools/lm_conic_lambert_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant over 65536
  sampled entries (`tests/lm_conic_lambert_cases.c`), with both IRQ settings,
  SUPER, EBANK, all five VACs, PUSHLOC, MODE and OVFIND.
  `tools/lm_conic_lambert_differential.py` checks the native producer against
  every entry and pins the stream identity
  (`lm-conic-lambert-source-stream.json`, SHA256 `9e590783...68a820`,
  269,090,816 bytes). LAMBERT returns on 60552 entries and reaches the 1302
  abort (a SQRT in the COGA bounds or INITV) on 4984. All 77 phase
  transitions taken are frozen.
- Host and sanitizer results: `make test-lm-conic-lambert
  sanitize-lm-conic-lambert`. Strict and ASAN/UBSAN builds each pass 11
  frozen entries, 24 admission guards, 176 frozen phase steps and the full
  65536-entry stream (`lm-conic-lambert-checks.log`).
- Phase frontiers: `tools/freeze_lm_conic_lambert_phases.py` requires each of
  the 717 native frontiers of the 11 frozen entries (which together take all
  77 transitions) to equal, in order, an original NEWOPS image of the oracle,
  then freezes 176 steps in nine groups.
- C64 execution results: `make vice-lm-conic-lambert`. Nine PRGs each link one
  phase group and keep both images in the RAM under KERNAL ROM (`$01=$35`),
  starting each step from a debris-only base state. In VICE (PAL, stock ROMs)
  they pass 17, 16, 16, 12, 26, 12, 33, 7 and 37 steps with no failures
  (`lm-conic-lambert-results.json`).

## Limits

- The C64 check runs each phase separately; the whole chains run on the host.
- Qualified from Luminary's one LAMBERT call (INITVEL through SETITCTR).
- Numerical validity is the original's; no widened claim.
