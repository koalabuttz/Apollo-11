# P32/P72: CSI targeting, with ALMXITA and ALMXIT

Lane E. Milestone `lm-p32`.

## Scope

From `Luminary099/P32-P35_P72-P75.agc`, bank 35 `02027`-`02215`: 119 words
on 119 source lines. Twenty-one basic routines are keyed by Z. Nineteen
interpretive routines are keyed by LOC under BANKSET at NEWOPS. All use the
control-transfer ABI (`include/agc_transfer.h`,
`docs/NATIVE_TRANSFER_ABI.md`). `include/lm_p32_routines.h` lists each
entry.

- **Entry (basic):** P32 and P72 go through AVFLAGA or AVFLAGP. P32STRT
  zeroes CENTANG from P30ZERO. P32/P72A calls P20FLGON, zeroes NN +1 and
  TCSI, shows V06N11 (TCSI) through VNPOOH and enters INTPRET.
- **TCSI (interpretive):** a positive TCSI goes to VN0655. Otherwise PRECSET
  from TETLEM, then RVEC and VVEC from RACT3 and VACT3 (VSR* by X2), RVSW
  set, and TIMERAD to DPPOSMAX. TCSI becomes TDEC2 plus that time, and the
  basic code shows V06N11 again.
- **VN0655:** V06N55 through GOFLASH. Terminate goes to GOTOPOOH, recycle
  repeats the display, and proceed shows V06N37 (TTPI) and re-enters
  INTPRET.
- **Setup and loop (interpretive):** TCSI into TIG, then SELECTMU.
  P32/P72B calls ADVANCE, pushes VPASS1, RPASS1, TCSI, TTPI and TWOPI and
  calls INTINT, then PASSIVE and CSI/A.
- **Times:** P32/P72C sets UPDATFLG unless FINALFLG is on. P32/P72D to
  P32/P72F take 60MIN off T1TOT2 and T2TOT3 while each stays positive, then
  exit to V06N75 through VNPOOH.
- **Display:** DELVEET1 through S32/33.1, DELVEET1 and RACT1 stored, VN
  0682 through DISDVLVC, TTPI into TTPI0 through VN1645, and GOTO
  P32/P72B.
- **ALMXITA and ALMXIT (interpretive):** ALMXITA stores X2 in CSIALRM.
  ALMXIT loads the ALARM/TB word that CSIALRM names (SLOAD* by the
  complement in X1) and exits. The basic code passes it to VARALARM, then
  shows V05N09 through GOFLASH. Terminate goes to GOTOPOOH and recycle
  repeats the display.

Each run ends at the first point outside these routines
(`tests/lm_p32_cases.c`, `lm_p32_stop`). That is one of:

- AVFLAGA, AVFLAGP, P20FLGON (lm-p33), VNPOOH (lm-p34), VARALARM,
  BANKCALL, GOTOPOOH or INTPRET;
- POODOO1 (alarm 1302);
- a NEWOPS outside `35,2036`-`2043`, `2063`-`2111`, `2113`, `2125`-`2172`
  and `2176`-`2215`: PRECSET, TIMERAD, SELECTMU, ADVANCE, INTINT, PASSIVE,
  CSI/A, S32/33.1, DISDVLVC or VN1645.

Every interpretive pair is compiled with the shared bodies
(`src/lm_interp_steps.c`, `src/lm_interp_orders.c`). The loops of
P32/P72E and P32/P72F run in their routines, each taken BPL ending at its
label's NEWOPS. SLOAD* forms its address with the shared indexed body and
then reads the ALARM/TB literal at that address. V06N11, V05N09, V06N55,
V06N37, V06N75, the P30ZERO, 60MIN, TWOPI and DPPOSMAX pairs and the seven
ALARM/TB words are literals that the source map checks against the fresh
rope. Each basic instruction is one `ai_*` register effect on the image, in
source order.

Interpretive routines admit a canonical NEWOPS image at their LOC in FB 35
meeting all of these conditions, and otherwise return `EXEC_OUT_OF_SCOPE`
with the image unchanged:

- EB and BB coherent, with BANKSET = BB;
- INTBIT15 set;
- EDOP, ZERO and NEWJOB zero;
- MODE `+0`, `+1` or vector;
- the VAC one of the five, with PUSHLOC no further than 32D into it;
- IRQ 0 or 1, and an allowed SUPERBNK;
- every word 15 bits;
- for ALMXIT, CSIALRM from 1 to 7 (an ALARM/TB entry).

Basic routines admit Z at their entry in FB 35, with EB and BB coherent,
ZERO zero, IRQ 0 or 1, an allowed SUPERBNK and every word 15 bits.

## Source map

`tools/lm_p32_source.py` checks this map
(`evidence/lm-p32/lm-p32-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, or a word of the basic routine whose ENTRY names that address. The
erasable words and destinations the code names, and the twenty fixed
constant words, must equal their native values in the fresh assembly.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P32-P35_P72-P75.agc:203 | 35,2027 | `P32 TC AVFLAGA` | 02347 | `p32` |
| P32-P35_P72-P75.agc:204 | 35,2030 | `TC P32STRT` | 02032 | `avflaga_ret` |
| P32-P35_P72-P75.agc:205 | 35,2031 | `P72 TC AVFLAGP` | 02354 | `p72` |
| P32-P35_P72-P75.agc:206 | 35,2032 | `P32STRT EXTEND` | 00006 | `p32strt` |
| P32-P35_P72-P75.agc:207 | 35,2033 | `DCA P30ZERO` | 32420 | `p32strt` |
| P32-P35_P72-P75.agc:208 | 35,2034 | `DXCH CENTANG` | 53621 | `p32strt` |
| P32-P35_P72-P75.agc:209 | 35,2035 | `TC P32/P72A` | 02053 | `p32strt` |
| P32-P35_P72-P75.agc:210 | 35,2036 | `ALMXITA SXA,2` | 77734 | `almxita` |
| P32-P35_P72-P75.agc:211 | 35,2037 | `CSIALRM` | 03613 | `almxita` |
| P32-P35_P72-P75.agc:212 | 35,2040 | `ALMXIT LXC,1` | 77740 | `almxit` |
| P32-P35_P72-P75.agc:213 | 35,2041 | `CSIALRM` | 03613 | `almxit` |
| P32-P35_P72-P75.agc:214 | 35,2042 | `SLOAD* EXIT` | 77533 | `almxit` |
| P32-P35_P72-P75.agc:215 | 35,2043 | `ALARM/TB -1,1` | 32423 | `almxit` |
| P32-P35_P72-P75.agc:216 | 35,2044 | `CA MPAC` | 30154 | `almxit_exit` |
| P32-P35_P72-P75.agc:217 | 35,2045 | `TC VARALARM` | 05744 | `almxit_exit` |
| P32-P35_P72-P75.agc:218 | 35,2046 | `CAF V05N09` | 35006 | `varalarm_ret` |
| P32-P35_P72-P75.agc:219 | 35,2047 | `TC BANKCALL` | 04616 | `varalarm_ret` |
| P32-P35_P72-P75.agc:220 | 35,2050 | `CADR GOFLASH` | 20476 | `varalarm_ret` |
| P32-P35_P72-P75.agc:221 | 35,2051 | `TC GOTOPOOH` | 06001 | `flash_terminate` |
| P32-P35_P72-P75.agc:222 | 35,2052 | `TC -4` | 02046 | `flash_recycle` |
| P32-P35_P72-P75.agc:223 | 35,2053 | `P32/P72A TC P20FLGON` | 02361 | `p32a` |
| P32-P35_P72-P75.agc:224 | 35,2054 | `CAF P30ZERO` | 32417 | `p20_ret` |
| P32-P35_P72-P75.agc:225 | 35,2055 | `TS NN +1` | 55467 | `p20_ret` |
| P32-P35_P72-P75.agc:226 | 35,2056 | `TS TCSI` | 55633 | `p20_ret` |
| P32-P35_P72-P75.agc:227 | 35,2057 | `TS TCSI +1` | 55634 | `p20_ret` |
| P32-P35_P72-P75.agc:228 | 35,2060 | `VN0611 CAF V06N11` | 32410 | `vn0611` |
| P32-P35_P72-P75.agc:229 | 35,2061 | `TC VNPOOH` | 03712 | `vn0611` |
| P32-P35_P72-P75.agc:230 | 35,2062 | `TC INTPRET` | 06037 | `vnpooh_ret` |
| P32-P35_P72-P75.agc:231 | 35,2063 | `DLOAD DCOMP` | 57545 | `tcsi_check` |
| P32-P35_P72-P75.agc:232 | 35,2064 | `TCSI` | 03634 | `tcsi_check` |
| P32-P35_P72-P75.agc:233 | 35,2065 | `BMN DLOAD` | 71240 | `tcsi_check` |
| P32-P35_P72-P75.agc:234 | 35,2066 | `VN0655` | 72113 | `tcsi_check` |
| P32-P35_P72-P75.agc:236 | 35,2067 | `TETLEM` | 01643 | `tcsi_check` |
| P32-P35_P72-P75.agc:237 | 35,2070 | `STCALL TDEC1` | 34041 | `tcsi_check` |
| P32-P35_P72-P75.agc:238 | 35,2071 | `PRECSET` | 46360 | `tcsi_check` |
| P32-P35_P72-P75.agc:239 | 35,2072 | `VLOAD VSR*` | 53775 | `precset_ret` |
| P32-P35_P72-P75.agc:240 | 35,2073 | `RACT3` | 03536 | `precset_ret` |
| P32-P35_P72-P75.agc:241 | 35,2074 | `0,2` | 57176 | `precset_ret` |
| P32-P35_P72-P75.agc:242 | 35,2075 | `STOVL RVEC` | 26655 | `precset_ret` |
| P32-P35_P72-P75.agc:243 | 35,2076 | `VACT3` | 03544 | `precset_ret` |
| P32-P35_P72-P75.agc:244 | 35,2077 | `VSR* SET` | 43057 | `precset_ret` |
| P32-P35_P72-P75.agc:245 | 35,2100 | `0,2` | 57176 | `precset_ret` |
| P32-P35_P72-P75.agc:246 | 35,2101 | `RVSW` | 03466 | `precset_ret` |
| P32-P35_P72-P75.agc:247 | 35,2102 | `STODL VVEC` | 16744 | `precset_ret` |
| P32-P35_P72-P75.agc:248 | 35,2103 | `DPPOSMAX` | 06530 | `precset_ret` |
| P32-P35_P72-P75.agc:249 | 35,2104 | `STCALL RDESIRED` | 36756 | `precset_ret` |
| P32-P35_P72-P75.agc:250 | 35,2105 | `TIMERAD` | 25547 | `precset_ret` |
| P32-P35_P72-P75.agc:251 | 35,2106 | `DAD` | 77615 | `timerad_ret` |
| P32-P35_P72-P75.agc:252 | 35,2107 | `TDEC2` | 03574 | `timerad_ret` |
| P32-P35_P72-P75.agc:253 | 35,2110 | `STORE TCSI` | 03634 | `timerad_ret` |
| P32-P35_P72-P75.agc:254 | 35,2111 | `EXIT` | 77776 | `timerad_ret` |
| P32-P35_P72-P75.agc:255 | 35,2112 | `TC VN0611` | 02060 | `tcsi_exit` |
| P32-P35_P72-P75.agc:256 | 35,2113 | `VN0655 EXIT` | 77776 | `vn0655` |
| P32-P35_P72-P75.agc:257 | 35,2114 | `CAF V06N55` | 33724 | `vn0655_exit` |
| P32-P35_P72-P75.agc:258 | 35,2115 | `TC BANKCALL` | 04616 | `vn0655_exit` |
| P32-P35_P72-P75.agc:259 | 35,2116 | `CADR GOFLASH` | 20476 | `vn0655_exit` |
| P32-P35_P72-P75.agc:260 | 35,2117 | `TC GOTOPOOH` | 06001 | `flash_terminate2` |
| P32-P35_P72-P75.agc:261 | 35,2120 | `TC +2` | 02122 | `flash_proceed2` |
| P32-P35_P72-P75.agc:262 | 35,2121 | `TC -5` | 02114 | `flash_recycle2` |
| P32-P35_P72-P75.agc:263 | 35,2122 | `CAF V06N37` | 33723 | `v06n37` |
| P32-P35_P72-P75.agc:264 | 35,2123 | `TC VNPOOH` | 03712 | `v06n37` |
| P32-P35_P72-P75.agc:265 | 35,2124 | `TC INTPRET` | 06037 | `vnpooh_ret2` |
| P32-P35_P72-P75.agc:266 | 35,2125 | `DLOAD` | 77745 | `tcsi_tig` |
| P32-P35_P72-P75.agc:267 | 35,2126 | `TCSI` | 03634 | `tcsi_tig` |
| P32-P35_P72-P75.agc:268 | 35,2127 | `STCALL TIG` | 37442 | `tcsi_tig` |
| P32-P35_P72-P75.agc:269 | 35,2130 | `SELECTMU` | 20000 | `tcsi_tig` |
| P32-P35_P72-P75.agc:270 | 35,2131 | `P32/P72B CALL` | 77624 | `p32b` |
| P32-P35_P72-P75.agc:271 | 35,2132 | `ADVANCE` | 71016 | `p32b` |
| P32-P35_P72-P75.agc:272 | 35,2133 | `SETPD VLOAD` | 77201 | `advance_ret` |
| P32-P35_P72-P75.agc:273 | 35,2134 | `0D` | 00001 | `advance_ret` |
| P32-P35_P72-P75.agc:274 | 35,2135 | `VPASS1` | 03506 | `advance_ret` |
| P32-P35_P72-P75.agc:275 | 35,2136 | `PDVL PDDL` | 65315 | `advance_ret` |
| P32-P35_P72-P75.agc:276 | 35,2137 | `RPASS1` | 03500 | `advance_ret` |
| P32-P35_P72-P75.agc:277 | 35,2140 | `TCSI` | 03634 | `advance_ret` |
| P32-P35_P72-P75.agc:278 | 35,2141 | `PDDL PDDL` | 65325 | `advance_ret` |
| P32-P35_P72-P75.agc:279 | 35,2142 | `TTPI` | 03636 | `advance_ret` |
| P32-P35_P72-P75.agc:280 | 35,2143 | `TWOPI` | 33732 | `advance_ret` |
| P32-P35_P72-P75.agc:281 | 35,2144 | `PUSH CALL` | 45006 | `advance_ret` |
| P32-P35_P72-P75.agc:282 | 35,2145 | `INTINT` | 73466 | `advance_ret` |
| P32-P35_P72-P75.agc:283 | 35,2146 | `CALL` | 77624 | `intint_ret` |
| P32-P35_P72-P75.agc:284 | 35,2147 | `PASSIVE` | 46412 | `intint_ret` |
| P32-P35_P72-P75.agc:285 | 35,2150 | `CALL` | 77624 | `passive_ret` |
| P32-P35_P72-P75.agc:287 | 35,2151 | `CSI/A` | 70113 | `passive_ret` |
| P32-P35_P72-P75.agc:288 | 35,2152 | `P32/P72C BON SET` | 43014 | `p32c` |
| P32-P35_P72-P75.agc:289 | 35,2153 | `FINALFLG` | 01311 | `p32c` |
| P32-P35_P72-P75.agc:290 | 35,2154 | `P32/P72D` | 72156 | `p32c` |
| P32-P35_P72-P75.agc:291 | 35,2155 | `UPDATFLG` | 00470 | `p32c` |
| P32-P35_P72-P75.agc:292 | 35,2156 | `P32/P72D DLOAD` | 77745 | `p32d` |
| P32-P35_P72-P75.agc:293 | 35,2157 | `T1TOT2` | 02253 | `p32d` |
| P32-P35_P72-P75.agc:294 | 35,2160 | `P32/P72E STORE T1TOT2` | 02253 | `p32e` |
| P32-P35_P72-P75.agc:295 | 35,2161 | `DSU BPL` | 51025 | `p32e` |
| P32-P35_P72-P75.agc:296 | 35,2162 | `60MIN` | 32422 | `p32e` |
| P32-P35_P72-P75.agc:297 | 35,2163 | `P32/P72E` | 72160 | `p32e` |
| P32-P35_P72-P75.agc:298 | 35,2164 | `DLOAD` | 77745 | `p32e` |
| P32-P35_P72-P75.agc:299 | 35,2165 | `T2TOT3` | 02255 | `p32e` |
| P32-P35_P72-P75.agc:300 | 35,2166 | `P32/P72F STORE T2TOT3` | 02255 | `p32f` |
| P32-P35_P72-P75.agc:301 | 35,2167 | `DSU BPL` | 51025 | `p32f` |
| P32-P35_P72-P75.agc:302 | 35,2170 | `60MIN` | 32422 | `p32f` |
| P32-P35_P72-P75.agc:303 | 35,2171 | `P32/P72F` | 72166 | `p32f` |
| P32-P35_P72-P75.agc:304 | 35,2172 | `EXIT` | 77776 | `p32f` |
| P32-P35_P72-P75.agc:305 | 35,2173 | `CAF V06N75` | 32412 | `v06n75` |
| P32-P35_P72-P75.agc:306 | 35,2174 | `TC VNPOOH` | 03712 | `v06n75` |
| P32-P35_P72-P75.agc:307 | 35,2175 | `TC INTPRET` | 06037 | `vnpooh_ret3` |
| P32-P35_P72-P75.agc:308 | 35,2176 | `VLOAD CALL` | 45175 | `vnpooh_ret_i` |
| P32-P35_P72-P75.agc:309 | 35,2177 | `DELVEET1` | 02267 | `vnpooh_ret_i` |
| P32-P35_P72-P75.agc:310 | 35,2200 | `S32/33.1` | 71100 | `vnpooh_ret_i` |
| P32-P35_P72-P75.agc:311 | 35,2201 | `STOVL DELVEET1` | 26267 | `s3233_ret` |
| P32-P35_P72-P75.agc:312 | 35,2202 | `RACT2` | 02311 | `s3233_ret` |
| P32-P35_P72-P75.agc:313 | 35,2203 | `STOVL RACT1` | 26303 | `s3233_ret` |
| P32-P35_P72-P75.agc:314 | 35,2204 | `DELVEET2` | 02275 | `s3233_ret` |
| P32-P35_P72-P75.agc:315 | 35,2205 | `AXT,1 CALL` | 45170 | `s3233_ret` |
| P32-P35_P72-P75.agc:316 | 35,2206 | `VN 0682` | 01522 | `s3233_ret` |
| P32-P35_P72-P75.agc:317 | 35,2207 | `DISDVLVC` | 72370 | `s3233_ret` |
| P32-P35_P72-P75.agc:318 | 35,2210 | `DLOAD` | 77745 | `disdvlvc_ret` |
| P32-P35_P72-P75.agc:319 | 35,2211 | `TTPI` | 03636 | `disdvlvc_ret` |
| P32-P35_P72-P75.agc:320 | 35,2212 | `STCALL TTPIO` | 37640 | `disdvlvc_ret` |
| P32-P35_P72-P75.agc:321 | 35,2213 | `VN1645` | 73606 | `disdvlvc_ret` |
| P32-P35_P72-P75.agc:322 | 35,2214 | `GOTO` | 77650 | `loop` |
| P32-P35_P72-P75.agc:323 | 35,2215 | `P32/P72B` | 72131 | `loop` |

## Evidence

Hashes are in `evidence/lm-p32/SHA256.json`. Bulk files are assets of the
`evidence-lm-p32` release (`python3 tools/evidence.py fetch lm-p32`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_p32_source.py` reassembles the unchanged file with the
  pinned yaYUL. The rope equals the certified fixed-data rope.
- **Oracle.** `tools/lm_p32_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant from 20480
  entries, 512 per routine, to the same stops.
  - The entries sit over poisoned debris with any times (both signs of
    TCSI), vectors, index registers and flags (FINALFLG, RVSW).
  - CSIALRM (for ALMXIT) and X2 (for ALMXITA) name an ALARM/TB entry.
  - EB is E7 three times in four.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/lm_p32_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`lm-p32-source-stream.json`, SHA256
  `f6b5b84f...2d3b18a5`) and archives the stream
  (`lm-p32-reference.snapshots.bin.xz`). `lm-p32-proof.json` lists the stops
  per routine.
- **Host and sanitizer results.** Run `make test-lm-p32 sanitize-lm-p32`.
  The strict build and the ASAN/UBSAN build each pass 20978 checks:
  - all 20480 frozen digests;
  - every run ending at one of the stops;
  - 415 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones, and CSIALRM 0 and 8 for ALMXIT.

  See `lm-p32-checks.log`.
- **C64 execution results.** Run `make vice-lm-p32`. One PRG registers
  every routine, built with `-fno-inline-functions`, and keeps the image in
  the RAM under KERNAL ROM (`$01=$35`). The case generator is integer-only,
  so the C64 builds the same entry images as the host. The 60MIN loops run
  from debris times, so `make vice-lm-p32` gives it a 9e9-cycle limit. In
  VICE (PAL, stock ROMs) the PRG passes 520 checks with no failures
  (`lm-p32-results.json`).

## Limits

- P33/P73 and the routines after it are lm-p33. The bank 34 routines (CSI,
  ADVANCE, S32/33.1, S32/33.X, CDHMVR) and the rest of the file are not
  covered. The file stays `partial`.
- PRECSET, TIMERAD, SELECTMU, ADVANCE, INTINT, PASSIVE, CSI/A, S32/33.1,
  DISDVLVC, VN1645, VNPOOH, AVFLAGA, AVFLAGP, P20FLGON, VARALARM and
  GOFLASH belong to other files or milestones and are stops here.
- Numerical validity is the original's. No wider claim is made.
