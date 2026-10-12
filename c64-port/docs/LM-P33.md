# P33/P73: CDH targeting, with AVFLAGA, AVFLAGP, P20FLGON and DISDVLVC

Lane E. Milestone `lm-p33`.

## Scope

From `Luminary099/P32-P35_P72-P75.agc`, bank 35 `02216`-`02407`: 122 words
on 122 source lines. Twenty-three basic routines are keyed by Z. Twenty
interpretive routines are keyed by LOC under BANKSET at NEWOPS. All use the
control-transfer ABI (`include/agc_transfer.h`,
`docs/NATIVE_TRANSFER_ABI.md`). `include/lm_p33_routines.h` lists each
entry.

- **Entry (basic):** P33 and P73 go through AVFLAGA or AVFLAGP. P33/P73A
  calls P20FLGON, shows V06N13 (TCDH) through VNPOOH and enters INTPRET.
- **Setup (interpretive):** TTPI0 into TTPI and TCDH into TIG, then
  SELECTMU. P33/P73B calls ADVANCE and CDHMVR, integrates both vehicles
  (INTINT3P from VACT3 and RACT2, then ACTIVE; from VPASS2 and RPASS2, then
  PASSIVE), sets ITSWICH, zeroes NOMTPI and calls S33/34.1.
- **Alarm (basic):** a nonzero S33/34.1 result exits to the 611 alarm and
  V05N09 through GOFLASH. Terminate goes to GOTOPOOH, recycle to
  P33/P73A, and proceed re-enters INTPRET, which zeroes NOMTPI.
- **Times (interpretive):** P33/P73C sets UPDATFLG unless FINALFLG is on.
  P33/P73D adds NOMTPI to TTPI. P33/P73E takes 60MIN off TTPI - TCDH while
  it stays positive and stores T1TOT2. P33/P73F does the same for
  |TTPI - TTPI0|, restores its sign into T2TOT3 and exits.
- **Display and loop:** V06N75 through VNPOOH, then DELVEET2 through
  S32/33.1, VN1645, and GOTO P33/P73B.
- **AVFLAGA, AVFLAGP, P20FLGON (basic):** each saves Q in SUBEXIT. They
  set or clear AVFLAG, or set UPDATFLG and TRACKFLG, through UPFLAG and
  DOWNFLAG, then TC SUBEXIT.
- **DISDVLVC (interpretive):** stores DELVLVC, saves QPRET in NORMEX and
  calls S32/33.X. Where it returns, DELVLVC goes through the matrix at 0D
  (MXV, VSL1), X1 goes into VERBNOUN, and it exits. The basic code shows
  VERBNOUN through VNPOOH, re-enters INTPRET and goes to C(NORMEX).

Each run ends at the first point outside these routines
(`tests/lm_p33_cases.c`, `lm_p33_stop`). That is one of:

- UPFLAG, DOWNFLAG, VNPOOH (lm-p34), ALARM, BANKCALL, GOTOPOOH or INTPRET;
- TC SUBEXIT's jump to E `1470`;
- POODOO1 (alarm 1302);
- a NEWOPS outside `35,2225`-`2265`, `2277`-`2346`, `2370`-`2402` and
  `2406`-`2407`: SELECTMU, ADVANCE, CDHMVR, INTINT3P, ACTIVE, PASSIVE,
  S33/34.1, S32/33.1, S32/33.X, VN1645 or the CADR in NORMEX.

Every interpretive pair is compiled with the shared bodies
(`src/lm_interp_steps.c`, `src/lm_interp_orders.c`). MXV runs the shared
work-matrix body (`src/lm_work_matrix.c`). The loops of P33/P73E and
P33/P73F run in their routines, each taken BPL ending at its label's
NEWOPS. V06N13, V05N09, V06N75 and the P30ZERO and 60MIN pairs are literals
that the source map checks against the fresh rope. Each basic instruction
is one `ai_*` register effect on the image, in source order.

Interpretive routines admit a canonical NEWOPS image at their LOC in FB 35
meeting all of these conditions, and otherwise return `EXEC_OUT_OF_SCOPE`
with the image unchanged:

- EB and BB coherent, with BANKSET = BB;
- INTBIT15 set;
- EDOP, ZERO and NEWJOB zero;
- MODE `+0`, `+1` or vector;
- the VAC one of the five, with PUSHLOC no further than 32D into it;
- IRQ 0 or 1, and an allowed SUPERBNK;
- every word 15 bits.

Basic routines admit Z at their entry in FB 35, with EB and BB coherent,
ZERO zero, IRQ 0 or 1, an allowed SUPERBNK and every word 15 bits.

## Source map

`tools/lm_p33_source.py` checks this map
(`evidence/lm-p33/lm-p33-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, or a word of the basic routine whose ENTRY names that address. The
erasable words and destinations the basic code names, and the seven fixed
constant words, must equal their native values in the fresh assembly.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P32-P35_P72-P75.agc:482 | 35,2216 | `P33 TC AVFLAGA` | 02347 | `p33` |
| P32-P35_P72-P75.agc:483 | 35,2217 | `TC P33/P73A` | 02221 | `avflaga_ret` |
| P32-P35_P72-P75.agc:484 | 35,2220 | `P73 TC AVFLAGP` | 02354 | `p73` |
| P32-P35_P72-P75.agc:485 | 35,2221 | `P33/P73A TC P20FLGON` | 02361 | `p33a` |
| P32-P35_P72-P75.agc:486 | 35,2222 | `CAF V06N13` | 32411 | `p20_ret` |
| P32-P35_P72-P75.agc:487 | 35,2223 | `TC VNPOOH` | 03712 | `p20_ret` |
| P32-P35_P72-P75.agc:488 | 35,2224 | `TC INTPRET` | 06037 | `vnpooh_ret` |
| P32-P35_P72-P75.agc:489 | 35,2225 | `DLOAD` | 77745 | `p33a_interp` |
| P32-P35_P72-P75.agc:490 | 35,2226 | `TTPIO` | 03640 | `p33a_interp` |
| P32-P35_P72-P75.agc:491 | 35,2227 | `STODL TTPI` | 17636 | `p33a_interp` |
| P32-P35_P72-P75.agc:492 | 35,2230 | `TCDH` | 01777 | `p33a_interp` |
| P32-P35_P72-P75.agc:493 | 35,2231 | `STCALL TIG` | 37442 | `p33a_interp` |
| P32-P35_P72-P75.agc:494 | 35,2232 | `SELECTMU` | 20000 | `p33a_interp` |
| P32-P35_P72-P75.agc:495 | 35,2233 | `P33/P73B CALL` | 77624 | `p33b` |
| P32-P35_P72-P75.agc:496 | 35,2234 | `ADVANCE` | 71016 | `p33b` |
| P32-P35_P72-P75.agc:497 | 35,2235 | `CALL` | 77624 | `advance_ret` |
| P32-P35_P72-P75.agc:498 | 35,2236 | `CDHMVR` | 71133 | `advance_ret` |
| P32-P35_P72-P75.agc:499 | 35,2237 | `SETPD VLOAD` | 77201 | `cdhmvr_ret` |
| P32-P35_P72-P75.agc:500 | 35,2240 | `0D` | 00001 | `cdhmvr_ret` |
| P32-P35_P72-P75.agc:501 | 35,2241 | `VACT3` | 03544 | `cdhmvr_ret` |
| P32-P35_P72-P75.agc:502 | 35,2242 | `PDVL CALL` | 45115 | `cdhmvr_ret` |
| P32-P35_P72-P75.agc:503 | 35,2243 | `RACT2` | 02311 | `cdhmvr_ret` |
| P32-P35_P72-P75.agc:504 | 35,2244 | `INTINT3P` | 71071 | `cdhmvr_ret` |
| P32-P35_P72-P75.agc:505 | 35,2245 | `CALL` | 77624 | `intint_ret` |
| P32-P35_P72-P75.agc:506 | 35,2246 | `ACTIVE` | 46402 | `intint_ret` |
| P32-P35_P72-P75.agc:507 | 35,2247 | `SETPD VLOAD` | 77201 | `active_ret` |
| P32-P35_P72-P75.agc:508 | 35,2250 | `0D` | 00001 | `active_ret` |
| P32-P35_P72-P75.agc:509 | 35,2251 | `VPASS2` | 03530 | `active_ret` |
| P32-P35_P72-P75.agc:510 | 35,2252 | `PDVL CALL` | 45115 | `active_ret` |
| P32-P35_P72-P75.agc:511 | 35,2253 | `RPASS2` | 03522 | `active_ret` |
| P32-P35_P72-P75.agc:512 | 35,2254 | `INTINT3P` | 71071 | `active_ret` |
| P32-P35_P72-P75.agc:513 | 35,2255 | `CALL` | 77624 | `intint_ret2` |
| P32-P35_P72-P75.agc:514 | 35,2256 | `PASSIVE` | 46412 | `intint_ret2` |
| P32-P35_P72-P75.agc:515 | 35,2257 | `DLOAD SET` | 43145 | `passive_ret` |
| P32-P35_P72-P75.agc:516 | 35,2260 | `P30ZERO` | 32420 | `passive_ret` |
| P32-P35_P72-P75.agc:517 | 35,2261 | `ITSWICH` | 03460 | `passive_ret` |
| P32-P35_P72-P75.agc:518 | 35,2262 | `STCALL NOMTPI` | 36317 | `passive_ret` |
| P32-P35_P72-P75.agc:519 | 35,2263 | `S33/34.1` | 72764 | `passive_ret` |
| P32-P35_P72-P75.agc:520 | 35,2264 | `BZE EXIT` | 77454 | `s331_ret` |
| P32-P35_P72-P75.agc:521 | 35,2265 | `P33/P73C` | 72302 | `s331_ret` |
| P32-P35_P72-P75.agc:522 | 35,2266 | `TC ALARM` | 05567 | `r_alarm` |
| P32-P35_P72-P75.agc:523 | 35,2267 | `OCT 611` | 00611 | `r_alarm` |
| P32-P35_P72-P75.agc:524 | 35,2270 | `CAF V05N09` | 35006 | `alarm_ret` |
| P32-P35_P72-P75.agc:525 | 35,2271 | `TC BANKCALL` | 04616 | `alarm_ret` |
| P32-P35_P72-P75.agc:526 | 35,2272 | `CADR GOFLASH` | 20476 | `alarm_ret` |
| P32-P35_P72-P75.agc:527 | 35,2273 | `TC GOTOPOOH` | 06001 | `flash_terminate` |
| P32-P35_P72-P75.agc:528 | 35,2274 | `TC +2` | 02276 | `flash_proceed` |
| P32-P35_P72-P75.agc:530 | 35,2275 | `TC P33/P73A` | 02221 | `flash_recycle` |
| P32-P35_P72-P75.agc:531 | 35,2276 | `TC INTPRET` | 06037 | `r_intpret` |
| P32-P35_P72-P75.agc:532 | 35,2277 | `DLOAD` | 77745 | `proceed_ret` |
| P32-P35_P72-P75.agc:533 | 35,2300 | `P30ZERO` | 32420 | `proceed_ret` |
| P32-P35_P72-P75.agc:534 | 35,2301 | `STORE NOMTPI` | 02317 | `proceed_ret` |
| P32-P35_P72-P75.agc:535 | 35,2302 | `P33/P73C BON SET` | 43014 | `p33c` |
| P32-P35_P72-P75.agc:536 | 35,2303 | `FINALFLG` | 01311 | `p33c` |
| P32-P35_P72-P75.agc:537 | 35,2304 | `P33/P73D` | 72306 | `p33c` |
| P32-P35_P72-P75.agc:538 | 35,2305 | `UPDATFLG` | 00470 | `p33c` |
| P32-P35_P72-P75.agc:539 | 35,2306 | `P33/P73D DLOAD DAD` | 43345 | `p33d` |
| P32-P35_P72-P75.agc:540 | 35,2307 | `NOMTPI` | 02317 | `p33d` |
| P32-P35_P72-P75.agc:541 | 35,2310 | `TTPI` | 03636 | `p33d` |
| P32-P35_P72-P75.agc:542 | 35,2311 | `STORE TTPI` | 03636 | `p33d` |
| P32-P35_P72-P75.agc:543 | 35,2312 | `DSU` | 77625 | `p33d` |
| P32-P35_P72-P75.agc:544 | 35,2313 | `TCDH` | 01777 | `p33d` |
| P32-P35_P72-P75.agc:545 | 35,2314 | `P33/P73E DSU BPL` | 51025 | `p33e` |
| P32-P35_P72-P75.agc:546 | 35,2315 | `60MIN` | 32422 | `p33e` |
| P32-P35_P72-P75.agc:547 | 35,2316 | `P33/P73E` | 72314 | `p33e` |
| P32-P35_P72-P75.agc:548 | 35,2317 | `DAD` | 77615 | `p33e` |
| P32-P35_P72-P75.agc:549 | 35,2320 | `60MIN` | 32422 | `p33e` |
| P32-P35_P72-P75.agc:550 | 35,2321 | `STODL T1TOT2` | 16253 | `p33e` |
| P32-P35_P72-P75.agc:551 | 35,2322 | `TTPI` | 03636 | `p33e` |
| P32-P35_P72-P75.agc:552 | 35,2323 | `DSU PUSH` | 41425 | `p33e` |
| P32-P35_P72-P75.agc:553 | 35,2324 | `TTPIO` | 03640 | `p33e` |
| P32-P35_P72-P75.agc:554 | 35,2325 | `P33/P73F ABS DSU` | 45246 | `p33f` |
| P32-P35_P72-P75.agc:555 | 35,2326 | `60MIN` | 32422 | `p33f` |
| P32-P35_P72-P75.agc:556 | 35,2327 | `BPL DAD` | 43244 | `p33f` |
| P32-P35_P72-P75.agc:557 | 35,2330 | `P33/P73F` | 72325 | `p33f` |
| P32-P35_P72-P75.agc:558 | 35,2331 | `60MIN` | 32422 | `p33f` |
| P32-P35_P72-P75.agc:559 | 35,2332 | `SIGN STADR` | 45565 | `p33f` |
| P32-P35_P72-P75.agc:560 | 35,2333 | `STORE T2TOT3` | 75522 | `p33f` |
| P32-P35_P72-P75.agc:561 | 35,2334 | `EXIT` | 77776 | `p33f` |
| P32-P35_P72-P75.agc:562 | 35,2335 | `CAF V06N75` | 32412 | `v06n75` |
| P32-P35_P72-P75.agc:563 | 35,2336 | `TC VNPOOH` | 03712 | `v06n75` |
| P32-P35_P72-P75.agc:564 | 35,2337 | `TC INTPRET` | 06037 | `vnpooh_ret2` |
| P32-P35_P72-P75.agc:565 | 35,2340 | `VLOAD CALL` | 45175 | `vnpooh_ret_i` |
| P32-P35_P72-P75.agc:566 | 35,2341 | `DELVEET2` | 02275 | `vnpooh_ret_i` |
| P32-P35_P72-P75.agc:567 | 35,2342 | `S32/33.1` | 71100 | `vnpooh_ret_i` |
| P32-P35_P72-P75.agc:568 | 35,2343 | `STCALL DELVEET2` | 36275 | `s3233_ret` |
| P32-P35_P72-P75.agc:569 | 35,2344 | `VN1645` | 73606 | `s3233_ret` |
| P32-P35_P72-P75.agc:570 | 35,2345 | `GOTO` | 77650 | `loop` |
| P32-P35_P72-P75.agc:571 | 35,2346 | `P33/P73B` | 72233 | `loop` |
| P32-P35_P72-P75.agc:581 | 35,2347 | `AVFLAGA EXTEND` | 00006 | `avflaga` |
| P32-P35_P72-P75.agc:582 | 35,2350 | `QXCH SUBEXIT` | 23470 | `avflaga` |
| P32-P35_P72-P75.agc:583 | 35,2351 | `TC UPFLAG` | 05504 | `avflaga` |
| P32-P35_P72-P75.agc:584 | 35,2352 | `ADRES AVFLAG` | 00050 | `avflaga` |
| P32-P35_P72-P75.agc:585 | 35,2353 | `TC SUBEXIT` | 01470 | `avflaga_up` |
| P32-P35_P72-P75.agc:586 | 35,2354 | `AVFLAGP EXTEND` | 00006 | `avflagp` |
| P32-P35_P72-P75.agc:587 | 35,2355 | `QXCH SUBEXIT` | 23470 | `avflagp` |
| P32-P35_P72-P75.agc:588 | 35,2356 | `TC DOWNFLAG` | 05516 | `avflagp` |
| P32-P35_P72-P75.agc:589 | 35,2357 | `ADRES AVFLAG` | 00050 | `avflagp` |
| P32-P35_P72-P75.agc:590 | 35,2360 | `TC SUBEXIT` | 01470 | `avflagp_down` |
| P32-P35_P72-P75.agc:591 | 35,2361 | `P20FLGON EXTEND` | 00006 | `p20flgon` |
| P32-P35_P72-P75.agc:592 | 35,2362 | `QXCH SUBEXIT` | 23470 | `p20flgon` |
| P32-P35_P72-P75.agc:593 | 35,2363 | `TC UPFLAG` | 05504 | `p20flgon` |
| P32-P35_P72-P75.agc:594 | 35,2364 | `ADRES UPDATFLG` | 00027 | `p20flgon` |
| P32-P35_P72-P75.agc:595 | 35,2365 | `TC UPFLAG` | 05504 | `p20flgon_up` |
| P32-P35_P72-P75.agc:596 | 35,2366 | `ADRES TRACKFLG` | 00031 | `p20flgon_up` |
| P32-P35_P72-P75.agc:597 | 35,2367 | `TC SUBEXIT` | 01470 | `p20flgon_up2` |
| P32-P35_P72-P75.agc:607 | 35,2370 | `DISDVLVC STORE DELVLVC` | 03434 | `disdvlvc` |
| P32-P35_P72-P75.agc:608 | 35,2371 | `STQ CALL` | 45020 | `disdvlvc` |
| P32-P35_P72-P75.agc:609 | 35,2372 | `NORMEX` | 03463 | `disdvlvc` |
| P32-P35_P72-P75.agc:610 | 35,2373 | `S32/33.X` | 71120 | `disdvlvc` |
| P32-P35_P72-P75.agc:611 | 35,2374 | `VLOAD MXV` | 64375 | `s3233x_ret` |
| P32-P35_P72-P75.agc:612 | 35,2375 | `DELVLVC` | 03434 | `s3233x_ret` |
| P32-P35_P72-P75.agc:613 | 35,2376 | `0D` | 00001 | `s3233x_ret` |
| P32-P35_P72-P75.agc:614 | 35,2377 | `VSL1 SXA,1` | 66172 | `s3233x_ret` |
| P32-P35_P72-P75.agc:615 | 35,2400 | `VERBNOUN` | 03615 | `s3233x_ret` |
| P32-P35_P72-P75.agc:616 | 35,2401 | `STORE DELVLVC` | 03434 | `s3233x_ret` |
| P32-P35_P72-P75.agc:617 | 35,2402 | `EXIT` | 77776 | `s3233x_ret` |
| P32-P35_P72-P75.agc:618 | 35,2403 | `CA VERBNOUN` | 31615 | `disdvlvc_exit` |
| P32-P35_P72-P75.agc:619 | 35,2404 | `TC VNPOOH` | 03712 | `disdvlvc_exit` |
| P32-P35_P72-P75.agc:620 | 35,2405 | `TC INTPRET` | 06037 | `vnpooh_ret3` |
| P32-P35_P72-P75.agc:621 | 35,2406 | `GOTO` | 77650 | `normex_goto` |
| P32-P35_P72-P75.agc:622 | 35,2407 | `NORMEX` | 03463 | `normex_goto` |

## Evidence

Hashes are in `evidence/lm-p33/SHA256.json`. Bulk files are assets of the
`evidence-lm-p33` release (`python3 tools/evidence.py fetch lm-p33`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_p33_source.py` reassembles the unchanged file with the
  pinned yaYUL. The rope equals the certified fixed-data rope.
- **Oracle.** `tools/lm_p33_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant from 22016
  entries, 512 per routine, to the same stops.
  - The entries sit over poisoned debris with any times, vectors and flags.
  - MPAC is zero in one S33/34.1 return in two, so both BZE outcomes run.
  - EB is E7 three times in four, and NORMEX holds a CADR in banks 2-27.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/lm_p33_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`lm-p33-source-stream.json`, SHA256
  `91a56553...1ccf8fd5`) and archives the stream
  (`lm-p33-reference.snapshots.bin.xz`). `lm-p33-proof.json` lists the stops
  per routine.
- **Host and sanitizer results.** Run `make test-lm-p33 sanitize-lm-p33`.
  The strict build and the ASAN/UBSAN build each pass 22546 checks:
  - all 22016 frozen digests;
  - every run ending at one of the stops;
  - 441 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, and VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones.

  See `lm-p33-checks.log`.
- **C64 execution results.** Run `make vice-lm-p33`. Two PRGs each register
  only the routines their cases reach:
  - group 1: DISDVLVC's S32/33.X return (MXV) (128 cases);
  - group 2: every other entry (504 cases).

  Both are built with `-fno-inline-functions`. Each keeps the image in the
  RAM under KERNAL ROM (`$01=$35`). The case generator is integer-only, so
  the C64 builds the same entry images as the host. Group 2 runs the 60MIN
  loops from debris times, so `make vice-lm-p33` gives it a 9e9-cycle
  limit. In VICE (PAL, stock ROMs) the two PRGs pass 128 and 504 checks
  with no failures (`lm-p33-results.json`).

## Limits

- P32/P72 (35,2027-2215), the constants after DISDVLVC, COMPTGO and the
  bank 34 routines (CSI, ADVANCE, S32/33.1, S32/33.X, CDHMVR) are not
  covered. The file stays `partial`.
- SELECTMU, ADVANCE, CDHMVR, INTINT3P, ACTIVE, PASSIVE, S33/34.1,
  S32/33.1, S32/33.X, VN1645, VNPOOH, UPFLAG, DOWNFLAG, ALARM and GOFLASH
  belong to other files or milestones and are stops here.
- Numerical validity is the original's. No wider claim is made.
