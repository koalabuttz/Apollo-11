# P76: target delta V

Lane E. Milestone `lm-p76`.

## Scope

All of `Luminary099/P76.agc`: bank 13 `02207`-`02350`, 98 words on 98
source lines. Sixteen basic routines are keyed by Z. Twelve interpretive
routines are keyed by LOC under BANKSET at NEWOPS. All use the
control-transfer ABI (`include/agc_transfer.h`, `docs/NATIVE_TRANSFER_ABI.md`).
`include/lm_p76_routines.h` lists each entry.

- **Entry (basic):** P76 sets TRACKFLG through UPFLAG and enters INTPRET.
- **Save (interpretive):** DELVLVC is stored into DELVOV, then EXIT.
- **Displays (basic):** V06N84 (DELVOV) and V06N33 (TIG) go through
  GOFLASH. Terminate goes to ENDP76, proceed to the next step, and recycle
  back to the previous display. The second proceed enters INTPRET.
- **Other vehicle (interpretive):** TIG goes to TDEC1 and OTHPREC is
  called. COMPMAT builds the local frame from RATT and VATT (UNIT, VXV),
  rotates DELVOV through it (VXM, VSL1), adds VATT and calls INTSTALL.
  The returns call P76SUB1, set up VCV, RCV and TET from the vehicle state
  scaled by X2 (VSR* 0,2), clear INTYPFLG and integrate to TETTHIS
  (INTEGRVS). After INTSTALL, RRECT, RCV and TET come from RATT1, TAT and
  VATT1, and MINIRECT is called. Then EXIT.
- **Restart (basic):** PHASCHNG with OCT 04024, REINTFLG set through
  UPFLAG, INTPRET.
- **Finish (interpretive):** CALL ATOPOTH, then SSP QPRET OUT and EXIT into
  BANKCALL INTWAKE1. OUT is EXIT into ENDP76.
- **ENDP76 (basic):** MARKCTR zeroed, then GOTOPOOH.
- **P76SUB1 (interpretive):** X2 = 2 and MOONFLAG set when CMOONFLG is on,
  returning to C(QPRET); otherwise X2 = 0, MOONFLAG cleared, RVQ.

Each run ends at the first point outside these routines
(`tests/lm_p76_cases.c`, `lm_p76_stop`). That is one of:

- UPFLAG, PHASCHNG, BANKCALL, GOTOPOOH or INTPRET;
- POODOO1 (alarm 1302);
- a NEWOPS outside bank 13's `2212`-`2215`, `2233`-`2315`, `2323`-`2332`
  and `2340`-`2350`: OTHPREC, INTSTALL, INTEGRVS, MINIRECT, ATOPOTH, or
  P76SUB1's return to C(QPRET).

Every interpretive pair is compiled with the shared bodies
(`src/lm_interp_steps.c`, `src/lm_interp_orders.c`). COMPMAT's VXM runs
the shared work-matrix body (`src/lm_work_matrix.c`). V06N84, V06N84 +1
and ZERO are literals that the source map checks against the fresh rope.
Each basic instruction is one `ai_*` register effect on the image, in
source order.

Interpretive routines admit a canonical NEWOPS image at their LOC in FB 13
meeting all of these conditions, and otherwise return `EXEC_OUT_OF_SCOPE`
with the image unchanged:

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

`tools/lm_p76_source.py` checks this map
(`evidence/lm-p76/lm-p76-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, or a word of the basic routine whose ENTRY names that address. The
erasable word and destinations the basic code names, and the three fixed
constants, must equal their native values in the fresh assembly.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P76.agc:57 | 13,2207 | `P76 TC UPFLAG` | 05504 | `p76` |
| P76.agc:58 | 13,2210 | `ADRES TRACKFLG` | 00031 | `p76` |
| P76.agc:60 | 13,2211 | `TC INTPRET` | 06037 | `trackflg_ret` |
| P76.agc:61 | 13,2212 | `VLOAD` | 77775 | `p76_save` |
| P76.agc:62 | 13,2213 | `DELVLVC` | 03434 | `p76_save` |
| P76.agc:63 | 13,2214 | `STORE DELVOV` | 02223 | `p76_save` |
| P76.agc:64 | 13,2215 | `EXIT` | 77776 | `p76_save` |
| P76.agc:66 | 13,2216 | `CAF V06N84` | 32336 | `r_flash84` |
| P76.agc:67 | 13,2217 | `TC BANKCALL` | 04616 | `r_flash84` |
| P76.agc:68 | 13,2220 | `CADR GOFLASH` | 20476 | `r_flash84` |
| P76.agc:69 | 13,2221 | `TCF ENDP76` | 12333 | `f84_terminate` |
| P76.agc:70 | 13,2222 | `TC +2` | 02224 | `f84_proceed` |
| P76.agc:71 | 13,2223 | `TC -5` | 02216 | `f84_recycle` |
| P76.agc:72 | 13,2224 | `CAF V06N84 +1` | 32337 | `r_flash33` |
| P76.agc:73 | 13,2225 | `TC BANKCALL` | 04616 | `r_flash33` |
| P76.agc:74 | 13,2226 | `CADR GOFLASH` | 20476 | `r_flash33` |
| P76.agc:75 | 13,2227 | `TCF ENDP76` | 12333 | `f33_terminate` |
| P76.agc:76 | 13,2230 | `TC +2` | 02232 | `f33_proceed` |
| P76.agc:77 | 13,2231 | `TC -5` | 02224 | `f33_recycle` |
| P76.agc:78 | 13,2232 | `TC INTPRET` | 06037 | `r_intpret` |
| P76.agc:80 | 13,2233 | `DLOAD` | 77745 | `p76_tig` |
| P76.agc:81 | 13,2234 | `TIG` | 03442 | `p76_tig` |
| P76.agc:82 | 13,2235 | `STCALL TDEC1` | 34041 | `p76_tig` |
| P76.agc:83 | 13,2236 | `OTHPREC` | 27043 | `p76_tig` |
| P76.agc:84 | 13,2237 | `COMPMAT VLOAD UNIT` | 53575 | `compmat` |
| P76.agc:85 | 13,2240 | `RATT` | 00001 | `compmat` |
| P76.agc:86 | 13,2241 | `VCOMP` | 77676 | `compmat` |
| P76.agc:87 | 13,2242 | `STORE 24D` | 00031 | `compmat` |
| P76.agc:88 | 13,2243 | `VXV UNIT` | 53435 | `compmat` |
| P76.agc:89 | 13,2244 | `VATT` | 00007 | `compmat` |
| P76.agc:90 | 13,2245 | `STORE 18D` | 00023 | `compmat` |
| P76.agc:91 | 13,2246 | `VXV UNIT` | 53435 | `compmat` |
| P76.agc:92 | 13,2247 | `24D` | 00031 | `compmat` |
| P76.agc:93 | 13,2250 | `STOVL 12D` | 24015 | `compmat` |
| P76.agc:94 | 13,2251 | `DELVOV` | 02223 | `compmat` |
| P76.agc:95 | 13,2252 | `VXM VSL1` | 76505 | `compmat` |
| P76.agc:96 | 13,2253 | `12D` | 00015 | `compmat` |
| P76.agc:97 | 13,2254 | `VAD` | 77655 | `compmat` |
| P76.agc:98 | 13,2255 | `VATT` | 00007 | `compmat` |
| P76.agc:99 | 13,2256 | `STORE 6` | 00007 | `compmat` |
| P76.agc:100 | 13,2257 | `CALL` | 77624 | `compmat` |
| P76.agc:101 | 13,2260 | `INTSTALL` | 27414 | `compmat` |
| P76.agc:102 | 13,2261 | `CALL` | 77624 | `call_sub1` |
| P76.agc:103 | 13,2262 | `P76SUB1` | 26340 | `call_sub1` |
| P76.agc:104 | 13,2263 | `VLOAD VSR*` | 53775 | `sub1_ret` |
| P76.agc:105 | 13,2264 | `6` | 00007 | `sub1_ret` |
| P76.agc:106 | 13,2265 | `0,2` | 57176 | `sub1_ret` |
| P76.agc:107 | 13,2266 | `STOVL VCV` | 25543 | `sub1_ret` |
| P76.agc:108 | 13,2267 | `RATT` | 00001 | `sub1_ret` |
| P76.agc:109 | 13,2270 | `VSR*` | 77657 | `sub1_ret` |
| P76.agc:110 | 13,2271 | `0,2` | 57176 | `sub1_ret` |
| P76.agc:111 | 13,2272 | `STODL RCV` | 15535 | `sub1_ret` |
| P76.agc:112 | 13,2273 | `TIG` | 03442 | `sub1_ret` |
| P76.agc:113 | 13,2274 | `STORE TET` | 01517 | `sub1_ret` |
| P76.agc:114 | 13,2275 | `CLEAR DLOAD` | 71214 | `sub1_ret` |
| P76.agc:115 | 13,2276 | `INTYPFLG` | 01673 | `sub1_ret` |
| P76.agc:116 | 13,2277 | `TETTHIS` | 01643 | `sub1_ret` |
| P76.agc:117 | 13,2300 | `INTOTHIS STCALL TDEC1` | 34041 | `sub1_ret` |
| P76.agc:118 | 13,2301 | `INTEGRVS` | 27107 | `sub1_ret` |
| P76.agc:119 | 13,2302 | `CALL` | 77624 | `integrvs_ret` |
| P76.agc:120 | 13,2303 | `INTSTALL` | 27414 | `integrvs_ret` |
| P76.agc:121 | 13,2304 | `VLOAD` | 77775 | `intstall_ret` |
| P76.agc:122 | 13,2305 | `RATT1` | 00017 | `intstall_ret` |
| P76.agc:123 | 13,2306 | `STORE RRECT` | 01503 | `intstall_ret` |
| P76.agc:124 | 13,2307 | `STODL RCV` | 15535 | `intstall_ret` |
| P76.agc:125 | 13,2310 | `TAT` | 00015 | `intstall_ret` |
| P76.agc:126 | 13,2311 | `STOVL TET` | 25517 | `intstall_ret` |
| P76.agc:127 | 13,2312 | `VATT1` | 00025 | `intstall_ret` |
| P76.agc:128 | 13,2313 | `CALL` | 77624 | `intstall_ret` |
| P76.agc:129 | 13,2314 | `MINIRECT` | 23455 | `intstall_ret` |
| P76.agc:131 | 13,2315 | `EXIT` | 77776 | `minirect_ret` |
| P76.agc:132 | 13,2316 | `TC PHASCHNG` | 05353 | `phaschng` |
| P76.agc:133 | 13,2317 | `OCT 04024` | 04024 | `phaschng` |
| P76.agc:135 | 13,2320 | `TC UPFLAG` | 05504 | `phaschng_ret` |
| P76.agc:136 | 13,2321 | `ADRES REINTFLG` | 00236 | `phaschng_ret` |
| P76.agc:138 | 13,2322 | `TC INTPRET` | 06037 | `reintflg_ret` |
| P76.agc:139 | 13,2323 | `CALL` | 77624 | `call_atopoth` |
| P76.agc:140 | 13,2324 | `ATOPOTH` | 26661 | `call_atopoth` |
| P76.agc:141 | 13,2325 | `SSP EXIT` | 77531 | `atopoth_ret` |
| P76.agc:142 | 13,2326 | `QPRET` | 00053 | `atopoth_ret` |
| P76.agc:143 | 13,2327 | `OUT` | 26332 | `atopoth_ret` |
| P76.agc:144 | 13,2330 | `TC BANKCALL` | 04616 | `intwake` |
| P76.agc:145 | 13,2331 | `CADR INTWAKE1` | 27445 | `intwake` |
| P76.agc:146 | 13,2332 | `OUT EXIT` | 77776 | `out` |
| P76.agc:147 | 13,2333 | `ENDP76 CAF ZERO` | 34755 | `r_endp76` |
| P76.agc:148 | 13,2334 | `TS MARKCTR` | 55462 | `r_endp76` |
| P76.agc:149 | 13,2335 | `TCF GOTOPOOH` | 16001 | `r_endp76` |
| P76.agc:151 | 13,2336 | `V06N84 NV 0684` | 01524 | `r_endp76` |
| P76.agc:152 | 13,2337 | `NV 0633` | 01441 | `r_endp76` |
| P76.agc:153 | 13,2340 | `P76SUB1 AXT,2 SET` | 43174 | `p76sub1` |
| P76.agc:154 | 13,2341 | `2` | 00002 | `p76sub1` |
| P76.agc:155 | 13,2342 | `MOONFLAG` | 00063 | `p76sub1` |
| P76.agc:156 | 13,2343 | `BON AXT,2` | 77014 | `p76sub1` |
| P76.agc:157 | 13,2344 | `CMOONFLG` | 04303 | `p76sub1` |
| P76.agc:158 | 13,2345 | `QPRET` | 00052 | `p76sub1` |
| P76.agc:159 | 13,2346 | `0` | 00000 | `p76sub1` |
| P76.agc:160 | 13,2347 | `CLEAR RVQ` | 43414 | `p76sub1` |
| P76.agc:161 | 13,2350 | `MOONFLAG` | 00263 | `p76sub1` |

## Evidence

Hashes are in `evidence/lm-p76/SHA256.json`. Bulk files are assets of the
`evidence-lm-p76` release (`python3 tools/evidence.py fetch lm-p76`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_p76_source.py` reassembles the unchanged file with the
  pinned yaYUL. The rope equals the certified fixed-data rope.
- **Oracle.** `tools/lm_p76_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant from 14336
  entries, 512 per routine, to the same stops.
  - The entries sit over poisoned debris with any vectors, times and flags.
  - EB is E7 three times in four.
  - QPRET is P76SUB1's real return (13,2263) three times in four, else a
    CADR in another bank.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/lm_p76_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`lm-p76-source-stream.json`, SHA256
  `3dd445f0...7078cfe`) and archives the stream
  (`lm-p76-reference.snapshots.bin.xz`). `lm-p76-proof.json` lists the stops
  per routine.
- **Host and sanitizer results.** Run `make test-lm-p76 sanitize-lm-p76`.
  The strict build and the ASAN/UBSAN build each pass 14675 checks:
  - all 14336 frozen digests;
  - every run ending at one of the stops;
  - 280 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, and VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones.

  See `lm-p76-checks.log`.
- **C64 execution results.** Run `make vice-lm-p76`. Two PRGs each register
  only the routines their cases reach:
  - group 1: COMPMAT (UNIT, VXV, VXM) (128 cases);
  - group 2: every other entry (520 cases).

  Each keeps the image in the RAM under KERNAL ROM (`$01=$35`). The case
  generator is integer-only, so the C64 builds the same entry images as the
  host. In VICE (PAL, stock ROMs) the two PRGs pass 128 and 520 checks
  with no failures (`lm-p76-results.json`).

## Limits

- OTHPREC, INTSTALL, INTEGRVS, MINIRECT, ATOPOTH, INTWAKE1, UPFLAG,
  PHASCHNG and GOFLASH belong to other files and are stops here.
- Numerical validity is the original's. No wider claim is made.
