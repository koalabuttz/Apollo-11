# CM P76: target delta V

Lane E. Milestone `cm-p76`.

## Scope

All of `Comanche055/P76.agc`: bank 13 `02071`-`02230`, 96 words on 96
source lines. Fifteen basic routines are keyed by Z. Twelve interpretive
routines are keyed by LOC under BANKSET at NEWOPS. All use the
control-transfer ABI (`include/agc_transfer.h`, `docs/NATIVE_TRANSFER_ABI.md`).
`include/cm_p76_routines.h` lists each entry. This is the first Lane E file
built on the Comanche055 interpretive bodies (`include/cm_interp.h`).

- **Entry and displays (basic):** P76 sets TRACKFLG through UPFLAG. V06N84
  (DELVOV) and V06N33 (TIG) go through GOFLASH. Terminate goes to ENDP76,
  proceed to the next step, and recycle back to the previous display. The
  second proceed enters INTPRET.
- **Other vehicle (interpretive):** TIG goes to TDEC1 and OTHPREC is
  called. COMPMAT builds the local frame from RATT and VATT (UNIT, VXV),
  rotates DELVOV through it (VXM, VSL1), adds VATT and calls INTSTALL.
  The returns call P76SUB1, set up VCV, RCV and TET from the vehicle state
  scaled by X2 (VSR* 0,2), clear INTYPFLG and integrate to TETTHIS
  (INTEGRVS). After INTSTALL and P76SUB1 again, RRECT, RCV and TET come
  from RATT1, TAT and VATT1, and MINIRECT is called. Then EXIT.
- **Restart (basic):** PHASCHNG with OCT 04024, REINTFLG set through
  UPFLAG, INTPRET.
- **Finish (interpretive):** CALL ATOPOTH, then SSP QPRET OUT and EXIT into
  BANKCALL INTWAKE1. OUT is EXIT into ENDP76.
- **ENDP76 (basic):** MARKCTR and VHFCNT zeroed, MRKBUF2 set to NEGONE,
  then GOTOPOOH.
- **P76SUB1 (interpretive):** MOONFLAG cleared and X2 loaded; unless X2 is
  zero (BHIZ), MOONFLAG is set; RVQ.

Each run ends at the first point outside these routines
(`tests/cm_p76_cases.c`, `cm_p76_stop`). That is one of:

- UPFLAG, PHASCHNG, BANKCALL, GOTOPOOH or INTPRET;
- POODOO (alarm 1302);
- a NEWOPS outside bank 13's `2110`-`2174`, `2202`-`2206`, `2211` and
  `2222`-`2230`: OTHPREC, INTSTALL, INTEGRVS, MINIRECT, ATOPOTH, or
  P76SUB1's return to a C(QPRET) in another bank.

Every interpretive pair is compiled with the shared bodies built for
Comanche055 (`src/cm_interp_steps.c`, `src/cm_interp_orders.c`). COMPMAT's
VXM reads its matrix from the work area at 12D. V06N84, V06N84 +1, ZERO and
NEGONE are literals that the source map checks against the fresh rope.
Each basic instruction is one `ai_*` register effect on the image, in
source order.

Interpretive routines admit a canonical NEWOPS image (Comanche055 NEWOPS,
`06037`) at their LOC in FB 13 meeting all of these conditions, and
otherwise return `EXEC_OUT_OF_SCOPE` with the image unchanged:

- EB and BB coherent, with BANKSET = BB;
- INTBIT15 clear (bank 13 is below bank 20);
- EDOP, ZERO and NEWJOB zero;
- MODE `+0`, `+1` or vector;
- the VAC one of the five, with PUSHLOC no further than 32D into it;
- IRQ 0 or 1, and an allowed SUPERBNK;
- every word 15 bits.

Basic routines admit Z at their entry in FB 13, with EB and BB coherent,
ZERO zero, IRQ 0 or 1, an allowed SUPERBNK and every word 15 bits.

## Source map

`tools/cm_p76_source.py` checks this map
(`evidence/cm-p76/cm-p76-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, or a word of the basic routine whose ENTRY names that address. The
erasable words and destinations the basic code names, and the four fixed
constants, must equal their native values in the fresh assembly.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P76.agc:59 | 13,2071 | `P76 TC UPFLAG` | 05546 | `p76` |
| P76.agc:60 | 13,2072 | `ADRES TRACKFLG` | 00031 | `p76` |
| P76.agc:62 | 13,2073 | `CAF V06N84` | 32220 | `r_flash84` |
| P76.agc:63 | 13,2074 | `TC BANKCALL` | 04662 | `r_flash84` |
| P76.agc:64 | 13,2075 | `CADR GOFLASH` | 20720 | `r_flash84` |
| P76.agc:65 | 13,2076 | `TCF ENDP76` | 12212 | `f84_terminate` |
| P76.agc:66 | 13,2077 | `TC +2` | 02101 | `f84_proceed` |
| P76.agc:67 | 13,2100 | `TC -5` | 02073 | `f84_recycle` |
| P76.agc:68 | 13,2101 | `CAF V06N84 +1` | 32221 | `r_flash33` |
| P76.agc:69 | 13,2102 | `TC BANKCALL` | 04662 | `r_flash33` |
| P76.agc:70 | 13,2103 | `CADR GOFLASH` | 20720 | `r_flash33` |
| P76.agc:71 | 13,2104 | `TCF ENDP76` | 12212 | `f33_terminate` |
| P76.agc:72 | 13,2105 | `TC +2` | 02107 | `f33_proceed` |
| P76.agc:73 | 13,2106 | `TC -5` | 02101 | `f33_recycle` |
| P76.agc:74 | 13,2107 | `TC INTPRET` | 06006 | `r_intpret` |
| P76.agc:75 | 13,2110 | `DLOAD` | 77745 | `p76_tig` |
| P76.agc:76 | 13,2111 | `TIG` | 03413 | `p76_tig` |
| P76.agc:77 | 13,2112 | `STCALL TDEC1` | 34041 | `p76_tig` |
| P76.agc:78 | 13,2113 | `OTHPREC` | 27105 | `p76_tig` |
| P76.agc:79 | 13,2114 | `COMPMAT VLOAD UNIT` | 53575 | `compmat` |
| P76.agc:80 | 13,2115 | `RATT` | 00001 | `compmat` |
| P76.agc:82 | 13,2116 | `VCOMP` | 77676 | `compmat` |
| P76.agc:83 | 13,2117 | `STORE 24D` | 00031 | `compmat` |
| P76.agc:84 | 13,2120 | `VXV UNIT` | 53435 | `compmat` |
| P76.agc:85 | 13,2121 | `VATT` | 00007 | `compmat` |
| P76.agc:86 | 13,2122 | `STORE 18D` | 00023 | `compmat` |
| P76.agc:87 | 13,2123 | `VXV UNIT` | 53435 | `compmat` |
| P76.agc:88 | 13,2124 | `24D` | 00031 | `compmat` |
| P76.agc:89 | 13,2125 | `STOVL 12D` | 24015 | `compmat` |
| P76.agc:90 | 13,2126 | `DELVOV` | 03540 | `compmat` |
| P76.agc:91 | 13,2127 | `VXM VSL1` | 76505 | `compmat` |
| P76.agc:92 | 13,2130 | `12D` | 00015 | `compmat` |
| P76.agc:93 | 13,2131 | `VAD` | 77655 | `compmat` |
| P76.agc:94 | 13,2132 | `VATT` | 00007 | `compmat` |
| P76.agc:95 | 13,2133 | `STORE 6` | 00007 | `compmat` |
| P76.agc:96 | 13,2134 | `CALL` | 77624 | `compmat` |
| P76.agc:97 | 13,2135 | `INTSTALL` | 27442 | `compmat` |
| P76.agc:98 | 13,2136 | `CALL` | 77624 | `call_sub1` |
| P76.agc:99 | 13,2137 | `P76SUB1` | 26222 | `call_sub1` |
| P76.agc:100 | 13,2140 | `VLOAD VSR*` | 53775 | `sub1_ret` |
| P76.agc:101 | 13,2141 | `6` | 00007 | `sub1_ret` |
| P76.agc:102 | 13,2142 | `0,2` | 57176 | `sub1_ret` |
| P76.agc:103 | 13,2143 | `STOVL VCV` | 25543 | `sub1_ret` |
| P76.agc:104 | 13,2144 | `RATT` | 00001 | `sub1_ret` |
| P76.agc:105 | 13,2145 | `VSR*` | 77657 | `sub1_ret` |
| P76.agc:106 | 13,2146 | `0,2` | 57176 | `sub1_ret` |
| P76.agc:107 | 13,2147 | `STODL RCV` | 15535 | `sub1_ret` |
| P76.agc:108 | 13,2150 | `TIG` | 03413 | `sub1_ret` |
| P76.agc:109 | 13,2151 | `STORE TET` | 01517 | `sub1_ret` |
| P76.agc:110 | 13,2152 | `CLEAR DLOAD` | 71214 | `sub1_ret` |
| P76.agc:111 | 13,2153 | `INTYPFLG` | 01673 | `sub1_ret` |
| P76.agc:112 | 13,2154 | `TETTHIS` | 01571 | `sub1_ret` |
| P76.agc:113 | 13,2155 | `INTOTHIS STCALL TDEC1` | 34041 | `sub1_ret` |
| P76.agc:114 | 13,2156 | `INTEGRVS` | 27135 | `sub1_ret` |
| P76.agc:115 | 13,2157 | `CALL` | 77624 | `integrvs_ret` |
| P76.agc:116 | 13,2160 | `INTSTALL` | 27442 | `integrvs_ret` |
| P76.agc:117 | 13,2161 | `CALL` | 77624 | `call_sub1_2` |
| P76.agc:118 | 13,2162 | `P76SUB1` | 26222 | `call_sub1_2` |
| P76.agc:119 | 13,2163 | `VLOAD` | 77775 | `sub1_ret2` |
| P76.agc:120 | 13,2164 | `RATT1` | 00017 | `sub1_ret2` |
| P76.agc:121 | 13,2165 | `STORE RRECT` | 01503 | `sub1_ret2` |
| P76.agc:122 | 13,2166 | `STODL RCV` | 15535 | `sub1_ret2` |
| P76.agc:123 | 13,2167 | `TAT` | 00015 | `sub1_ret2` |
| P76.agc:124 | 13,2170 | `STOVL TET` | 25517 | `sub1_ret2` |
| P76.agc:125 | 13,2171 | `VATT1` | 00025 | `sub1_ret2` |
| P76.agc:126 | 13,2172 | `CALL` | 77624 | `sub1_ret2` |
| P76.agc:127 | 13,2173 | `MINIRECT` | 23435 | `sub1_ret2` |
| P76.agc:128 | 13,2174 | `EXIT` | 77776 | `minirect_ret` |
| P76.agc:129 | 13,2175 | `TC PHASCHNG` | 05412 | `phaschng` |
| P76.agc:130 | 13,2176 | `OCT 04024` | 04024 | `phaschng` |
| P76.agc:132 | 13,2177 | `TC UPFLAG` | 05546 | `phaschng_ret` |
| P76.agc:133 | 13,2200 | `ADRES REINTFLG` | 00236 | `phaschng_ret` |
| P76.agc:135 | 13,2201 | `TC INTPRET` | 06006 | `reintflg_ret` |
| P76.agc:136 | 13,2202 | `CALL` | 77624 | `call_atopoth` |
| P76.agc:137 | 13,2203 | `ATOPOTH` | 26760 | `call_atopoth` |
| P76.agc:138 | 13,2204 | `SSP EXIT` | 77531 | `atopoth_ret` |
| P76.agc:139 | 13,2205 | `QPRET` | 00053 | `atopoth_ret` |
| P76.agc:140 | 13,2206 | `OUT` | 26211 | `atopoth_ret` |
| P76.agc:141 | 13,2207 | `TC BANKCALL` | 04662 | `intwake` |
| P76.agc:142 | 13,2210 | `CADR INTWAKE1` | 27477 | `intwake` |
| P76.agc:143 | 13,2211 | `OUT EXIT` | 77776 | `out` |
| P76.agc:144 | 13,2212 | `ENDP76 CAF ZERO` | 35021 | `r_endp76` |
| P76.agc:145 | 13,2213 | `TS MARKCTR` | 55126 | `r_endp76` |
| P76.agc:146 | 13,2214 | `TS VHFCNT` | 55125 | `r_endp76` |
| P76.agc:148 | 13,2215 | `CAF NEGONE` | 37716 | `r_endp76` |
| P76.agc:149 | 13,2216 | `TS MRKBUF2` | 55734 | `r_endp76` |
| P76.agc:151 | 13,2217 | `TCF GOTOPOOH` | 14106 | `r_endp76` |
| P76.agc:153 | 13,2220 | `V06N84 NV 0684` | 01524 | `r_endp76` |
| P76.agc:154 | 13,2221 | `NV 0633` | 01441 | `r_endp76` |
| P76.agc:155 | 13,2222 | `P76SUB1 CLEAR SLOAD` | 67214 | `p76sub1` |
| P76.agc:156 | 13,2223 | `MOONFLAG` | 00263 | `p76sub1` |
| P76.agc:157 | 13,2224 | `X2` | 00050 | `p76sub1` |
| P76.agc:158 | 13,2225 | `BHIZ SET` | 43030 | `p76sub1` |
| P76.agc:159 | 13,2226 | `+2` | 26230 | `p76sub1` |
| P76.agc:160 | 13,2227 | `MOONFLAG` | 00063 | `p76sub1` |
| P76.agc:161 | 13,2230 | `RVQ` | 77616 | `p76sub1` |

## Evidence

Hashes are in `evidence/cm-p76/SHA256.json`. Bulk files are assets of the
`evidence-cm-p76` release (`python3 tools/evidence.py fetch cm-p76`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/cm_p76_source.py` reassembles the unchanged Comanche055
  with the pinned yaYUL. The rope equals the certified fixed-data rope.
- **Oracle.** `tools/cm_p76_reference.c` (GPL, host only) runs the
  unchanged Comanche055 (`deps/cm-rtb-fixture/MAIN.agc.bin`, equal to the
  certified rope) in the pinned yaAGC hardware-DV variant from 13824
  entries, 512 per routine, to the same stops.
  - The entries sit over poisoned debris with any vectors, times and flags.
  - X2 is zero in one case in two, so both BHIZ outcomes run.
  - EB is E7 three times in four.
  - QPRET is one of P76SUB1's real returns (13,2140 or 13,2163) three times
    in four, else a CADR in another bank.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/cm_p76_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`cm-p76-source-stream.json`, SHA256
  `e0bd74c3...584c97dd`) and archives the stream
  (`cm-p76-reference.snapshots.bin.xz`). `cm-p76-proof.json` lists the stops
  per routine.
- **Host and sanitizer results.** Run `make test-cm-p76 sanitize-cm-p76`.
  The strict build and the ASAN/UBSAN build each pass 14154 checks:
  - all 13824 frozen digests;
  - every run ending at one of the stops;
  - 273 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, and VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones.

  See `cm-p76-checks.log`.
- **C64 execution results.** Run `make vice-cm-p76`. Two PRGs each register
  only the routines their cases reach:
  - group 1: COMPMAT (UNIT, VXV, VXM) (128 cases);
  - group 2: every other entry (520 cases).

  Each keeps the image in the RAM under KERNAL ROM (`$01=$35`). The case
  generator is integer-only, so the C64 builds the same entry images as the
  host. In VICE (PAL, stock ROMs) the two PRGs pass 128 and 520 checks
  with no failures (`cm-p76-results.json`).

## Limits

- OTHPREC, INTSTALL, INTEGRVS, MINIRECT, ATOPOTH, INTWAKE1, UPFLAG,
  PHASCHNG and GOFLASH belong to other files and are stops here.
- Numerical validity is the original's. No wider claim is made.
