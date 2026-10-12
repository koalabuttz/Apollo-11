# Translate Luminary time-theta subroutine (TIMETHET, PARAM, GEOM, GETX, NEWSTATE)

Lane B. Milestone `lm-conic-timethet`.

## Scope

The time-theta subroutine of `Luminary099/CONIC_SUBROUTINES.agc`: bank 12
`02653`-`03211` (NEWSTATE/LAMENTER, TIMETHET, COMMNOUT, COGAOVFL, GETX with
WLOOP, POLYCOEF, XCOMMON, RESETX2, 360CHECK, INVRSEQN, 1/WLOOP, TRUE360X and
INFINITY), ABTCONIC `12,3742`-`3744` and the bank-4 PARAM/GEOM block
`4,3064`-`3163`: 290 assembled words on 283 source lines. DELTIME is the Kepler
milestone's (`src/lm_conic_kepler.c`), called through its `lck_deltime*`
steps. ITERATOR, TIMERAD, APSIDES, LAMBERT and the remaining constants are not
yet ported, so the file stays `partial`.

`lm_conic_timethet(state, io)` runs from the original interpretive entry
frontier a `CALL TIMETHET` or `STCALL ... TIMETHET` leaves (NEWOPS, LOC
`02732`, FBANK 12) to the caller's NEWOPS frontier at C(RTNTT). It returns 1
there, 3 at ABTCONIC's POODOO frontier (alarm 607, COGAFLAG or INFINFLG set),
2 at the UNIT/SQRT abort frontier (alarm 1302), or 0 with the image unchanged
when the entry is outside the contract. Admitted entries:

- canonical 2048-word image, coherent EB/FB/BB with BANKSET = BB, INTBIT15 and
  EDOP `+0`, all words 15-bit, Z and NEWJOB zero, one of the five VACs;
- X1 `-2` (Earth) or `-10` (Moon), as the callers' VECSHIFT leaves it;
- QPRET one of the six callers' returns: `064320` (P20-P25 `32,2317`),
  `070250`, `070363`, `071171` (P32-P35 `34,2247`, `34,2362`, `34,3170`),
  `072664` (`35,2663`) and `071600` (STABLE_ORBIT `34,3577`). The cases sample
  all six, with RVSW both set (time only: `BON RVSW` returns before NEWSTATE) and clear (time and new state).

PARAM clears NORMSW, so HAVENORM (`4,3162`) is compiled but unreachable from
TIMETHET (LAMBERT reaches it). No sampled entry reaches the 1302 abort.

### How it is translated

Every interpretive pair is compiled with the general Luminary interpretive
bodies (`src/lm_interp_orders.c`) and the dispatch paths of
`src/lm_interp_steps.c` (Lane A). Three new general bodies were needed:
ITA (STQ), TIX and TLOAD, each checked against yaAGC with all earlier bodies
by `tools/lmi_body_differential.py` (59 bodies, 2000 cases each when added).
Each call passes its source line's order and operand words as literals.
MUTABLE comes from the certified fixed-data table; D1/4 through D1/256, 2PISC,
the VLOAD* code (`040001` at `06107`) and GETX's POLY table are literals.

The routine is split into 27 steps between original NEWOPS frontiers
(`enum lm_conic_timethet_phase`, `lm_conic_timethet_step`, the `ltt_*` entry
points); the image carries every continuation (LOC, X1/X2, QPRET, RTNTT,
RTNPRM, the push-down list). The longest sampled entry runs 26 steps; the
guard is 200.

Original details the translation reproduces: STQ RTNTT and STQ RTNPRM save
the return CADRs through ITA; `GOTO RTNPRM`, `RVQ` and `GOTO RTNTT` go
through GOTOERS to an erasable CADR; `STOVL* COGA MUTABLE,1` stores, then
loads through the indexed VLOAD* path; GETX's loops count X2 down with TIX;
POLYCOEF leaves the interpreter, runs POLY in basic code and re-enters with
`TC INTPRET`; plain (unindexed) `SR` and `SRR` take the direct GSHIFT
address path, the starred forms the indexed one.

### Interpreter ABI

ITA, TIX and TLOAD are new shared interpreter code in `lm_interp_orders`.
Lane A owns the interpreter ABI and should review them; they add no runtime
entry points.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_conic_timethet_source.py`,
`evidence/lm-conic-timethet/lm-conic-timethet-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| 1016 | 12,2653 | `NEWSTATE DLOAD SR4R` | 44545 | `ltt_newstate` |
| 1017 | 12,2654 | `R1` | 00041 | `ltt_newstate` |
| 1018 | 12,2655 | `DSU VXSC` | 74225 | `ltt_newstate` |
| 1019 | 12,2656 | `XSQC(XI)` | 00035 | `ltt_newstate` |
| 1020 | 12,2657 | `UR1` | 02722 | `ltt_newstate` |
| 1021 | 12,2660 | `VSL1 PDDL` | 65372 | `ltt_newstate` |
| 1022 | 12,2661 | `X` | 00025 | `ltt_newstate` |
| 1023 | 12,2662 | `DSQ NORM` | 60316 | `ltt_newstate` |
| 1024 | 12,2663 | `X1` | 00047 | `ltt_newstate` |
| 1025 | 12,2664 | `DMPR DMPR` | 57275 | `ltt_newstate` |
| 1026 | 12,2665 | `1/ROOTMU` | 00023 | `ltt_newstate` |
| 1027 | 12,2666 | `X` | 00025 | `ltt_newstate` |
| 1028 | 12,2667 | `DMP SRR*` | 53605 | `ltt_newstate` |
| 1029 | 12,2670 | `S(XI)` | 00033 | `ltt_newstate` |
| 1030 | 12,2671 | `0 -7,1` | 21572 | `ltt_newstate` |
| 1031 | 12,2672 | `BDSU` | 77621 | `ltt_newstate` |
| 1032 | 12,2673 | `T` | 00037 | `ltt_newstate` |
| 1033 | 12,2674 | `SL1 VXSC` | 74352 | `ltt_newstate` |
| 1034 | 12,2675 | `VVEC` | 02744 | `ltt_newstate` |
| 1035 | 12,2676 | `VSL1 VAD` | 53372 | `ltt_newstate` |
| 1036 | 12,2677 | `VSL4 PUSH` | 41512 | `ltt_newstate` |
| 1037 | 12,2700 | `ABVAL` | 77646 | `ltt_newstate` |
| 1038 | 12,2701 | `LAMENTER NORM` | 77701 | `ltt_newstate` |
| 1039 | 12,2702 | `X1` | 00047 | `ltt_newstate` |
| 1040 | 12,2703 | `STODL R2` | 16720 | `ltt_newstate` |
| 1041 | 12,2704 | `XI` | 00031 | `ltt_newstate` |
| 1042 | 12,2705 | `DMP DSU` | 45205 | `ltt_newstate` |
| 1043 | 12,2706 | `S(XI)` | 00033 | `ltt_newstate` |
| 1044 | 12,2707 | `D1/128` | 11033 | `ltt_newstate` |
| 1045 | 12,2710 | `DMP SL1R` | 76405 | `ltt_newstate` |
| 1046 | 12,2711 | `ROOTMU` | 00021 | `ltt_newstate` |
| 1047 | 12,2712 | `DMP SLR*` | 53605 | `ltt_newstate` |
| 1048 | 12,2713 | `X` | 00025 | `ltt_newstate` |
| 1049 | 12,2714 | `0 -3,1` | 21176 | `ltt_newstate` |
| 1050 | 12,2715 | `DDV VXSC` | 74271 | `ltt_newstate` |
| 1051 | 12,2716 | `R2` | 02720 | `ltt_newstate` |
| 1052 | 12,2717 | `UR1` | 02722 | `ltt_newstate` |
| 1053 | 12,2720 | `VSL1 PDDL` | 65372 | `ltt_newstate` |
| 1054 | 12,2721 | `XSQC(XI)` | 00035 | `ltt_newstate` |
| 1055 | 12,2722 | `SLR* DDV` | 56257 | `ltt_newstate` |
| 1056 | 12,2723 | `0 -4,1` | 21175 | `ltt_newstate` |
| 1057 | 12,2724 | `R2` | 02720 | `ltt_newstate` |
| 1058 | 12,2725 | `BDSU` | 77621 | `ltt_newstate` |
| 1059 | 12,2726 | `D1/256` | 11047 | `ltt_newstate` |
| 1060 | 12,2727 | `VXSC VAD` | 53361 | `ltt_newstate` |
| 1061 | 12,2730 | `VVEC` | 02744 | `ltt_newstate` |
| 1062 | 12,2731 | `VSL8 RVQ` | 43412 | `ltt_newstate` |
| 1094 | 12,2732 | `TIMETHET STQ SETPD` | 40220 | `ltt_entry` |
| 1095 | 12,2733 | `RTNTT` | 02710 | `ltt_entry` |
| 1096 | 12,2734 | `0` | 00001 | `ltt_entry` |
| 1097 | 12,2735 | `BOV` | 77600 | `ltt_entry` |
| 1098 | 12,2736 | `+1` | 24737 | `ltt_entry` |
| 1099 | 12,2737 | `VLOAD PDVL` | 63375 | `ltt_entry` |
| 1100 | 12,2740 | `RVEC` | 02655 | `ltt_entry` |
| 1101 | 12,2741 | `VVEC` | 02744 | `ltt_entry` |
| 1102 | 12,2742 | `CALL` | 77624 | `ltt_entry` |
| 1103 | 12,2743 | `PARAM` | 11064 | `ltt_entry` |
| 1104 | 12,2744 | `BOV CALL` | 45000 | `ltt_afterparam` |
| 1105 | 12,2745 | `COGAOVFL` | 24764 | `ltt_afterparam` |
| 1106 | 12,2746 | `GETX` | 24767 | `ltt_afterparam` |
| 1107 | 12,2747 | `COMMNOUT DLOAD BON` | 43145 | `ltt_commnout` |
| 1108 | 12,2750 | `XI` | 00031 | `ltt_commnout` |
| 1109 | 12,2751 | `INFINFLG` | 04310 | `ltt_commnout` |
| 1110 | 12,2752 | `ABTCONIC` | 25742 | `ltt_commnout` |
| 1111 | 12,2753 | `CLEAR CALL` | 45014 | `ltt_commnout` |
| 1112 | 12,2754 | `COGAFLAG` | 04273 | `ltt_commnout` |
| 1113 | 12,2755 | `DELTIME` | 24421 | `ltt_commnout` |
| 1114 | 12,2756 | `BON CALL` | 45014 | `ltt_afterdt` |
| 1115 | 12,2757 | `RVSW` | 03706 | `ltt_afterdt` |
| 1116 | 12,2760 | `RTNTT` | 02710 | `ltt_afterdt` |
| 1117 | 12,2761 | `NEWSTATE` | 24653 | `ltt_afterdt` |
| 1118 | 12,2762 | `GOTO` | 77650 | `ltt_final` |
| 1119 | 12,2763 | `RTNTT` | 02710 | `ltt_final` |
| 1121 | 12,2764 | `COGAOVFL SETGO` | 77614 | `ltt_cogaovfl` |
| 1122 | 12,2765 | `COGAFLAG` | 04033 | `ltt_cogaovfl` |
| 1123 | 12,2766 | `ABTCONIC` | 25742 | `ltt_cogaovfl` |
| 1129 | 04,3064 | `PARAM STQ CLEAR` | 43020 | `ltt_param` |
| 1130 | 04,3065 | `RTNPRM` | 02753 | `ltt_param` |
| 1131 | 04,3066 | `NORMSW` | 03665 | `ltt_param` |
| 1132 | 04,3067 | `CLEAR` | 77614 | `ltt_param` |
| 1133 | 04,3070 | `COGAFLAG` | 04273 | `ltt_param` |
| 1134 | 04,3071 | `SSP CALL` | 45131 | `ltt_param` |
| 1135 | 04,3072 | `GEOMSGN` | 02673 | `ltt_param` |
| 1136 | 04,3073 | `37777` | 27777 | `ltt_param` |
| 1137 | 04,3074 | `GEOM` | 11130 | `ltt_param` |
| 1138 | 04,3075 | `STODL 36D` | 14045 | `ltt_param2` |
| 1139 | 04,3076 | `SR DDV` | 56261 | `ltt_param2` |
| 1141 | 04,3077 | `5` | 20606 | `ltt_param2` |
| 1142 | 04,3100 | `36D` | 00045 | `ltt_param2` |
| 1143 | 04,3101 | `STOVL* COGA` | 32766 | `ltt_param2` |
| 1144 | 04,3102 | `MUTABLE,1` | 10005 | `ltt_param2` |
| 1145 | 04,3103 | `STODL 1/MU` | 14017 | `ltt_param2` |
| 1146 | 04,3104 | `MAGVEC2` | 02720 | `ltt_param2` |
| 1147 | 04,3105 | `DSQ NORM` | 60316 | `ltt_param2` |
| 1148 | 04,3106 | `X1` | 00047 | `ltt_param2` |
| 1149 | 04,3107 | `DMPR DMP` | 41275 | `ltt_param2` |
| 1150 | 04,3110 | `1/MU` | 00017 | `ltt_param2` |
| 1151 | 04,3111 | `R1` | 00041 | `ltt_param2` |
| 1152 | 04,3112 | `SRR*` | 77657 | `ltt_param2` |
| 1153 | 04,3113 | `0 -3,1` | 21576 | `ltt_param2` |
| 1154 | 04,3114 | `PUSH BDSU` | 44206 | `ltt_param2` |
| 1155 | 04,3115 | `D1/32` | 11043 | `ltt_param2` |
| 1156 | 04,3116 | `STODL R1A` | 16742 | `ltt_param2` |
| 1158 | 04,3117 | `DMP NORM` | 60205 | `ltt_param2` |
| 1159 | 04,3120 | `36D` | 00045 | `ltt_param2` |
| 1160 | 04,3121 | `X1` | 00047 | `ltt_param2` |
| 1161 | 04,3122 | `DMP SR*` | 53605 | `ltt_param2` |
| 1162 | 04,3123 | `36D` | 00045 | `ltt_param2` |
| 1163 | 04,3124 | `0 -4,1` | 20575 | `ltt_param2` |
| 1164 | 04,3125 | `STORE P` | 02740 | `ltt_param2` |
| 1165 | 04,3126 | `GOTO` | 77650 | `ltt_param2` |
| 1166 | 04,3127 | `RTNPRM` | 02753 | `ltt_param2` |
| 1169 | 04,3130 | `GEOM UNIT` | 77656 | `ltt_geom` |
| 1170 | 04,3131 | `STODL U2` | 16712 | `ltt_geom` |
| 1171 | 04,3132 | `36D` | 00045 | `ltt_geom` |
| 1172 | 04,3133 | `STOVL MAGVEC2` | 26720 | `ltt_geom` |
| 1173 | 04,3134 | `UNIT` | 77656 | `ltt_geom` |
| 1174 | 04,3135 | `STORE UR1` | 02722 | `ltt_geom` |
| 1175 | 04,3136 | `DOT SL1` | 72441 | `ltt_geom` |
| 1176 | 04,3137 | `U2` | 02712 | `ltt_geom` |
| 1177 | 04,3140 | `PDDL` | 77725 | `ltt_geom` |
| 1178 | 04,3141 | `36D` | 00045 | `ltt_geom` |
| 1179 | 04,3142 | `STOVL R1` | 24041 | `ltt_geom` |
| 1180 | 04,3143 | `UR1` | 02722 | `ltt_geom` |
| 1181 | 04,3144 | `VXV VSL1` | 76435 | `ltt_geom` |
| 1182 | 04,3145 | `U2` | 02712 | `ltt_geom` |
| 1183 | 04,3146 | `BON SIGN` | 75214 | `ltt_geom` |
| 1184 | 04,3147 | `NORMSW` | 03705 | `ltt_geom` |
| 1185 | 04,3150 | `HAVENORM` | 11162 | `ltt_geom` |
| 1186 | 04,3151 | `GEOMSGN` | 02673 | `ltt_geom` |
| 1187 | 04,3152 | `UNIT BOV` | 40056 | `ltt_geom` |
| 1188 | 04,3153 | `COLINEAR` | 11160 | `ltt_geom` |
| 1189 | 04,3154 | `UNITNORM STODL UN` | 16674 | `ltt_unitnorm` |
| 1190 | 04,3155 | `36D` | 00045 | `ltt_unitnorm` |
| 1191 | 04,3156 | `SIGN RVQ` | 43565 | `ltt_unitnorm` |
| 1192 | 04,3157 | `GEOMSGN` | 02673 | `ltt_unitnorm` |
| 1194 | 04,3160 | `COLINEAR VSR1 GOTO` | 52162 | `ltt_colinear` |
| 1195 | 04,3161 | `UNITNORM` | 11154 | `ltt_colinear` |
| 1197 | 04,3162 | `HAVENORM ABVAL SIGN` | 75246 | `ltt_havenorm` |
| 1198 | 04,3163 | `GEOMSGN` | 02673 | `ltt_havenorm` |
| 1207 | 12,2767 | `GETX AXT,2 SSP` | 66374 | `ltt_getx` |
| 1208 | 12,2770 | `3` | 00003 | `ltt_getx` |
| 1209 | 12,2771 | `S2` | 00052 | `ltt_getx` |
| 1210 | 12,2772 | `1` | 00001 | `ltt_getx` |
| 1211 | 12,2773 | `CLEAR` | 77614 | `ltt_getx` |
| 1212 | 12,2774 | `360SW` | 04276 | `ltt_getx` |
| 1213 | 12,2775 | `SQRT PDDL` | 65366 | `ltt_getx` |
| 1214 | 12,2776 | `CSTH` | 02732 | `ltt_getx` |
| 1215 | 12,2777 | `SR1 BDSU` | 44342 | `ltt_getx` |
| 1216 | 12,3000 | `D1/4` | 11037 | `ltt_getx` |
| 1217 | 12,3001 | `PDDL SRR` | 54325 | `ltt_getx` |
| 1218 | 12,3002 | `SNTH` | 02730 | `ltt_getx` |
| 1219 | 12,3003 | `6` | 21607 | `ltt_getx` |
| 1220 | 12,3004 | `DDV` | 77671 | `ltt_getx` |
| 1221 | 12,3005 | `BOV` | 77600 | `ltt_getx` |
| 1222 | 12,3006 | `360CHECK` | 25122 | `ltt_getx` |
| 1223 | 12,3007 | `DSU DMP` | 41225 | `ltt_getx` |
| 1224 | 12,3010 | `COGA` | 02766 | `ltt_getx` |
| 1225 | 12,3011 | `SL2R BOV` | 40132 | `ltt_getx` |
| 1226 | 12,3012 | `360CHECK` | 25122 | `ltt_getx` |
| 1227 | 12,3013 | `WLOOP PUSH DSQ` | 63406 | `ltt_wloop` |
| 1228 | 12,3014 | `TLOAD PDDL` | 65351 | `ltt_wloop` |
| 1229 | 12,3015 | `MPAC` | 00155 | `ltt_wloop` |
| 1230 | 12,3016 | `R1A` | 02742 | `ltt_wloop` |
| 1231 | 12,3017 | `SR4 TAD` | 76202 | `ltt_wloop` |
| 1232 | 12,3020 | `BMN SQRT` | 75440 | `ltt_wloop` |
| 1233 | 12,3021 | `INFINITY` | 25205 | `ltt_wloop` |
| 1234 | 12,3022 | `ROUND DAD` | 43306 | `ltt_wloop` |
| 1235 | 12,3023 | `BOV TIX,2` | 61000 | `ltt_wloop` |
| 1236 | 12,3024 | `RESETX2` | 25120 | `ltt_wloop` |
| 1237 | 12,3025 | `WLOOP` | 25013 | `ltt_wloop` |
| 1239 | 12,3026 | `BDDV BOV` | 40065 | `ltt_wloop` |
| 1240 | 12,3027 | `D1/128` | 11033 | `ltt_wloop` |
| 1241 | 12,3030 | `INFINITY` | 25205 | `ltt_wloop` |
| 1242 | 12,3031 | `POLYCOEF BMN PUSH` | 41440 | `ltt_polycoef` |
| 1243 | 12,3032 | `INFINITY` | 25205 | `ltt_polycoef` |
| 1244 | 12,3033 | `DSQ` | 77716 | `ltt_polycoef` |
| 1245 | 12,3034 | `NORM DMP` | 41301 | `ltt_polycoef` |
| 1246 | 12,3035 | `X1` | 00047 | `ltt_polycoef` |
| 1247 | 12,3036 | `R1A` | 02742 | `ltt_polycoef` |
| 1248 | 12,3037 | `SRR* EXIT` | 77457 | `ltt_polycoef` |
| 1249 | 12,3040 | `0 -10D,1` | 21567 | `ltt_polycoef` |
| 1250 | 12,3041 | `TC POLY` | 07222 | `ltt_polycoef` |
| 1252 | 12,3042 | `DEC 5` | 00005 | `ltt_polycoef` |
| 1253 | 12,3043 | `2DEC .5` | 20000 00000 | `ltt_polycoef` |
| 1254 | 12,3045 | `2DEC -.166666770` | 72525 52471 | `ltt_polycoef` |
| 1255 | 12,3047 | `2DEC .100000392` | 03146 15003 | `ltt_polycoef` |
| 1256 | 12,3051 | `2DEC -.071401086` | 75556 45210 | `ltt_polycoef` |
| 1257 | 12,3053 | `2DEC .055503292` | 01615 13553 | `ltt_polycoef` |
| 1258 | 12,3055 | `2DEC -.047264098` | 76371 63777 | `ltt_polycoef` |
| 1259 | 12,3057 | `2DEC .040694204` | 01232 27367 | `ltt_polycoef` |
| 1261 | 12,3061 | `TC INTPRET` | 06037 | `ltt_polycoef` |
| 1262 | 12,3062 | `DMP SL1R` | 76405 | `ltt_polycoef` |
| 1263 | 12,3063 | `PUSH BON` | 43006 | `ltt_polycoef` |
| 1264 | 12,3064 | `360SW` | 04316 | `ltt_polycoef` |
| 1265 | 12,3065 | `TRUE360X` | 25172 | `ltt_polycoef` |
| 1266 | 12,3066 | `XCOMMON DSQ NORM` | 60316 | `ltt_xcommon` |
| 1267 | 12,3067 | `X1` | 00047 | `ltt_xcommon` |
| 1268 | 12,3070 | `DMP SRR*` | 53605 | `ltt_xcommon` |
| 1269 | 12,3071 | `R1A` | 02742 | `ltt_xcommon` |
| 1270 | 12,3072 | `0 -12D,1` | 21565 | `ltt_xcommon` |
| 1271 | 12,3073 | `STODL XI` | 14031 | `ltt_xcommon` |
| 1272 | 12,3074 | `R1` | 00041 | `ltt_xcommon` |
| 1273 | 12,3075 | `SR1 SQRT` | 75542 | `ltt_xcommon` |
| 1274 | 12,3076 | `ROUND DMP` | 41306 | `ltt_xcommon` |
| 1275 | 12,3077 | `SL4R` | 77632 | `ltt_xcommon` |
| 1276 | 12,3100 | `STORE X` | 00025 | `ltt_xcommon` |
| 1278 | 12,3101 | `DSQ NORM` | 60316 | `ltt_xcommon` |
| 1279 | 12,3102 | `X1` | 00047 | `ltt_xcommon` |
| 1280 | 12,3103 | `PDDL DMP` | 41325 | `ltt_xcommon` |
| 1281 | 12,3104 | `P` | 02740 | `ltt_xcommon` |
| 1282 | 12,3105 | `R1` | 00041 | `ltt_xcommon` |
| 1283 | 12,3106 | `SL3 SQRT` | 75452 | `ltt_xcommon` |
| 1284 | 12,3107 | `DMP SL3R` | 56405 | `ltt_xcommon` |
| 1285 | 12,3110 | `COGA` | 02766 | `ltt_xcommon` |
| 1286 | 12,3111 | `STODL KEPC1` | 14043 | `ltt_xcommon` |
| 1287 | 12,3112 | `R1A` | 02742 | `ltt_xcommon` |
| 1288 | 12,3113 | `BDSU CLEAR` | 43021 | `ltt_xcommon` |
| 1289 | 12,3114 | `D1/64` | 11035 | `ltt_xcommon` |
| 1290 | 12,3115 | `INFINFLG` | 04270 | `ltt_xcommon` |
| 1291 | 12,3116 | `STORE KEPC2` | 00045 | `ltt_xcommon` |
| 1292 | 12,3117 | `RVQ` | 77616 | `ltt_xcommon` |
| 1295 | 12,3120 | `RESETX2 AXT,2` | 77774 | `ltt_resetx2` |
| 1296 | 12,3121 | `3` | 00003 | `ltt_resetx2` |
| 1298 | 12,3122 | `360CHECK SETPD BPL` | 51001 | `ltt_360check` |
| 1299 | 12,3123 | `0D` | 00001 | `ltt_360check` |
| 1300 | 12,3124 | `INVRSEQN` | 25127 | `ltt_360check` |
| 1301 | 12,3125 | `SET` | 77614 | `ltt_360check` |
| 1302 | 12,3126 | `360SW` | 04076 | `ltt_360check` |
| 1304 | 12,3127 | `INVRSEQN DLOAD SQRT` | 75545 | `ltt_invrseqn` |
| 1305 | 12,3130 | `P` | 02740 | `ltt_invrseqn` |
| 1306 | 12,3131 | `PDDL DMP` | 41325 | `ltt_invrseqn` |
| 1307 | 12,3132 | `SNTH` | 02730 | `ltt_invrseqn` |
| 1308 | 12,3133 | `COGA` | 02766 | `ltt_invrseqn` |
| 1309 | 12,3134 | `SL1 PDDL` | 65352 | `ltt_invrseqn` |
| 1310 | 12,3135 | `CSTH` | 02732 | `ltt_invrseqn` |
| 1311 | 12,3136 | `SR4 DAD` | 43202 | `ltt_invrseqn` |
| 1312 | 12,3137 | `D1/32` | 11043 | `ltt_invrseqn` |
| 1313 | 12,3140 | `DSU DMP` | 41225 | `ltt_invrseqn` |
| 1314 | 12,3141 | `NORM BDDV` | 55301 | `ltt_invrseqn` |
| 1315 | 12,3142 | `X1` | 00047 | `ltt_invrseqn` |
| 1316 | 12,3143 | `SNTH` | 02730 | `ltt_invrseqn` |
| 1317 | 12,3144 | `SLR* ABS` | 51457 | `ltt_invrseqn` |
| 1318 | 12,3145 | `0 -5,1` | 21174 | `ltt_invrseqn` |
| 1319 | 12,3146 | `PUSH DSQ` | 63406 | `ltt_invrseqn` |
| 1320 | 12,3147 | `STODL 34D` | 14043 | `ltt_invrseqn` |
| 1321 | 12,3150 | `D1/16` | 11041 | `ltt_invrseqn` |
| 1322 | 12,3151 | `1/WLOOP PUSH DSQ` | 63406 | `ltt_1wloop` |
| 1323 | 12,3152 | `RTB PDDL` | 65234 | `ltt_1wloop` |
| 1324 | 12,3153 | `TPMODE` | 21633 | `ltt_1wloop` |
| 1325 | 12,3154 | `R1A` | 02742 | `ltt_1wloop` |
| 1326 | 12,3155 | `DMP SR4` | 40405 | `ltt_1wloop` |
| 1327 | 12,3156 | `34D` | 00043 | `ltt_1wloop` |
| 1328 | 12,3157 | `TAD` | 77771 | `ltt_1wloop` |
| 1329 | 12,3160 | `BMN SQRT` | 75440 | `ltt_1wloop` |
| 1330 | 12,3161 | `INFINITY` | 25205 | `ltt_1wloop` |
| 1331 | 12,3162 | `DAD` | 77615 | `ltt_1wloop` |
| 1332 | 12,3163 | `TIX,2 NORM` | 60304 | `ltt_1wloop` |
| 1333 | 12,3164 | `1/WLOOP` | 25151 | `ltt_1wloop` |
| 1334 | 12,3165 | `X1` | 00047 | `ltt_1wloop` |
| 1335 | 12,3166 | `BDDV` | 77665 | `ltt_1wloop` |
| 1336 | 12,3167 | `SLR* GOTO` | 52057 | `ltt_1wloop` |
| 1337 | 12,3170 | `0 -7,1` | 21172 | `ltt_1wloop` |
| 1338 | 12,3171 | `POLYCOEF` | 25031 | `ltt_1wloop` |
| 1340 | 12,3172 | `TRUE360X DLOAD BMN` | 50145 | `ltt_true360x` |
| 1341 | 12,3173 | `R1A` | 02742 | `ltt_true360x` |
| 1343 | 12,3174 | `INFINITY` | 25205 | `ltt_true360x` |
| 1344 | 12,3175 | `SQRT NORM` | 60366 | `ltt_true360x` |
| 1345 | 12,3176 | `X1` | 00047 | `ltt_true360x` |
| 1346 | 12,3177 | `BDDV SL*` | 53665 | `ltt_true360x` |
| 1347 | 12,3200 | `2PISC` | 11055 | `ltt_true360x` |
| 1348 | 12,3201 | `0 -3,1` | 20176 | `ltt_true360x` |
| 1349 | 12,3202 | `DSU PUSH` | 41425 | `ltt_true360x` |
| 1350 | 12,3203 | `GOTO` | 77650 | `ltt_true360x` |
| 1351 | 12,3204 | `XCOMMON` | 25066 | `ltt_true360x` |
| 1352 | 12,3205 | `INFINITY SETPD BOV` | 40001 | `ltt_infinity` |
| 1353 | 12,3206 | `0` | 00001 | `ltt_infinity` |
| 1354 | 12,3207 | `OVFLCLR` | 25210 | `ltt_infinity` |
| 1355 | 12,3210 | `OVFLCLR SET RVQ` | 43414 | `ltt_infinity` |
| 1356 | 12,3211 | `INFINFLG` | 04070 | `ltt_infinity` |
| 1728 | 12,3742 | `ABTCONIC EXIT` | 77776 | `ltt_abtconic` |
| 1729 | 12,3743 | `TC POODOO` | 05652 | `ltt_abtconic` |
| 1730 | 12,3744 | `OCT 00607` | 00607 | `ltt_abtconic` |

## Evidence

Hashes: `evidence/lm-conic-timethet/SHA256.json`. Bulk files are assets of the
`evidence-lm-conic-timethet` release
(`python3 tools/evidence.py fetch lm-conic-timethet`).

- Source-to-native map and independent assembly comparison:
  `python3 tools/lm_conic_timethet_source.py` reassembles the unchanged file
  with the pinned yaYUL (rope equal to the certified fixed-data rope) and maps
  all 290 words of the ranges to native literals in 24 functions.
- Oracle: `tools/lm_conic_timethet_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant over 65536
  sampled entries (`tests/lm_conic_timethet_cases.c`): Earth and Moon state
  vectors with speeds from well below circular to hyperbolic, near-radial
  flight paths, transfer angles near 0, 180 and 360 degrees and across both
  half-planes, raw edge words in one case in eight, all six caller returns,
  RVSW set and clear, both IRQ settings, SUPER, EBANK, all five VACs, PUSHLOC,
  MODE and OVFIND. `tools/lm_conic_timethet_differential.py` checks the native
  producer against every entry and pins the stream identity
  (`lm-conic-timethet-source-stream.json`, SHA256 `f5558c19...411b16`,
  269,090,816 bytes; 50152 return, 15384 end at ABTCONIC). Every phase but
  HAVENORM and all 38 phase transitions are reached.
- Host and sanitizer results: `make test-lm-conic-timethet
  sanitize-lm-conic-timethet`. Strict and ASAN/UBSAN builds each pass 8 frozen
  entries, 22 admission guards, 90 frozen phase steps and the full
  65536-entry stream (`lm-conic-timethet-checks.log`).
- Phase frontiers: `tools/freeze_lm_conic_timethet_phases.py` requires each of
  the 144 native frontiers of the 8 frozen entries (which together take all 38
  transitions) to equal, in order, an original NEWOPS image of the oracle,
  then freezes 90 steps in four groups.
- C64 execution results: `make vice-lm-conic-timethet`. Four PRGs each link
  one phase group and keep both images in the RAM under KERNAL ROM (`$01=$35`),
  starting each step from a debris-only base state. In VICE (PAL, stock ROMs):
  group 1 (TIMETHET, PARAM, GEOM) 22 steps, group 2 (GETX, WLOOP, POLYCOEF,
  ABTCONIC) 21 steps, group 3 (XCOMMON through INFINITY) 23 steps, group 4
  (COMMNOUT, DELTIME, NEWSTATE, the final GOTO) 24 steps, no failures
  (`lm-conic-timethet-results.json`).

## Limits

- The C64 check runs each phase separately; the whole chain runs on the host.
  Running it as one program needs overlay ABI 2 modules (the 27 phases are the
  module entry points).
- Qualified from the six TIMETHET call sites only. HAVENORM is not exercised
  here; LAMBERT's milestone will qualify it.
- Numerical validity of the time-theta computation is the original's; no
  widened claim.
