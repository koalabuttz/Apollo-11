# P31: Lambert aimpoint guidance

Lane E. Milestone `lm-p31`.

## Scope

All of `Luminary099/GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc`: bank 35
`02450`-`02544`, 61 words on 61 source lines. Six basic routines are keyed
by Z. Eleven interpretive routines are keyed by LOC under BANKSET at
NEWOPS. All use the control-transfer ABI (`include/agc_transfer.h`,
`docs/NATIVE_TRANSFER_ABI.md`). `include/lm_p31_routines.h` lists each
entry.

- **Entry (basic):** P31 calls P20FLGON and shows V06N33 (TIG) through
  VNPOOH, then enters INTPRET.
- **Setup (interpretive):** clears UPDATFLG and integrates to TIG
  (LEMPREC). Where LEMPREC returns:
  - RTIG and RINIT from RATT, VTIG and VINIT from VATT;
  - P30ZERO pushed (E4 and NUMIT zero) and DELLT4 pushed;
  - TPASS4 = TIG + DELLT4, with X1 from RTX1 and X2 from RTX2;
  - CALL INITVEL.
- **Results (interpretive):** DELVEET3 is pushed and stored into DELVSIN.
  XDELVFLG is cleared, |DELVSIN| goes to VGDISP, and GET.LVC is called.
  Then RTIG is pushed, VIPRIME loaded and PERIAPO1 called. SHIFTR1 and
  MAXCHK run twice, storing HPER and then HAPO, and EXIT.
- **Displays (basic):** V06N81 and V06N42 through VNPOOH, then INTPRET.
- **REVN1645 (interpretive):** SET FINALFLG, CALL VN1645, and GOTO
  REVN1645 where it returns.

Each run ends at the first point outside these routines
(`tests/lm_p31_cases.c`, `lm_p31_stop`). That is one of:

- P20FLGON (P32-P35_P72-P75), VNPOOH (lm-p34) or INTPRET;
- ABVAL's POODOO1 (alarm 1302);
- a NEWOPS outside `35,2454`-`2544`: LEMPREC, INITVEL, GET.LVC, PERIAPO1,
  SHIFTR1, MAXCHK or VN1645.

Every interpretive pair is compiled with the shared bodies
(`src/lm_interp_steps.c`, `src/lm_interp_orders.c`). V06N33, V06N81, V06N42
and the P30ZERO pair are literals that the source map checks against the
fresh rope. Each basic instruction is one `ai_*` register effect on the
image, in source order.

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

`tools/lm_p31_source.py` checks this map
(`evidence/lm-p31/lm-p31-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, or a word of the basic routine whose ENTRY names that address. The
destinations the basic code names, and the five fixed constant words, must
equal their native values in the fresh assembly.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:104 | 35,2450 | `P31 TC P20FLGON` | 02361 | `p31` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:105 | 35,2451 | `CAF V06N33` | 32025 | `p20_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:106 | 35,2452 | `TC VNPOOH` | 03712 | `p20_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:107 | 35,2453 | `TC INTPRET` | 06037 | `vnpooh_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:108 | 35,2454 | `CLEAR DLOAD` | 71214 | `p31_start` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:109 | 35,2455 | `UPDATFLG` | 00670 | `p31_start` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:110 | 35,2456 | `TIG` | 03442 | `p31_start` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:111 | 35,2457 | `STCALL TDEC1` | 34041 | `p31_start` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:112 | 35,2460 | `LEMPREC` | 27057 | `p31_start` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:113 | 35,2461 | `VLOAD SETPD` | 40375 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:114 | 35,2462 | `RATT` | 00001 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:115 | 35,2463 | `0D` | 00001 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:116 | 35,2464 | `STORE RTIG` | 03642 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:117 | 35,2465 | `STOVL RINIT` | 26323 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:118 | 35,2466 | `VATT` | 00007 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:119 | 35,2467 | `STORE VTIG` | 03650 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:120 | 35,2470 | `STODL VINIT` | 16331 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:121 | 35,2471 | `P30ZERO` | 32420 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:122 | 35,2472 | `PUSH PDDL` | 65206 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:123 | 35,2473 | `DELLT4` | 03452 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:124 | 35,2474 | `DAD SXA,1` | 66015 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:125 | 35,2475 | `TIG` | 03442 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:126 | 35,2476 | `RTX1` | 02776 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:127 | 35,2477 | `STORE TPASS4` | 03631 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:128 | 35,2500 | `SXA,2 CALL` | 45134 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:129 | 35,2501 | `RTX2` | 02777 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:130 | 35,2502 | `INITVEL` | 22000 | `lemprec_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:131 | 35,2503 | `VLOAD PUSH` | 41575 | `initvel_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:133 | 35,2504 | `DELVEET3` | 02366 | `initvel_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:134 | 35,2505 | `STORE DELVSIN` | 03656 | `initvel_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:135 | 35,2506 | `ABVAL CLEAR` | 43046 | `initvel_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:136 | 35,2507 | `XDELVFLG` | 01267 | `initvel_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:137 | 35,2510 | `STCALL VGDISP` | 37664 | `initvel_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:138 | 35,2511 | `GET.LVC` | 15733 | `initvel_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:139 | 35,2512 | `VLOAD PDVL` | 63375 | `getlvc_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:140 | 35,2513 | `RTIG` | 03642 | `getlvc_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:141 | 35,2514 | `VIPRIME` | 02337 | `getlvc_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:142 | 35,2515 | `CALL` | 77624 | `getlvc_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:143 | 35,2516 | `PERIAPO1` | 46316 | `getlvc_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:144 | 35,2517 | `CALL` | 77624 | `periapo_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:145 | 35,2520 | `SHIFTR1` | 46426 | `periapo_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:146 | 35,2521 | `CALL` | 77624 | `shiftr1_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:147 | 35,2522 | `MAXCHK` | 45636 | `shiftr1_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:148 | 35,2523 | `STODL HPER` | 16321 | `maxchk_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:149 | 35,2524 | `4D` | 00005 | `maxchk_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:150 | 35,2525 | `CALL` | 77624 | `maxchk_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:151 | 35,2526 | `SHIFTR1` | 46426 | `maxchk_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:152 | 35,2527 | `CALL` | 77624 | `shiftr1_ret2` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:153 | 35,2530 | `MAXCHK` | 45636 | `shiftr1_ret2` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:154 | 35,2531 | `STORE HAPO` | 02317 | `maxchk_ret2` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:155 | 35,2532 | `EXIT` | 77776 | `maxchk_ret2` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:156 | 35,2533 | `CAF V06N81` | 33727 | `v06n81` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:157 | 35,2534 | `TC VNPOOH` | 03712 | `v06n81` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:158 | 35,2535 | `CAF V06N42` | 32026 | `v06n42` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:159 | 35,2536 | `TC VNPOOH` | 03712 | `v06n42` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:160 | 35,2537 | `TC INTPRET` | 06037 | `v06n42_ret` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:161 | 35,2540 | `REVN1645 SET CALL` | 45014 | `revn1645` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:162 | 35,2541 | `FINALFLG` | 01071 | `revn1645` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:163 | 35,2542 | `VN1645` | 73606 | `revn1645` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:164 | 35,2543 | `GOTO` | 77650 | `loop` |
| GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc:165 | 35,2544 | `REVN1645` | 72540 | `loop` |

## Evidence

Hashes are in `evidence/lm-p31/SHA256.json`. Bulk files are assets of the
`evidence-lm-p31` release (`python3 tools/evidence.py fetch lm-p31`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_p31_source.py` reassembles the unchanged file with the
  pinned yaYUL. The rope equals the certified fixed-data rope.
- **Oracle.** `tools/lm_p31_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant from 8704
  entries, 512 per routine, to the same stops.
  - The entries sit over poisoned debris with any vectors, times and flags.
  - EB is E7 three times in four.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/lm_p31_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`lm-p31-source-stream.json`, SHA256
  `ad1ae4ae...c0d81c2f`) and archives the stream
  (`lm-p31-reference.snapshots.bin.xz`). `lm-p31-proof.json` lists the stops
  per routine.
- **Host and sanitizer results.** Run `make test-lm-p31 sanitize-lm-p31`.
  The strict build and the ASAN/UBSAN build each pass 8937 checks:
  - all 8704 frozen digests;
  - every run ending at one of the stops;
  - 196 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, and VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones.

  See `lm-p31-checks.log`.
- **C64 execution results.** Run `make vice-lm-p31`. One PRG registers
  every routine and runs 510 cases, 30 per entry. It keeps the image in the
  RAM under KERNAL ROM (`$01=$35`). The case generator is integer-only, so
  the C64 builds the same entry images as the host. In VICE (PAL, stock ROMs) it passes 510 checks with no
  failures (`lm-p31-results.json`).

## Limits

- LEMPREC, INITVEL, GET.LVC, PERIAPO1, SHIFTR1, MAXCHK, VN1645, P20FLGON
  and VNPOOH belong to other files or milestones and are stops here.
- Numerical validity is the original's. No wider claim is made.
