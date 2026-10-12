# Translate Luminary INITVEL (required velocity, with LAMBERT)

Lane E. Milestone `lm-initvel`.

## Scope

INITVEL from `Luminary099/P34-35_P74-75.agc`: INITVEL, HAVEGUES, INITVEL1
through INITVEL7 and INITVELX (bank 11 `02000`-`02274`, 189 words). It
computes the velocity that reaches RTARG from RINIT in DELLT4, rotating a
near-180-degree target into the plane of RINIT and VINIT. It is called by
P34's TPI and P35's midcourse logic. SETITCTR
(`Luminary099/ORBITAL_INTEGRATION.agc`, `11,3765`-`3775`, 9 words) sits at
the end of the bank only "for purposes of a one-module remanufacture"; it
is compiled inline at INITVEL's `CALL SETITCTR`. That makes 198 words on
198 source lines. LAMBERT runs unchanged as the `lm-conic-lambert`
milestone's steps (Lane B), and with them GEOM, GETX and DELTIME.
INTSTALL and INTEGRVS (bank 13, INTEGRATION_INITIALIZATION and the
coasting integration) are not ported. Their NEWOPS is an end frontier, and
INITVEL's code after each of them is qualified from the place where they
return.

`lm_initvel(state, io)` returns one of these values:

- 1 at the caller's NEWOPS frontier at C(NORMEX).
- 2 at the UNIT/SQRT abort frontier (alarm 1302, from LAMBERT).
- 3 at the NEWOPS of INTSTALL (`13,3414`) or INTEGRVS (`13,3107`). This
  happens when 0D asks for integration passes.
- 0 with the image unchanged, when the entry is outside the contract.

Admitted entries must meet all of these conditions:

- The image is a canonical 2048-word image. EB, FB and BB are coherent
  (FBANK 11, BANKSET = BB, INTBIT15 clear). EDOP is `+0`, all words are 15
  bits, ZERO and NEWJOB are zero, and the VAC is one of the five.
- RTX1/RTX2 is the Earth (`-2`, `0`) or Moon (`-10`, `2`) pair.
- NEWOPS is at INITVEL with QPRET `072503`, `073367` or `056373` (P34
  `35,2502` and `35,3366`, P35 `27,2372`), or at HAVEGUES with QPRET
  `056725` (P35 `27,2724`). In both cases PUSHLOC is 4D, as the calling
  sequence requires.
- Or NEWOPS is at `11,2155` (INTSTALL's return) or `11,2200` (INTEGRVS'
  return), with NORMEX one of those four returns and PUSHLOC within the VAC.

The cases (`tests/lm_initvel_cases.c`) work like this. RINIT and VINIT lie
on Earth or Moon conics. RTARG is where a velocity a few per cent different
(partly out of plane) reaches after DELLT4: near half a revolution, short,
or anywhere in one revolution. The cone angle E4 at 2D is small or large.
The iteration count at 0D is zero (the conic case) or 1-3. HAVEGUES gets a
COGA guess with GUESSW clear. The integration-return entries get the
integrated RATT1/VATT1 and iteration state for the last pass and for
others. Raw edge words appear on one case in eight. All of this sits over
poisoned erasable debris with every VAC, OVFIND, IRQ setting and SUPERBNK
value.

### How it is translated

Every interpretive pair is compiled with the general Luminary interpretive
bodies (`src/lm_interp_orders.c`) and the dispatch paths of
`src/lm_interp_steps.c`. COSINE uses `sband_trig`. The milestone adds two
shared bodies:

- `lmi_vproj`: VPROJ `07425`, PREDOT and DOT, then DVXSC.
- `lmi_tload_fixed`: TLOAD of a fixed TP operand, ZEROVEC.

`lmi_vxsc`'s scalar path is factored out as DVXSC for VPROJ to share. Both
new bodies pass `tools/lmi_body_differential.py`, which now has 61 bodies.
MUEARTH (bank 13), ZEROVEC (bank 12) and MUTABLE (bank 4) are read from the
certified fixed-data table at the bank and ADDRWD that address formation
leaves. The routine is ten steps between original NEWOPS frontiers
(`enum lm_initvel_phase`, `liv_*`). LAMBERT keeps its own phase numbers,
and `liv_remap` follows its `GOTO RTNLAMB` back to `11,2145`.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_initvel_source.py`,
`evidence/lm-initvel/lm-initvel-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P34-35_P74-75.agc:1124 | 11,2000 | `INITVEL SET` | 77614 | `liv_initvel` |
| P34-35_P74-75.agc:1125 | 11,2001 | `GUESSW` | 00475 | `liv_initvel` |
| P34-35_P74-75.agc:1126 | 11,2002 | `HAVEGUES VLOAD STQ` | 44175 | `liv_havegues` |
| P34-35_P74-75.agc:1127 | 11,2003 | `RTARG` | 03444 | `liv_havegues` |
| P34-35_P74-75.agc:1128 | 11,2004 | `NORMEX` | 03463 | `liv_havegues` |
| P34-35_P74-75.agc:1129 | 11,2005 | `STORE RTARG1` | 03472 | `liv_havegues` |
| P34-35_P74-75.agc:1130 | 11,2006 | `ABVAL` | 77646 | `liv_havegues` |
| P34-35_P74-75.agc:1131 | 11,2007 | `STORE RTMAG` | 03723 | `liv_havegues` |
| P34-35_P74-75.agc:1132 | 11,2010 | `SLOAD BHIZ` | 46135 | `liv_havegues` |
| P34-35_P74-75.agc:1133 | 11,2011 | `RTX2` | 03000 | `liv_havegues` |
| P34-35_P74-75.agc:1134 | 11,2012 | `INITVEL1` | 22026 | `liv_havegues` |
| P34-35_P74-75.agc:1135 | 11,2013 | `VLOAD VSL2` | 72575 | `liv_havegues` |
| P34-35_P74-75.agc:1136 | 11,2014 | `RINIT` | 02323 | `liv_havegues` |
| P34-35_P74-75.agc:1137 | 11,2015 | `STOVL RINIT` | 26323 | `liv_havegues` |
| P34-35_P74-75.agc:1138 | 11,2016 | `VINIT` | 02331 | `liv_havegues` |
| P34-35_P74-75.agc:1139 | 11,2017 | `VSL2` | 77752 | `liv_havegues` |
| P34-35_P74-75.agc:1140 | 11,2020 | `STOVL VINIT` | 26331 | `liv_havegues` |
| P34-35_P74-75.agc:1141 | 11,2021 | `RTARG1` | 03472 | `liv_havegues` |
| P34-35_P74-75.agc:1142 | 11,2022 | `VSL2` | 77752 | `liv_havegues` |
| P34-35_P74-75.agc:1143 | 11,2023 | `STORE RTARG1` | 03472 | `liv_havegues` |
| P34-35_P74-75.agc:1144 | 11,2024 | `ABVAL` | 77646 | `liv_havegues` |
| P34-35_P74-75.agc:1145 | 11,2025 | `STORE RTMAG` | 03723 | `liv_havegues` |
| P34-35_P74-75.agc:1149 | 11,2026 | `INITVEL1 SSP DLOAD` | 71331 | `liv_initvel1` |
| P34-35_P74-75.agc:1150 | 11,2027 | `ITCTR` | 03617 | `liv_initvel1` |
| P34-35_P74-75.agc:1151 | 11,2030 | `0 -1` | 77776 | `liv_initvel1` |
| P34-35_P74-75.agc:1152 | 11,2031 | `COSINE SR1` | 70546 | `liv_initvel1` |
| P34-35_P74-75.agc:1153 | 11,2032 | `STODL COZY4` | 17667 | `liv_initvel1` |
| P34-35_P74-75.agc:1154 | 11,2033 | `LXA,2 SXA,2` | 67154 | `liv_initvel1` |
| P34-35_P74-75.agc:1155 | 11,2034 | `MPAC` | 00154 | `liv_initvel1` |
| P34-35_P74-75.agc:1156 | 11,2035 | `VTARGTAG` | 02701 | `liv_initvel1` |
| P34-35_P74-75.agc:1157 | 11,2036 | `VLOAD` | 77775 | `liv_initvel1` |
| P34-35_P74-75.agc:1159 | 11,2037 | `RINIT` | 02323 | `liv_initvel1` |
| P34-35_P74-75.agc:1160 | 11,2040 | `STOVL R1VEC` | 26655 | `liv_initvel1` |
| P34-35_P74-75.agc:1161 | 11,2041 | `RTARG1` | 03472 | `liv_initvel1` |
| P34-35_P74-75.agc:1162 | 11,2042 | `STODL R2VEC` | 16663 | `liv_initvel1` |
| P34-35_P74-75.agc:1163 | 11,2043 | `DELLT4` | 03452 | `liv_initvel1` |
| P34-35_P74-75.agc:1164 | 11,2044 | `STORE TDESIRED` | 02671 | `liv_initvel1` |
| P34-35_P74-75.agc:1165 | 11,2045 | `SETPD VLOAD` | 77201 | `liv_initvel1` |
| P34-35_P74-75.agc:1166 | 11,2046 | `0D` | 00001 | `liv_initvel1` |
| P34-35_P74-75.agc:1167 | 11,2047 | `RINIT` | 02323 | `liv_initvel1` |
| P34-35_P74-75.agc:1168 | 11,2050 | `UNIT PUSH` | 41456 | `liv_initvel1` |
| P34-35_P74-75.agc:1169 | 11,2051 | `VXV UNIT` | 53435 | `liv_initvel1` |
| P34-35_P74-75.agc:1170 | 11,2052 | `VINIT` | 02331 | `liv_initvel1` |
| P34-35_P74-75.agc:1171 | 11,2053 | `STOVL UN` | 26674 | `liv_initvel1` |
| P34-35_P74-75.agc:1172 | 11,2054 | `RTARG1` | 03472 | `liv_initvel1` |
| P34-35_P74-75.agc:1173 | 11,2055 | `UNIT DOT` | 50256 | `liv_initvel1` |
| P34-35_P74-75.agc:1174 | 11,2056 | `DAD CLEAR` | 43015 | `liv_initvel1` |
| P34-35_P74-75.agc:1175 | 11,2057 | `COZY4` | 03667 | `liv_initvel1` |
| P34-35_P74-75.agc:1176 | 11,2060 | `NORMSW` | 03665 | `liv_initvel1` |
| P34-35_P74-75.agc:1177 | 11,2061 | `STORE COZY4` | 03667 | `liv_initvel1` |
| P34-35_P74-75.agc:1178 | 11,2062 | `INITVEL2 BPL SET` | 43044 | `liv_initvel2` |
| P34-35_P74-75.agc:1179 | 11,2063 | `INITVEL3` | 22105 | `liv_initvel2` |
| P34-35_P74-75.agc:1180 | 11,2064 | `NORMSW` | 03465 | `liv_initvel2` |
| P34-35_P74-75.agc:1184 | 11,2065 | `VLOAD PUSH` | 41575 | `liv_initvel2` |
| P34-35_P74-75.agc:1185 | 11,2066 | `R2VEC` | 02663 | `liv_initvel2` |
| P34-35_P74-75.agc:1186 | 11,2067 | `ABVAL PDVL` | 63246 | `liv_initvel2` |
| P34-35_P74-75.agc:1187 | 11,2070 | `PUSH VPROJ` | 46206 | `liv_initvel2` |
| P34-35_P74-75.agc:1188 | 11,2071 | `UN` | 02674 | `liv_initvel2` |
| P34-35_P74-75.agc:1189 | 11,2072 | `VSL2 BVSU` | 51352 | `liv_initvel2` |
| P34-35_P74-75.agc:1190 | 11,2073 | `UNIT VXSC` | 74256 | `liv_initvel2` |
| P34-35_P74-75.agc:1191 | 11,2074 | `VSL1` | 77772 | `liv_initvel2` |
| P34-35_P74-75.agc:1192 | 11,2075 | `STORE R2VEC` | 02663 | `liv_initvel2` |
| P34-35_P74-75.agc:1193 | 11,2076 | `TLOAD SLOAD` | 67351 | `liv_initvel2` |
| P34-35_P74-75.agc:1194 | 11,2077 | `ZEROVEC` | 24007 | `liv_initvel2` |
| P34-35_P74-75.agc:1195 | 11,2100 | `ITCTR` | 03617 | `liv_initvel2` |
| P34-35_P74-75.agc:1196 | 11,2101 | `BPL VLOAD` | 77244 | `liv_initvel2` |
| P34-35_P74-75.agc:1197 | 11,2102 | `INITVEL3` | 22105 | `liv_initvel2` |
| P34-35_P74-75.agc:1198 | 11,2103 | `R2VEC` | 02663 | `liv_initvel2` |
| P34-35_P74-75.agc:1199 | 11,2104 | `STORE RTARG1` | 03472 | `liv_initvel2` |
| P34-35_P74-75.agc:1200 | 11,2105 | `INITVEL3 DLOAD PDVL` | 63345 | `liv_initvel3` |
| P34-35_P74-75.agc:1201 | 11,2106 | `MUEARTH` | 26007 | `liv_initvel3` |
| P34-35_P74-75.agc:1202 | 11,2107 | `R2VEC` | 02663 | `liv_initvel3` |
| P34-35_P74-75.agc:1203 | 11,2110 | `UNIT PDVL` | 63256 | `liv_initvel3` |
| P34-35_P74-75.agc:1204 | 11,2111 | `R1VEC` | 02655 | `liv_initvel3` |
| P34-35_P74-75.agc:1205 | 11,2112 | `UNIT PUSH` | 41456 | `liv_initvel3` |
| P34-35_P74-75.agc:1206 | 11,2113 | `VXV VCOMP` | 57435 | `liv_initvel3` |
| P34-35_P74-75.agc:1207 | 11,2114 | `2D` | 00003 | `liv_initvel3` |
| P34-35_P74-75.agc:1208 | 11,2115 | `PUSH` | 77606 | `liv_initvel3` |
| P34-35_P74-75.agc:1209 | 11,2116 | `LXA,1 DLOAD` | 71350 | `liv_initvel3` |
| P34-35_P74-75.agc:1211 | 11,2117 | `RTX1` | 02776 | `liv_initvel3` |
| P34-35_P74-75.agc:1212 | 11,2120 | `18D` | 00023 | `liv_initvel3` |
| P34-35_P74-75.agc:1213 | 11,2121 | `BMN INCR,1` | 62040 | `liv_initvel3` |
| P34-35_P74-75.agc:1214 | 11,2122 | `+2` | 22124 | `liv_initvel3` |
| P34-35_P74-75.agc:1215 | 11,2123 | `DEC -8` | 77767 | `liv_initvel3` |
| P34-35_P74-75.agc:1216 | 11,2124 | `INCR,1 SLOAD` | 67310 | `liv_initvel3` |
| P34-35_P74-75.agc:1217 | 11,2125 | `10D` | 00012 | `liv_initvel3` |
| P34-35_P74-75.agc:1218 | 11,2126 | `X1` | 00047 | `liv_initvel3` |
| P34-35_P74-75.agc:1219 | 11,2127 | `BHIZ VLOAD` | 77230 | `liv_initvel3` |
| P34-35_P74-75.agc:1220 | 11,2130 | `+2` | 22132 | `liv_initvel3` |
| P34-35_P74-75.agc:1221 | 11,2131 | `VCOMP PUSH` | 41476 | `liv_initvel3` |
| P34-35_P74-75.agc:1222 | 11,2132 | `VLOAD` | 77775 | `liv_initvel3` |
| P34-35_P74-75.agc:1223 | 11,2133 | `VXV DOT` | 50235 | `liv_initvel3` |
| P34-35_P74-75.agc:1224 | 11,2134 | `BPL DLOAD` | 71244 | `liv_initvel3` |
| P34-35_P74-75.agc:1225 | 11,2135 | `INITVEL4` | 22137 | `liv_initvel3` |
| P34-35_P74-75.agc:1226 | 11,2136 | `DCOMP PUSH` | 41476 | `liv_initvel3` |
| P34-35_P74-75.agc:1227 | 11,2137 | `INITVEL4 LXA,2 SXA,2` | 67154 | `liv_initvel3` |
| P34-35_P74-75.agc:1228 | 11,2140 | `0D` | 00000 | `liv_initvel3` |
| P34-35_P74-75.agc:1229 | 11,2141 | `GEOMSGN` | 02672 | `liv_initvel3` |
| P34-35_P74-75.agc:1233 | 11,2142 | `LXA,1 CALL` | 45150 | `liv_initvel3` |
| P34-35_P74-75.agc:1234 | 11,2143 | `RTX1` | 02776 | `liv_initvel3` |
| P34-35_P74-75.agc:1238 | 11,2144 | `SETITCTR` | 23765 | `liv_initvel3` |
| P34-35_P74-75.agc:1243 | 11,2145 | `CLEAR VLOAD` | 77214 | `liv_afterlambert` |
| P34-35_P74-75.agc:1244 | 11,2146 | `GUESSW` | 00675 | `liv_afterlambert` |
| P34-35_P74-75.agc:1245 | 11,2147 | `VVEC` | 02744 | `liv_afterlambert` |
| P34-35_P74-75.agc:1249 | 11,2150 | `STODL VIPRIME` | 16337 | `liv_afterlambert` |
| P34-35_P74-75.agc:1254 | 11,2151 | `VTARGTAG` | 02702 | `liv_afterlambert` |
| P34-35_P74-75.agc:1255 | 11,2152 | `BHIZ CALL` | 45030 | `liv_afterlambert` |
| P34-35_P74-75.agc:1256 | 11,2153 | `INITVEL7` | 22230 | `liv_afterlambert` |
| P34-35_P74-75.agc:1257 | 11,2154 | `INTSTALL` | 27414 | `liv_afterlambert` |
| P34-35_P74-75.agc:1258 | 11,2155 | `SLOAD CLEAR` | 43135 | `liv_afterintstall` |
| P34-35_P74-75.agc:1259 | 11,2156 | `RTX2` | 03000 | `liv_afterintstall` |
| P34-35_P74-75.agc:1260 | 11,2157 | `MOONFLAG` | 00263 | `liv_afterintstall` |
| P34-35_P74-75.agc:1261 | 11,2160 | `BHIZ SET` | 43030 | `liv_afterintstall` |
| P34-35_P74-75.agc:1262 | 11,2161 | `INITVEL5` | 22163 | `liv_afterintstall` |
| P34-35_P74-75.agc:1264 | 11,2162 | `MOONFLAG` | 00063 | `liv_afterintstall` |
| P34-35_P74-75.agc:1265 | 11,2163 | `INITVEL5 VLOAD` | 77775 | `liv_afterintstall` |
| P34-35_P74-75.agc:1266 | 11,2164 | `RINIT` | 02323 | `liv_afterintstall` |
| P34-35_P74-75.agc:1267 | 11,2165 | `STORE R1VEC` | 02655 | `liv_afterintstall` |
| P34-35_P74-75.agc:1268 | 11,2166 | `STOVL RCV` | 25535 | `liv_afterintstall` |
| P34-35_P74-75.agc:1269 | 11,2167 | `VIPRIME` | 02337 | `liv_afterintstall` |
| P34-35_P74-75.agc:1270 | 11,2170 | `STODL VCV` | 15543 | `liv_afterintstall` |
| P34-35_P74-75.agc:1271 | 11,2171 | `INTIME` | 03610 | `liv_afterintstall` |
| P34-35_P74-75.agc:1272 | 11,2172 | `STORE TET` | 01517 | `liv_afterintstall` |
| P34-35_P74-75.agc:1273 | 11,2173 | `DAD CLEAR` | 43015 | `liv_afterintstall` |
| P34-35_P74-75.agc:1274 | 11,2174 | `DELLT4` | 03452 | `liv_afterintstall` |
| P34-35_P74-75.agc:1275 | 11,2175 | `INTYPFLG` | 01673 | `liv_afterintstall` |
| P34-35_P74-75.agc:1276 | 11,2176 | `STCALL TDEC1` | 34041 | `liv_afterintstall` |
| P34-35_P74-75.agc:1277 | 11,2177 | `INTEGRVS` | 27107 | `liv_afterintstall` |
| P34-35_P74-75.agc:1278 | 11,2200 | `VLOAD` | 77775 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1279 | 11,2201 | `VATT1` | 00025 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1280 | 11,2202 | `STORE VTARGET` | 02703 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1285 | 11,2203 | `LXA,2 INCR,2` | 63154 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1286 | 11,2204 | `ITCTR` | 03616 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1287 | 11,2205 | `1D` | 00001 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1288 | 11,2206 | `SXA,2 XSU,2` | 55134 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1289 | 11,2207 | `ITCTR` | 03616 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1290 | 11,2210 | `VTARGTAG` | 02701 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1291 | 11,2211 | `SLOAD BHIZ` | 46135 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1292 | 11,2212 | `X2` | 00050 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1293 | 11,2213 | `INITVEL6` | 22225 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1297 | 11,2214 | `VLOAD VSU` | 52375 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1298 | 11,2215 | `RTARG1` | 03472 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1299 | 11,2216 | `RATT1` | 00017 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1300 | 11,2217 | `VAD` | 77655 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1301 | 11,2220 | `R2VEC` | 02663 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1302 | 11,2221 | `STODL R2VEC` | 16663 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1303 | 11,2222 | `COZY4` | 03667 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1304 | 11,2223 | `GOTO` | 77650 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1305 | 11,2224 | `INITVEL2` | 22062 | `liv_afterintegrvs` |
| P34-35_P74-75.agc:1309 | 11,2225 | `INITVEL6 VLOAD` | 77775 | `liv_initvel6` |
| P34-35_P74-75.agc:1310 | 11,2226 | `R2VEC` | 02663 | `liv_initvel6` |
| P34-35_P74-75.agc:1311 | 11,2227 | `STORE RTARG1` | 03472 | `liv_initvel6` |
| P34-35_P74-75.agc:1312 | 11,2230 | `INITVEL7 VLOAD VSU` | 52375 | `liv_initvel7` |
| P34-35_P74-75.agc:1313 | 11,2231 | `VIPRIME` | 02337 | `liv_initvel7` |
| P34-35_P74-75.agc:1314 | 11,2232 | `VINIT` | 02331 | `liv_initvel7` |
| P34-35_P74-75.agc:1315 | 11,2233 | `STOVL DELVEET3` | 26366 | `liv_initvel7` |
| P34-35_P74-75.agc:1317 | 11,2234 | `VTARGET` | 02703 | `liv_initvel7` |
| P34-35_P74-75.agc:1318 | 11,2235 | `STORE VTPRIME` | 03566 | `liv_initvel7` |
| P34-35_P74-75.agc:1319 | 11,2236 | `SLOAD BHIZ` | 46135 | `liv_initvel7` |
| P34-35_P74-75.agc:1320 | 11,2237 | `RTX2` | 03000 | `liv_initvel7` |
| P34-35_P74-75.agc:1321 | 11,2240 | `INITVELX` | 22255 | `liv_initvel7` |
| P34-35_P74-75.agc:1322 | 11,2241 | `VLOAD VSR2` | 70575 | `liv_initvel7` |
| P34-35_P74-75.agc:1323 | 11,2242 | `VTPRIME` | 03566 | `liv_initvel7` |
| P34-35_P74-75.agc:1324 | 11,2243 | `STOVL VTPRIME` | 27566 | `liv_initvel7` |
| P34-35_P74-75.agc:1325 | 11,2244 | `VIPRIME` | 02337 | `liv_initvel7` |
| P34-35_P74-75.agc:1326 | 11,2245 | `VSR2` | 77742 | `liv_initvel7` |
| P34-35_P74-75.agc:1327 | 11,2246 | `STOVL VIPRIME` | 26337 | `liv_initvel7` |
| P34-35_P74-75.agc:1328 | 11,2247 | `RTARG1` | 03472 | `liv_initvel7` |
| P34-35_P74-75.agc:1329 | 11,2250 | `VSR2` | 77742 | `liv_initvel7` |
| P34-35_P74-75.agc:1330 | 11,2251 | `STOVL RTARG1` | 27472 | `liv_initvel7` |
| P34-35_P74-75.agc:1331 | 11,2252 | `DELVEET3` | 02366 | `liv_initvel7` |
| P34-35_P74-75.agc:1332 | 11,2253 | `VSR2` | 77742 | `liv_initvel7` |
| P34-35_P74-75.agc:1333 | 11,2254 | `STORE DELVEET3` | 02366 | `liv_initvel7` |
| P34-35_P74-75.agc:1334 | 11,2255 | `INITVELX LXA,1 DLOAD*` | 70750 | `liv_initvel7` |
| P34-35_P74-75.agc:1335 | 11,2256 | `RTX1` | 02776 | `liv_initvel7` |
| P34-35_P74-75.agc:1336 | 11,2257 | `MUTABLE -2,1` | 10003 | `liv_initvel7` |
| P34-35_P74-75.agc:1337 | 11,2260 | `PUSH DMP` | 41206 | `liv_initvel7` |
| P34-35_P74-75.agc:1338 | 11,2261 | `R1A` | 02742 | `liv_initvel7` |
| P34-35_P74-75.agc:1339 | 11,2262 | `SR1 DDV` | 56342 | `liv_initvel7` |
| P34-35_P74-75.agc:1340 | 11,2263 | `R1` | 00041 | `liv_initvel7` |
| P34-35_P74-75.agc:1341 | 11,2264 | `STODL MU/A` | 17721 | `liv_initvel7` |
| P34-35_P74-75.agc:1342 | 11,2265 | `SR` | 77661 | `liv_initvel7` |
| P34-35_P74-75.agc:1343 | 11,2266 | `6` | 20607 | `liv_initvel7` |
| P34-35_P74-75.agc:1344 | 11,2267 | `STORE MUASTEER` | 03717 | `liv_initvel7` |
| P34-35_P74-75.agc:1345 | 11,2270 | `SETPD VLOAD` | 77201 | `liv_initvel7` |
| P34-35_P74-75.agc:1346 | 11,2271 | `0D` | 00001 | `liv_initvel7` |
| P34-35_P74-75.agc:1347 | 11,2272 | `RTARG1` | 03472 | `liv_initvel7` |
| P34-35_P74-75.agc:1348 | 11,2273 | `STCALL RTARG` | 37444 | `liv_initvel7` |
| P34-35_P74-75.agc:1349 | 11,2274 | `NORMEX` | 03463 | `liv_initvel7` |
| ORBITAL_INTEGRATION.agc:862 | 11,3765 | `SETITCTR SSP BOFF` | 43131 | `liv_initvel3` |
| ORBITAL_INTEGRATION.agc:863 | 11,3766 | `ITERCTR` | 00027 | `liv_initvel3` |
| ORBITAL_INTEGRATION.agc:864 | 11,3767 | `20D` | 00024 | `liv_initvel3` |
| ORBITAL_INTEGRATION.agc:865 | 11,3770 | `AVEGFLAG` | 03752 | `liv_initvel3` |
| ORBITAL_INTEGRATION.agc:866 | 11,3771 | `LAMBERT` | 25212 | `liv_initvel3` |
| ORBITAL_INTEGRATION.agc:867 | 11,3772 | `SSP GOTO` | 52131 | `liv_initvel3` |
| ORBITAL_INTEGRATION.agc:868 | 11,3773 | `ITERCTR` | 00027 | `liv_initvel3` |
| ORBITAL_INTEGRATION.agc:869 | 11,3774 | `5` | 00005 | `liv_initvel3` |
| ORBITAL_INTEGRATION.agc:870 | 11,3775 | `LAMBERT` | 25212 | `liv_initvel3` |

## Evidence

Hashes are in `evidence/lm-initvel/SHA256.json`. Bulk files are assets of
the `evidence-lm-initvel` release (`python3 tools/evidence.py fetch
lm-initvel`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_initvel_source.py` reassembles the unchanged files with
  the pinned yaYUL. The rope equals the certified fixed-data rope. All 198
  words map to native literals in 10 functions.
- **Oracle.** `tools/lm_initvel_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant over 65536
  sampled entries. It stops at the caller's NEWOPS frontier named by
  NORMEX, the abort frontier, or INTSTALL/INTEGRVS.
  `tools/lm_initvel_differential.py` checks the native producer against
  every entry and pins the stream identity (`lm-initvel-source-stream.json`,
  SHA256 `0ecbabe1...11c02e`, 269,090,816 bytes).
  - INITVEL returns on 39567 entries.
  - 2334 entries reach LAMBERT's SQRT/UNIT abort.
  - 23635 entries reach INTSTALL or INTEGRVS.
  - Every phase transition and both sides of each runtime branch are taken:
    the Moon BHIZ, both BPLs of INITVEL2, BMN, BHIZ and BPL of INITVEL3,
    SETITCTR's AVEGFLAG, the VTARGTAG BHIZ, MOONFLAG, the INITVEL6 exit and
    the INITVELX BHIZ.
- **Host and sanitizer results.** Run
  `make test-lm-initvel sanitize-lm-initvel`. The strict build and the
  ASAN/UBSAN build each pass 22 frozen entries, 28 admission guards, 49
  frozen phase steps and the full 65536-entry stream
  (`lm-initvel-checks.log`).
- **Phase frontiers.** `tools/freeze_lm_initvel_phases.py` requires each of
  the 7301 native frontiers of the 22 frozen entries (LAMBERT's included) to
  equal, in order, an original NEWOPS image of the oracle. It then freezes
  49 steps in four groups.
- **C64 execution results.** Run `make vice-lm-initvel`. Four PRGs each
  link one phase group: INITVEL through INITVEL1; LAMBERT's entry and its
  INITV/TARGETV returns; INITVEL2 and INITVEL3; and the rest. Each keeps
  both images in the RAM under KERNAL ROM (`$01=$35`). In VICE (PAL, stock
  ROMs) they pass 11, 12, 8 and 18 steps with no failures
  (`lm-initvel-results.json`).

## Limits

- The C64 check runs each phase separately. The whole chains run on the
  host.
- INTSTALL and INTEGRVS are not translated, so the precision-trajectory
  iteration stops at them. The rest of P34-35_P74-75 is not covered by this
  milestone. In ORBITAL_INTEGRATION only SETITCTR's words are compiled, and
  that file's coverage entry is unchanged.
- Numerical validity is the original's. No wider claim is made.
