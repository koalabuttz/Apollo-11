# P35/P75: the rendezvous midcourse programs' loop

Lane E. Milestone `lm-p35`.

## Scope

From `Luminary099/P34-35_P74-75.agc`, bank 35 `02725`-`02763`: 31 words on
31 source lines. Five basic routines are keyed by Z. Eight interpretive
routines are keyed by LOC under BANKSET at NEWOPS. All use the
control-transfer ABI (`include/agc_transfer.h`, `docs/NATIVE_TRANSFER_ABI.md`).

- **P35 (`35,2725`), basic:** TC AVFLAGA.
- **AVFLAGA's return (`35,2726`), basic:** EXTEND, DCA ATIGINC, TC
  P35/P75A. P35/P75A does DXCH KT, then TC P20FLGON.
- **P75 (`35,2731`), basic:** TC AVFLAGP.
- **AVFLAGP's return (`35,2732`), basic:** EXTEND, DCA PTIGINC, and on into
  P35/P75A.
- **P20FLGON's return (`35,2736`), basic:** TC INTPRET.
- **INTPRET's first pair (`35,2737`):** CALL SELECTMU.
- **P35/P75B (`35,2741`):** RTB LOADTIME.
- **LOADTIME's return (`35,2743`):** STORE TSTRT, DAD KT, STORE TIG, STORE
  INTIME, then STCALL TDEC1 PRECSET.
- **PRECSET's, S34/35.1's, S34/35.2's and S34/35.5's returns (`35,2752`,
  `2754`, `2756`, `2760`):** CALL S34/35.1, S34/35.2, S34/35.5 and VN1645 in
  turn.
- **VN1645's return (`35,2762`):** GOTO P35/P75B.

Each run ends at the first point outside these routines (`tests/lm_p35_cases.c`,
`lm_p35_stop`). That is one of:

- AVFLAGA, AVFLAGP or P20FLGON (P32-P35_P72-P75, bank 35);
- INTPRET;
- LOADTIME (`10,3573`, entered by RTB);
- a NEWOPS outside `35,2737`-`2763`: SELECTMU, PRECSET, or the S34/35.1,
  S34/35.2, S34/35.5 and VN1645 entries, which other milestones register.

Every interpretive pair is compiled with the shared bodies
(`src/lm_interp_steps.c`, `src/lm_interp_orders.c`). Each basic instruction
is one `ai_*` register effect on the image, in source order. DCA and DXCH
assemble K+1, so the native ATIGINC, PTIGINC and KT are the symbols
themselves (`E7,1400`, `E7,1402`, `E7,1575`).

Interpretive routines admit a canonical NEWOPS image at their LOC in FB 35
meeting all of these conditions, and otherwise return `EXEC_OUT_OF_SCOPE`
with the image unchanged:

- EB and BB coherent, with BANKSET = BB;
- INTBIT15 set;
- EDOP, ZERO and NEWJOB zero;
- MODE `+0`, `+1` or vector;
- the VAC one of the five, with PUSHLOC no further than 40D into it;
- IRQ 0 or 1, and an allowed SUPERBNK;
- every word 15 bits.

Basic routines admit Z at their entry in FB 35, with EB and BB coherent,
ZERO zero, IRQ 0 or 1, an allowed SUPERBNK and every word 15 bits.

## Source map

`tools/lm_p35_source.py` checks this map
(`evidence/lm-p35/lm-p35-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, or a word of the basic routine whose ENTRY names that address. The
erasable words and destinations the basic code names (ATIGINC, PTIGINC, KT,
AVFLAGA, AVFLAGP, P20FLGON, INTPRET) must equal their native macros.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P34-35_P74-75.agc:435 | 35,2725 | `P35 TC AVFLAGA` | 02347 | `p35` |
| P34-35_P74-75.agc:436 | 35,2726 | `EXTEND` | 00006 | `p35_avflaga` |
| P34-35_P74-75.agc:437 | 35,2727 | `DCA ATIGINC` | 31401 | `p35_avflaga` |
| P34-35_P74-75.agc:438 | 35,2730 | `TC P35/P75A` | 02734 | `p35_avflaga` |
| P34-35_P74-75.agc:439 | 35,2731 | `P75 TC AVFLAGP` | 02354 | `p75` |
| P34-35_P74-75.agc:440 | 35,2732 | `EXTEND` | 00006 | `p75_avflagp` |
| P34-35_P74-75.agc:441 | 35,2733 | `DCA PTIGINC` | 31403 | `p75_avflagp` |
| P34-35_P74-75.agc:442 | 35,2734 | `P35/P75A DXCH KT` | 53576 | `p75_avflagp` |
| P34-35_P74-75.agc:443 | 35,2735 | `TC P20FLGON` | 02361 | `p75_avflagp` |
| P34-35_P74-75.agc:444 | 35,2736 | `TC INTPRET` | 06037 | `p20_intpret` |
| P34-35_P74-75.agc:445 | 35,2737 | `CALL` | 77624 | `sel_mu` |
| P34-35_P74-75.agc:446 | 35,2740 | `SELECTMU` | 20000 | `sel_mu` |
| P34-35_P74-75.agc:447 | 35,2741 | `P35/P75B RTB` | 77634 | `r_p35b` |
| P34-35_P74-75.agc:448 | 35,2742 | `LOADTIME` | 21573 | `r_p35b` |
| P34-35_P74-75.agc:449 | 35,2743 | `STORE TSTRT` | 03612 | `loadtime_ret` |
| P34-35_P74-75.agc:450 | 35,2744 | `DAD` | 77615 | `loadtime_ret` |
| P34-35_P74-75.agc:451 | 35,2745 | `KT` | 03576 | `loadtime_ret` |
| P34-35_P74-75.agc:452 | 35,2746 | `STORE TIG` | 03442 | `loadtime_ret` |
| P34-35_P74-75.agc:453 | 35,2747 | `STORE INTIME` | 03610 | `loadtime_ret` |
| P34-35_P74-75.agc:454 | 35,2750 | `STCALL TDEC1` | 34041 | `loadtime_ret` |
| P34-35_P74-75.agc:455 | 35,2751 | `PRECSET` | 46360 | `loadtime_ret` |
| P34-35_P74-75.agc:456 | 35,2752 | `CALL` | 77624 | `call_s1` |
| P34-35_P74-75.agc:457 | 35,2753 | `S34/35.1` | 73274 | `call_s1` |
| P34-35_P74-75.agc:458 | 35,2754 | `CALL` | 77624 | `call_s2` |
| P34-35_P74-75.agc:459 | 35,2755 | `S34/35.2` | 73306 | `call_s2` |
| P34-35_P74-75.agc:460 | 35,2756 | `CALL` | 77624 | `call_s5` |
| P34-35_P74-75.agc:461 | 35,2757 | `S34/35.5` | 73522 | `call_s5` |
| P34-35_P74-75.agc:462 | 35,2760 | `CALL` | 77624 | `call_vn1645` |
| P34-35_P74-75.agc:463 | 35,2761 | `VN1645` | 73606 | `call_vn1645` |
| P34-35_P74-75.agc:464 | 35,2762 | `GOTO` | 77650 | `loop` |
| P34-35_P74-75.agc:465 | 35,2763 | `P35/P75B` | 72741 | `loop` |

## Evidence

Hashes are in `evidence/lm-p35/SHA256.json`. Bulk files are assets of the
`evidence-lm-p35` release (`python3 tools/evidence.py fetch lm-p35`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_p35_source.py` reassembles the unchanged file with the
  pinned yaYUL. The rope equals the certified fixed-data rope.
- **Oracle.** `tools/lm_p35_reference.c` (GPL, host only) runs the unchanged
  Luminary099 in the pinned yaAGC hardware-DV variant from 13312 entries,
  1024 per routine, to the same stops.
  - The entries sit over poisoned debris with any times and flags.
  - EB is KT's bank three times in four.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/lm_p35_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`lm-p35-source-stream.json`, SHA256
  `418af5cd...5d55afcf`) and archives the stream
  (`lm-p35-reference.snapshots.bin.xz`). `lm-p35-proof.json` lists the stops
  per routine.
- **Host and sanitizer results.** Run `make test-lm-p35 sanitize-lm-p35`. The
  strict build and the ASAN/UBSAN build each pass 13488 checks:
  - all 13312 frozen digests;
  - every run ending at one of the stops;
  - 147 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, and VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones.

  See `lm-p35-checks.log`.
- **C64 execution results.** Run `make vice-lm-p35`. One PRG registers every
  routine. It runs 520 cases spread over the variants, cycling through all
  thirteen entries, with the image in the RAM under KERNAL ROM (`$01=$35`).
  The case generator is integer-only, so the C64 builds the same entry
  images as the host. In VICE (PAL, stock ROMs) the PRG passes 520 checks with no
  failures (`lm-p35-results.json`).

## Limits

- P34/P74, S33/34.1, ELCALC and S34/35.2's ACOS and Lambert setup are not
  covered. The file stays `partial`.
- AVFLAGA, AVFLAGP and P20FLGON belong to P32-P35_P72-P75 and are stops
  here.
- Numerical validity is the original's. No wider claim is made.
