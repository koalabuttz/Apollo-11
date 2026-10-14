# CM P21: ground track determination

Lane E. Milestone `cm-p21`.

## Scope

All of `Comanche055/GROUND_TRACKING_DETERMINATION_PROGRAM.agc` (P21): bank
37 `02001`-`02154`, 108 words on 106 source lines. Fourteen basic routines
are keyed by Z. Ten interpretive routines are keyed by LOC under BANKSET at
NEWOPS. All use the control-transfer ABI (`include/agc_transfer.h`,
`docs/NATIVE_TRANSFER_ABI.md`). `include/cm_p21_routines.h` lists each
entry.

- **Entry and displays (basic):** PROG21 sets OPTION2 to 1 and requests
  GOPERF4 (BIT2); P21PROG1 flashes V06N34 for the time. Terminate goes to
  GOTOPOOH, proceed onward, recycle back to the request. The second
  proceed enters INTPRET.
- **Integration (interpretive):** DSPTEM1 into TDEC1 and CALL INTSTALL. On
  return, BON P21FLAG goes to P21CONT; otherwise VINTFLAG is set, and
  cleared again unless OPTION2 SR1 is zero (BHIZ). DIM0FLAG and INTYPFLG
  are cleared and INTEGRV is called; on its return GOTO P21VSAVE.
- **Recycle (interpretive):** P21CONT loads RCV, VCV and TET from the base
  vector, clears DIM0FLAG and MOONFLAG, sets MOONFLAG unless P21ORIG is
  zero (BZE), and calls INTEGRVS.
- **Base vector (interpretive):** P21VSAVE stores TAT, RATT1 and VATT1 as
  the base vector, |V| scaled by X2 (ABVAL SL* 0,2) as P21VEL, then forms
  U(R).V / P21VEL (UNIT, DOT, DDV) and dispatches ASIN.
- **Display (interpretive):** after ASIN, P21GAM is stored, X2 saved to
  P21ORIG (SXA,2) and P21FLAG set. P21DSP sets LUNAFLAG unless X2 is zero,
  loads RATT into ALPHAV and TAT, clears ERADFLAG and calls LAT-LONG. On
  return the altitude is scaled by K.01 into P21ALT, then EXIT.
- **Display and recycle (basic, then interpretive):** V06N43 is flashed;
  its terminate and proceed go to GOTOPOOH and recycle enters INTPRET,
  which adds 600SEC to P21TIME into DSPTEM1 and RTBs to P21PROG1.

P21ONENN (`37,2147`) is read by TPI_SEARCH. It sits in this file's range,
and the source map checks its words against native literals.

Each run ends at the first point outside these routines
(`tests/cm_p21_cases.c`, `cm_p21_stop`). That is one of:

- BANKCALL, GOTOPOOH or INTPRET;
- ARCSIN (`00,3607`), the ASIN body, which is outside this file;
- POODOO (alarm 1302);
- a NEWOPS outside bank 37's `2020`-`2130` and `2137`-`2144`: INTSTALL,
  INTEGRV, INTEGRVS or LAT-LONG.

Every interpretive pair is compiled with the shared bodies built for
Comanche055 (`src/cm_interp_steps.c`, `src/cm_interp_orders.c`). ONE,
BIT2, V06N43, V6N34, K.01, 600SEC and P21ONENN are literals that the source
map checks against the fresh rope. Each basic instruction is one `ai_*`
register effect on the image, in source order.

Interpretive routines admit a canonical NEWOPS image (Comanche055 NEWOPS,
`06037`) at their LOC in FB 37 meeting all of these conditions, and
otherwise return `EXEC_OUT_OF_SCOPE` with the image unchanged:

- EB and BB coherent, with BANKSET = BB;
- INTBIT15 set (bank 37 is at or above bank 20);
- EDOP, ZERO and NEWJOB zero;
- MODE `+0`, `+1` or vector;
- the VAC one of the five, with PUSHLOC no further than 32D into it;
- IRQ 0 or 1, and an allowed SUPERBNK;
- every word 15 bits.

Basic routines admit Z at their entry in FB 37, with EB and BB coherent,
ZERO zero, IRQ 0 or 1, an allowed SUPERBNK and every word 15 bits.

## Source map

`tools/cm_p21_source.py` checks this map
(`evidence/cm-p21/cm-p21-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, a word of the basic routine whose ENTRY names that address, or a
data word equal to its native literal. The erasable words and destinations
the code names, and the fixed constants, must equal their native values in
the fresh assembly.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:96 | 37,2001 | `PROG21 CAF ONE` | 35017 | `prog21` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:97 | 37,2002 | `TS OPTION2` | 55132 | `prog21` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:98 | 37,2003 | `CAF BIT2` | 35016 | `r_perf4` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:99 | 37,2004 | `TC BANKCALL` | 04662 | `r_perf4` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:100 | 37,2005 | `CADR GOPERF4` | 21055 | `r_perf4` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:101 | 37,2006 | `TC GOTOPOOH` | 04106 | `perf4_terminate` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:102 | 37,2007 | `TC +2` | 02011 | `perf4_proceed` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:103 | 37,2010 | `TC -5` | 02003 | `perf4_enter` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:104 | 37,2011 | `P21PROG1 CAF V6N34` | 32152 | `p21prog1` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:105 | 37,2012 | `TC BANKCALL` | 04662 | `p21prog1` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:106 | 37,2013 | `CADR GOFLASH` | 20720 | `p21prog1` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:107 | 37,2014 | `TC GOTOPOOH` | 04106 | `f34_terminate` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:108 | 37,2015 | `TC +2` | 02017 | `f34_proceed` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:109 | 37,2016 | `TC -5` | 02011 | `f34_recycle` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:110 | 37,2017 | `TC INTPRET` | 06006 | `r_intpret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:111 | 37,2020 | `DLOAD` | 77745 | `p21_tdec1` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:112 | 37,2021 | `DSPTEM1` | 01046 | `p21_tdec1` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:113 | 37,2022 | `STCALL TDEC1` | 34041 | `p21_tdec1` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:114 | 37,2023 | `INTSTALL` | 27442 | `p21_tdec1` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:115 | 37,2024 | `BON SET` | 43014 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:116 | 37,2025 | `P21FLAG` | 01303 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:117 | 37,2026 | `P21CONT` | 76044 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:118 | 37,2027 | `VINTFLAG` | 01474 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:119 | 37,2030 | `SLOAD SR1` | 70535 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:120 | 37,2031 | `OPTION2` | 01133 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:121 | 37,2032 | `BHIZ CLEAR` | 43030 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:122 | 37,2033 | `+2` | 76035 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:123 | 37,2034 | `VINTFLAG` | 01674 | `p21_flags` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:124 | 37,2035 | `CLEAR CLEAR` | 43014 | `p21_integrv` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:125 | 37,2036 | `DIM0FLAG` | 01676 | `p21_integrv` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:126 | 37,2037 | `INTYPFLG` | 01673 | `p21_integrv` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:127 | 37,2040 | `CALL` | 77624 | `p21_integrv` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:128 | 37,2041 | `INTEGRV` | 27162 | `p21_integrv` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:129 | 37,2042 | `GOTO` | 77650 | `integrv_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:130 | 37,2043 | `P21VSAVE` | 76065 | `integrv_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:131 | 37,2044 | `P21CONT VLOAD` | 77775 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:132 | 37,2045 | `P21BASER` | 03605 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:134 | 37,2046 | `STOVL RCV` | 25535 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:135 | 37,2047 | `P21BASEV` | 03613 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:136 | 37,2050 | `STODL VCV` | 15543 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:137 | 37,2051 | `P21TIME` | 02316 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:138 | 37,2052 | `STORE TET` | 01517 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:139 | 37,2053 | `CLEAR CLEAR` | 43014 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:140 | 37,2054 | `DIM0FLAG` | 01676 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:141 | 37,2055 | `MOONFLAG` | 00263 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:142 | 37,2056 | `SLOAD BZE` | 53135 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:143 | 37,2057 | `P21ORIG` | 03604 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:144 | 37,2060 | `+3` | 76063 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:145 | 37,2061 | `SET` | 77614 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:146 | 37,2062 | `MOONFLAG` | 00063 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:147 | 37,2063 | `CALL` | 77624 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:148 | 37,2064 | `INTEGRVS` | 27135 | `p21cont` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:149 | 37,2065 | `P21VSAVE DLOAD` | 77745 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:150 | 37,2066 | `TAT` | 00015 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:151 | 37,2067 | `STOVL P21TIME` | 26316 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:152 | 37,2070 | `RATT1` | 00017 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:153 | 37,2071 | `STOVL P21BASER` | 27605 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:154 | 37,2072 | `VATT1` | 00025 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:155 | 37,2073 | `STORE P21BASEV` | 03613 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:156 | 37,2074 | `ABVAL SL*` | 53646 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:157 | 37,2075 | `0,2` | 57576 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:158 | 37,2076 | `STOVL P21VEL` | 27623 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:159 | 37,2077 | `RATT` | 00001 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:160 | 37,2100 | `UNIT DOT` | 50256 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:161 | 37,2101 | `VATT` | 00007 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:162 | 37,2102 | `DDV ASIN` | 67471 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:163 | 37,2103 | `P21VEL` | 03623 | `p21vsave` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:164 | 37,2104 | `STORE P21GAM` | 03625 | `arcsin_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:165 | 37,2105 | `SXA,2 SET` | 43134 | `arcsin_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:166 | 37,2106 | `P21ORIG` | 03603 | `arcsin_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:167 | 37,2107 | `P21FLAG` | 01063 | `arcsin_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:168 | 37,2110 | `P21DSP CLEAR SLOAD` | 67214 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:169 | 37,2111 | `LUNAFLAG` | 01663 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:170 | 37,2112 | `X2` | 00050 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:171 | 37,2113 | `BZE SET` | 43054 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:172 | 37,2114 | `+2` | 76116 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:173 | 37,2115 | `LUNAFLAG` | 01463 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:174 | 37,2116 | `VLOAD` | 77775 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:175 | 37,2117 | `RATT` | 00001 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:176 | 37,2120 | `STODL ALPHAV` | 16152 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:177 | 37,2121 | `TAT` | 00015 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:178 | 37,2122 | `CLEAR CALL` | 45014 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:179 | 37,2123 | `ERADFLAG` | 00662 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:180 | 37,2124 | `LAT-LONG` | 26370 | `p21dsp` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:181 | 37,2125 | `DMP` | 77605 | `latlong_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:182 | 37,2126 | `K.01` | 36154 | `latlong_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:183 | 37,2127 | `STORE P21ALT` | 03621 | `latlong_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:185 | 37,2130 | `EXIT` | 77776 | `latlong_ret` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:186 | 37,2131 | `CAF V06N43` | 32151 | `v06n43` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:187 | 37,2132 | `TC BANKCALL` | 04662 | `v06n43` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:188 | 37,2133 | `CADR GOFLASH` | 20720 | `v06n43` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:189 | 37,2134 | `TC GOTOPOOH` | 04106 | `f43_terminate` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:190 | 37,2135 | `TC GOTOPOOH` | 04106 | `f43_proceed` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:191 | 37,2136 | `TC INTPRET` | 06006 | `f43_recycle` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:192 | 37,2137 | `DLOAD DAD` | 43345 | `p21_next` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:193 | 37,2140 | `P21TIME` | 02316 | `p21_next` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:194 | 37,2141 | `600SEC` | 36146 | `p21_next` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:195 | 37,2142 | `STORE DSPTEM1` | 01046 | `p21_next` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:196 | 37,2143 | `RTB` | 77634 | `p21_next` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:197 | 37,2144 | `P21PROG1` | 76011 | `p21_next` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:198 | 37,2145 | `600SEC 2DEC 60000` | 00003 25140 | `K_600SEC_HI K_600SEC_LO` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:200 | 37,2147 | `P21ONENN OCT 00001` | 00001 | `K_P21ONENN_HI` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:201 | 37,2150 | `OCT 00000` | 00000 | `K_P21ONENN_LO` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:202 | 37,2151 | `V06N43 VN 00643` | 01453 | `K_V06N43` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:203 | 37,2152 | `V6N34 VN 00634` | 01442 | `K_V6N34` |
| GROUND_TRACKING_DETERMINATION_PROGRAM.agc:204 | 37,2153 | `K.01 2DEC .01` | 00243 32703 | `K_K01_HI K_K01_LO` |

## Evidence

Hashes are in `evidence/cm-p21/SHA256.json`. Bulk files are assets of the
`evidence-cm-p21` release (`python3 tools/evidence.py fetch cm-p21`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/cm_p21_source.py` reassembles the unchanged Comanche055
  with the pinned yaYUL. The rope equals the certified fixed-data rope.
- **Oracle.** `tools/cm_p21_reference.c` (GPL, host only) runs the
  unchanged Comanche055 (`deps/cm-rtb-fixture/MAIN.agc.bin`, equal to the
  certified rope) in the pinned yaAGC hardware-DV variant from 12288
  entries, 512 per routine, to the same stops.
  - The entries sit over poisoned debris with any vectors, times and flags.
  - OPTION2, P21ORIG and X2 are each zero in one case in two, so both
    outcomes of BHIZ, BZE and the LUNAFLAG and MOONFLAG choices run.
  - EB is E4 three times in four.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/cm_p21_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`cm-p21-source-stream.json`, SHA256
  `fde47bff...2cb2a524`) and archives the stream
  (`cm-p21-reference.snapshots.bin.xz`). `cm-p21-proof.json` lists the stops
  per routine. Changing 600SEC, inverting the BZE, storing P21ALT one word
  off or calling the wrong INTEGRV address each fail it.
- **Host and sanitizer results.** Run `make test-cm-p21 sanitize-cm-p21`.
  The strict build and the ASAN/UBSAN build each pass 12577 checks:
  - all 12288 frozen digests;
  - every run ending at one of the stops;
  - 238 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, and VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones.

  See `cm-p21-checks.log`.
- **C64 execution results.** Run `make cm-p21-vice`
  (vice-cm-p21-1 and -2 on CI, one per group). Two PRGs each register
  only the routines their cases reach:
  - group 1: TDEC1 through P21VSAVE (INTEGRV, DDV, ASIN) (516 cases);
  - group 2: every other entry (518 cases).

  Each keeps the image in the RAM under KERNAL ROM (`$01=$35`). The case
  generator is integer-only, so the C64 builds the same entry images as the
  host. In VICE (PAL, stock ROMs) the two PRGs pass 516 and 518 checks
  with no failures (`cm-p21-results.json`).

## Limits

- ARCSIN (ASIN), INTSTALL, INTEGRV, INTEGRVS, LAT-LONG, GOPERF4 and GOFLASH
  belong to other files and are stops here.
- Numerical validity is the original's. No wider claim is made.
