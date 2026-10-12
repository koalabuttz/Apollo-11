# P30: external delta V, and S30.1

Lane E. Milestone `lm-p30`.

## Scope

All of `Luminary099/P30_P37.agc`: P30 at bank 35 `02000`-`02026` and S30.1
at bank 34 `02000`-`02056`, 70 words on 70 source lines. Eight basic
routines are keyed by Z in bank 35. Ten interpretive routines are keyed by
LOC under BANKSET at NEWOPS, seven in bank 34 and three in bank 35. All use
the control-transfer ABI (`include/agc_transfer.h`,
`docs/NATIVE_TRANSFER_ABI.md`). `include/lm_p30_routines.h` lists each
entry.

- **P30 (basic):** sets UPDATFLG and TRACKFLG through UPFLAG. P30N33 shows
  V06N33 (TIG) and then V06N81 (DELVSLV) through VNPOOH. It then clears
  UPDATFLG through DOWNFLAG and enters INTPRET.
- **P30 (interpretive):** CALL S30.1. Where it returns, SET UPDATFLG and
  EXIT.
- **PARAM30 (basic):** V06N42 (HAPO, HPER, DELVSAB) through VNPOOH, then
  INTPRET.
- **PARAM30 (interpretive):** SETGO XDELVFLG into REVN1645.
- **S30.1 (interpretive):** STQ QTEMP, then TIG into TDEC1 and STCALL
  LEMPREC. Where LEMPREC returns:
  - RTIG and VTIG from RATT and VATT, with X2 from RTX2 and X1 from RTX1;
  - the local-vertical frame pushed at 0D (UNIT, VCOMP, VXV);
  - DELVSLV through it (VXM, VSL1) into DELVSIN, and |DELVSIN| into
    DELVSAB;
  - RTIG pushed and VTIG + DELVSIN loaded, then CALL PERIAPO1.

  The returns call SHIFTR1 and MAXCHK twice, storing HPER and then HAPO,
  and STCALL HAPO goes to the CADR in QTEMP.

Each run ends at the first point outside these routines
(`tests/lm_p30_cases.c`, `lm_p30_stop`). That is one of:

- UPFLAG, DOWNFLAG, VNPOOH (lm-p34) or INTPRET;
- POODOO1 (alarm 1302) from UNIT or ABVAL;
- a NEWOPS outside `34,2000`-`2056`, `35,2013`-`2016` and `35,2022`-`2024`:
  LEMPREC, PERIAPO1, SHIFTR1, MAXCHK, REVN1645, or the CADR in QTEMP.

Every interpretive pair is compiled with the shared bodies
(`src/lm_interp_steps.c`, `src/lm_interp_orders.c`). VXM runs the shared
work-matrix body (`src/lm_work_matrix.c`) on the frame at 0D. V06N33, V06N81
and V06N42 are literals that the source map checks against the fresh rope.
Each basic instruction is one `ai_*` register effect on the image, in
source order.

Interpretive routines admit a canonical NEWOPS image at their LOC in their
own FB (34 or 35) meeting all of these conditions, and otherwise return
`EXEC_OUT_OF_SCOPE` with the image unchanged:

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

`tools/lm_p30_source.py` checks this map
(`evidence/lm-p30/lm-p30-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, or a word of the basic routine whose ENTRY names that address.
V06N33 and V06N42 belong to the routines whose CAF reads them. The
destinations the basic code names, and the three fixed constants, must
equal their native values in the fresh assembly.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P30_P37.agc:61 | 35,2000 | `P30 TC UPFLAG` | 05504 | `p30` |
| P30_P37.agc:62 | 35,2001 | `ADRES UPDATFLG` | 00027 | `p30` |
| P30_P37.agc:63 | 35,2002 | `TC UPFLAG` | 05504 | `updatflg_ret` |
| P30_P37.agc:64 | 35,2003 | `ADRES TRACKFLG` | 00031 | `updatflg_ret` |
| P30_P37.agc:66 | 35,2004 | `P30N33 CAF V06N33` | 32025 | `p30n33` |
| P30_P37.agc:67 | 35,2005 | `TC VNPOOH` | 03712 | `p30n33` |
| P30_P37.agc:69 | 35,2006 | `CAF V06N81` | 33727 | `v06n81` |
| P30_P37.agc:70 | 35,2007 | `TC VNPOOH` | 03712 | `v06n81` |
| P30_P37.agc:72 | 35,2010 | `TC DOWNFLAG` | 05516 | `downflag` |
| P30_P37.agc:73 | 35,2011 | `ADRES UPDATFLG` | 00027 | `downflag` |
| P30_P37.agc:74 | 35,2012 | `TC INTPRET` | 06037 | `downflag_ret` |
| P30_P37.agc:75 | 35,2013 | `CALL` | 77624 | `call_s301` |
| P30_P37.agc:76 | 35,2014 | `S30.1` | 70000 | `call_s301` |
| P30_P37.agc:77 | 35,2015 | `SET EXIT` | 77414 | `s301_ret` |
| P30_P37.agc:78 | 35,2016 | `UPDATFLG` | 00470 | `s301_ret` |
| P30_P37.agc:79 | 35,2017 | `PARAM30 CAF V06N42` | 32026 | `param30` |
| P30_P37.agc:80 | 35,2020 | `TC VNPOOH` | 03712 | `param30` |
| P30_P37.agc:83 | 35,2021 | `TC INTPRET` | 06037 | `param30_ret` |
| P30_P37.agc:84 | 35,2022 | `SETGO` | 77614 | `setgo` |
| P30_P37.agc:85 | 35,2023 | `XDELVFLG` | 01027 | `setgo` |
| P30_P37.agc:86 | 35,2024 | `REVN1645` | 72540 | `setgo` |
| P30_P37.agc:88 | 35,2025 | `V06N33 VN 0633` | 01441 | `p30n33` |
| P30_P37.agc:89 | 35,2026 | `V06N42 VN 0642` | 01452 | `param30` |
| P30_P37.agc:136 | 34,2000 | `S30.1 STQ DLOAD` | 71220 | `s301` |
| P30_P37.agc:137 | 34,2001 | `QTEMP` | 03632 | `s301` |
| P30_P37.agc:138 | 34,2002 | `TIG` | 03442 | `s301` |
| P30_P37.agc:139 | 34,2003 | `STCALL TDEC1` | 34041 | `s301` |
| P30_P37.agc:140 | 34,2004 | `LEMPREC` | 27057 | `s301` |
| P30_P37.agc:142 | 34,2005 | `VLOAD SXA,2` | 67175 | `lemprec_ret` |
| P30_P37.agc:144 | 34,2006 | `RATT` | 00001 | `lemprec_ret` |
| P30_P37.agc:145 | 34,2007 | `RTX2` | 02777 | `lemprec_ret` |
| P30_P37.agc:146 | 34,2010 | `STORE RTIG` | 03642 | `lemprec_ret` |
| P30_P37.agc:147 | 34,2011 | `UNIT VCOMP` | 57456 | `lemprec_ret` |
| P30_P37.agc:148 | 34,2012 | `STOVL DELVSIN` | 27656 | `lemprec_ret` |
| P30_P37.agc:149 | 34,2013 | `VATT` | 00007 | `lemprec_ret` |
| P30_P37.agc:150 | 34,2014 | `STORE VTIG` | 03650 | `lemprec_ret` |
| P30_P37.agc:151 | 34,2015 | `VXV UNIT` | 53435 | `lemprec_ret` |
| P30_P37.agc:152 | 34,2016 | `RTIG` | 03642 | `lemprec_ret` |
| P30_P37.agc:153 | 34,2017 | `SETPD SXA,1` | 66001 | `lemprec_ret` |
| P30_P37.agc:154 | 34,2020 | `0` | 00001 | `lemprec_ret` |
| P30_P37.agc:155 | 34,2021 | `RTX1` | 02776 | `lemprec_ret` |
| P30_P37.agc:156 | 34,2022 | `PUSH VXV` | 47206 | `lemprec_ret` |
| P30_P37.agc:157 | 34,2023 | `DELVSIN` | 03656 | `lemprec_ret` |
| P30_P37.agc:158 | 34,2024 | `VSL1 PDVL` | 63372 | `lemprec_ret` |
| P30_P37.agc:159 | 34,2025 | `PDVL PDVL` | 63315 | `lemprec_ret` |
| P30_P37.agc:160 | 34,2026 | `DELVSIN` | 03656 | `lemprec_ret` |
| P30_P37.agc:161 | 34,2027 | `DELVSLV` | 03434 | `lemprec_ret` |
| P30_P37.agc:162 | 34,2030 | `VXM VSL1` | 76505 | `lemprec_ret` |
| P30_P37.agc:163 | 34,2031 | `0` | 00001 | `lemprec_ret` |
| P30_P37.agc:164 | 34,2032 | `STORE DELVSIN` | 03656 | `lemprec_ret` |
| P30_P37.agc:165 | 34,2033 | `ABVAL` | 77646 | `lemprec_ret` |
| P30_P37.agc:166 | 34,2034 | `STOVL DELVSAB` | 27664 | `lemprec_ret` |
| P30_P37.agc:167 | 34,2035 | `RTIG` | 03642 | `lemprec_ret` |
| P30_P37.agc:168 | 34,2036 | `PDVL VAD` | 53315 | `lemprec_ret` |
| P30_P37.agc:169 | 34,2037 | `VTIG` | 03650 | `lemprec_ret` |
| P30_P37.agc:170 | 34,2040 | `DELVSIN` | 03656 | `lemprec_ret` |
| P30_P37.agc:171 | 34,2041 | `CALL` | 77624 | `lemprec_ret` |
| P30_P37.agc:172 | 34,2042 | `PERIAPO1` | 46316 | `lemprec_ret` |
| P30_P37.agc:173 | 34,2043 | `CALL` | 77624 | `periapo_ret` |
| P30_P37.agc:174 | 34,2044 | `SHIFTR1` | 46426 | `periapo_ret` |
| P30_P37.agc:175 | 34,2045 | `CALL` | 77624 | `shiftr1_ret` |
| P30_P37.agc:176 | 34,2046 | `MAXCHK` | 45636 | `shiftr1_ret` |
| P30_P37.agc:177 | 34,2047 | `STODL HPER` | 16321 | `maxchk_ret` |
| P30_P37.agc:178 | 34,2050 | `4D` | 00005 | `maxchk_ret` |
| P30_P37.agc:179 | 34,2051 | `CALL` | 77624 | `maxchk_ret` |
| P30_P37.agc:180 | 34,2052 | `SHIFTR1` | 46426 | `maxchk_ret` |
| P30_P37.agc:181 | 34,2053 | `CALL` | 77624 | `shiftr1_ret2` |
| P30_P37.agc:182 | 34,2054 | `MAXCHK` | 45636 | `shiftr1_ret2` |
| P30_P37.agc:183 | 34,2055 | `STCALL HAPO` | 36317 | `maxchk_ret2` |
| P30_P37.agc:184 | 34,2056 | `QTEMP` | 03632 | `maxchk_ret2` |

## Evidence

Hashes are in `evidence/lm-p30/SHA256.json`. Bulk files are assets of the
`evidence-lm-p30` release (`python3 tools/evidence.py fetch lm-p30`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_p30_source.py` reassembles the unchanged file with the
  pinned yaYUL. The rope equals the certified fixed-data rope.
- **Oracle.** `tools/lm_p30_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant from 9216
  entries, 512 per routine, to the same stops.
  - The entries sit over poisoned debris with any vectors, times and flags.
  - EB is E7 three times in four.
  - QPRET and QTEMP are P30's real return from S30.1 (35,2015) three times
    in four, else a CADR in banks 2-27.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/lm_p30_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`lm-p30-source-stream.json`, SHA256
  `219b7688...ba59f817`) and archives the stream
  (`lm-p30-reference.snapshots.bin.xz`). `lm-p30-proof.json` lists the stops
  per routine.
- **Host and sanitizer results.** Run `make test-lm-p30 sanitize-lm-p30`.
  The strict build and the ASAN/UBSAN build each pass 9451 checks:
  - all 9216 frozen digests;
  - every run ending at one of the stops;
  - 196 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, and VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones.

  See `lm-p30-checks.log`.
- **C64 execution results.** Run `make vice-lm-p30`. Two PRGs each register
  only the routines their cases reach:
  - group 1: S30.1's LEMPREC return (UNIT, VXV, VXM, ABVAL) (128 cases);
  - group 2: every other entry (510 cases).

  Both are built with `-fno-inline-functions`, so group 1 fits under the
  mailbox page. Each keeps the image in the RAM under KERNAL ROM
  (`$01=$35`). The case generator is integer-only, so the C64 builds the
  same entry images as the host. In VICE (PAL, stock ROMs) the two PRGs pass 128 and 510 checks
  with no failures (`lm-p30-results.json`).

## Limits

- LEMPREC, PERIAPO1, SHIFTR1, MAXCHK, REVN1645, UPFLAG, DOWNFLAG and VNPOOH
  belong to other files or milestones and are stops here.
- Numerical validity is the original's. No wider claim is made.
