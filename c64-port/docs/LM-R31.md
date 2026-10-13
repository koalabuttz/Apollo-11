# LM R31: rendezvous range, rate and angle display

Lane E. Milestone `lm-r31`.

## Scope

All of `Luminary099/R31.agc`: bank 40 `03674`-`03712` and bank 37
`03113`-`03407`, 204 words on 202 source lines. Eighteen basic routines are
keyed by Z (bank 40 is FB 30 with SUPERBNK 4). Twenty-four interpretive
routines are keyed by LOC under BANKSET at NEWOPS. All use the
control-transfer ABI (`include/agc_transfer.h`,
`docs/NATIVE_TRANSFER_ABI.md`). `include/lm_r31_routines.h` lists each
entry.

- **V83 task (basic, bank 40):** R31CALL asks FINDVAC for V83CALL.
  DSPDELAY waits on FIXDELAY until EXTVBACT BIT12 is set, then NOVAC
  starts DISPN5X and the task ends at TASKOVER.
- **Display (basic):** DISPN5X flashes V16N54 through GOMARKF; its returns
  go to B5OFF twice and back to DISPN5X.
- **Base vectors (basic, then interpretive):** V83CALL and REV83 branch on
  AVEGFLAG (to MUNG? or GETRVN) and SURFFLAG (to ONEBASE or R31SURF). The
  interpretive code integrates the LM (LEMPREC) and CSM (CSMPREC) to
  LOADTIME and saves both base states. REV83 integrates each base state to
  LOADTIME with INTSTALL and INTEGRVS, picking MOONFLAG by MOONTHIS.
- **COMPDISP (interpretive):** RANGE is |RATT - RONE| (RTB NORMUNX1, SL*
  0,1), RRATE is (VATT - VONE).U, and the line of sight goes through
  CDU*NBSM and REFSMMAT (VXM, VPROJ, BVSU, UNIT, VXV, DOT, SIGN) into SL2
  ACOS. RTHETA is that angle, or DPPOSMAX minus it when the BPL is not
  taken. EXIT ends the pass.
- **Recycle (basic):** ENDEXT unless EXTVBACT BIT5 is set; otherwise BIT12
  is set (ADS) and REV83 runs again.
- **GETRVN (basic, then interpretive):** under PRIO22, RN, VN, V(CSM),
  R(CSM) and PIPTIME are loaded. Back at PRIO3, BOFF MUNFLAG goes to
  GETRVN2 (INTSTALL, then OTHINT); otherwise both vectors rotate through
  REFSMMAT (VXM VSR4, VXM VSL1) into COMPDISP.

RTB NORMUNX1 runs the shared NORM front and UNIT phases (`src/rtb_vector.c`,
`src/rtb_unit.c`). Every interpretive pair is compiled with the shared
bodies (`src/lm_interp_steps.c`, `src/lm_interp_orders.c`). UNITZ,
DPPOSMAX, V16N54 and the PRIO and BIT masks are literals that the source
map checks against the fresh rope. Each basic instruction is one `ai_*`
register effect on the image, in source order.

Each run ends at the first point outside these routines
(`tests/lm_r31_cases.c`, `lm_r31_stop`). That is one of:

- FINDVAC, FIXDELAY, NOVAC or TASKOVER from bank 40;
- BANKCALL, B5OFF, PRIOCHNG, ENDEXT or INTPRET from bank 37 (NORMUNX1's own
  BANKCALL runs in bank 10 and is not a stop);
- LOADTIME (`10,3573`) after an RTB;
- ARCCOS (`00,3612`), the ACOS body, which is outside this file;
- POODOO1 (alarm 1302), or UNIT's 1302 exit (`00,3402`) inside NORMUNX1;
- a NEWOPS outside bank 37's `3132`-`3154`, `3166`-`3315`, `3331`-`3343`,
  `3347`-`3367`, `3371`-`3376` and `3404`-`3406`: LEMPREC, CSMPREC,
  INTSTALL, INTEGRVS or CDU*NBSM;
- a NEWOPS at a split point: COMPDISP (`3240`), RANGE and RRATE (`3246`)
  or the second REFSMMAT rotation (`3355`). These points are entries of
  their own. A run that reaches one stops there, so that no C64 build has
  to hold both UNIT implementations or two VXMs.

Interpretive routines admit a canonical NEWOPS image (NEWOPS `06070`) at
their LOC in FB 37 meeting all of these conditions, and otherwise return
`EXEC_OUT_OF_SCOPE` with the image unchanged:

- EB and BB coherent, with BANKSET = BB;
- INTBIT15 set (bank 37 is at or above bank 20);
- EDOP, ZERO and NEWJOB zero;
- MODE `+0`, `+1` or vector;
- the VAC one of the five, with PUSHLOC no further than 32D into it;
- IRQ 0 or 1, and an allowed SUPERBNK below bank 40's;
- every word 15 bits.

Basic routines admit Z at their entry in FB 37 (SUPERBNK below bank 40's)
or in bank 40 (FB 30, SUPERBNK 4), with EB and BB coherent, ZERO zero, IRQ
0 or 1 and every word 15 bits.

## Source map

`tools/lm_r31_source.py` checks this map
(`evidence/lm-r31/lm-r31-source-map.json`). Every word is a literal of the
registered routine entered at the first address of its range, in source
order, a word of the basic routine whose ENTRY names that address, or a
data word equal to its native literal. The erasable words and destinations
the code names, and the fixed constants, must equal their native values in
the fresh assembly.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| R31.agc:36 | 40,3674 | `R31CALL CAF PRIO3` | 35015 | `r31call` |
| R31.agc:37 | 40,3675 | `TC FINDVAC` | 05105 | `r31call` |
| R31.agc:39 | 40,3676 | `2CADR V83CALL` | 03121 76067 | `r31call` |
| R31.agc:41 | 40,3700 | `DSPDELAY TC FIXDELAY` | 05221 | `dspdelay` |
| R31.agc:42 | 40,3701 | `DEC 100` | 00144 | `dspdelay` |
| R31.agc:43 | 40,3702 | `CA EXTVBACT` | 31044 | `fixdelay_ret` |
| R31.agc:44 | 40,3703 | `MASK BIT12` | 74740 | `fixdelay_ret` |
| R31.agc:45 | 40,3704 | `EXTEND` | 00006 | `fixdelay_ret` |
| R31.agc:46 | 40,3705 | `BZF DSPDELAY` | 13700 | `fixdelay_ret` |
| R31.agc:48 | 40,3706 | `CAF PRIO5` | 35017 | `fixdelay_ret` |
| R31.agc:49 | 40,3707 | `TC NOVAC` | 05072 | `fixdelay_ret` |
| R31.agc:51 | 40,3710 | `2CADR DISPN5X` | 03113 76067 | `fixdelay_ret` |
| R31.agc:53 | 40,3712 | `TCF TASKOVER` | 15261 | `r_taskover` |
| R31.agc:60 | 37,3113 | `DISPN5X CAF V16N54` | 33407 | `dispn5x` |
| R31.agc:61 | 37,3114 | `TC BANKCALL` | 04616 | `dispn5x` |
| R31.agc:62 | 37,3115 | `CADR GOMARKF` | 20334 | `dispn5x` |
| R31.agc:63 | 37,3116 | `TC B5OFF` | 05563 | `b5off_1` |
| R31.agc:64 | 37,3117 | `TC B5OFF` | 05563 | `b5off_2` |
| R31.agc:65 | 37,3120 | `TCF DISPN5X` | 13113 | `redisplay` |
| R31.agc:67 | 37,3121 | `V83CALL CS FLAGWRD7` | 40103 | `v83call` |
| R31.agc:68 | 37,3122 | `MASK AVEGFBIT` | 74747 | `v83call` |
| R31.agc:69 | 37,3123 | `EXTEND` | 00006 | `v83call` |
| R31.agc:70 | 37,3124 | `BZF MUNG?` | 13377 | `v83call` |
| R31.agc:72 | 37,3125 | `CS FLAGWRD8` | 40104 | `v83call` |
| R31.agc:73 | 37,3126 | `MASK SURFFBIT` | 74744 | `v83call` |
| R31.agc:74 | 37,3127 | `EXTEND` | 00006 | `v83call` |
| R31.agc:75 | 37,3130 | `BZF ONEBASE` | 13403 | `v83call` |
| R31.agc:77 | 37,3131 | `TC INTPRET` | 06037 | `v83call` |
| R31.agc:78 | 37,3132 | `RTB` | 77634 | `v83_rtb` |
| R31.agc:80 | 37,3133 | `LOADTIME` | 21573 | `v83_rtb` |
| R31.agc:81 | 37,3134 | `STCALL TDEC1` | 34041 | `v83_lemprec` |
| R31.agc:82 | 37,3135 | `LEMPREC` | 27057 | `v83_lemprec` |
| R31.agc:83 | 37,3136 | `VLOAD` | 77775 | `lemprec_ret` |
| R31.agc:84 | 37,3137 | `RATT1` | 00017 | `lemprec_ret` |
| R31.agc:85 | 37,3140 | `STOVL BASETHP` | 26170 | `lemprec_ret` |
| R31.agc:86 | 37,3141 | `VATT1` | 00025 | `lemprec_ret` |
| R31.agc:87 | 37,3142 | `STODL BASETHV` | 16105 | `lemprec_ret` |
| R31.agc:88 | 37,3143 | `TAT` | 00015 | `lemprec_ret` |
| R31.agc:89 | 37,3144 | `DOCMBASE STORE BASETIME` | 02114 | `docmbase` |
| R31.agc:90 | 37,3145 | `STCALL TDEC1` | 34041 | `docmbase` |
| R31.agc:91 | 37,3146 | `CSMPREC` | 27043 | `docmbase` |
| R31.agc:92 | 37,3147 | `VLOAD` | 77775 | `csmprec_ret` |
| R31.agc:93 | 37,3150 | `RATT1` | 00017 | `csmprec_ret` |
| R31.agc:94 | 37,3151 | `STOVL BASEOTP` | 26140 | `csmprec_ret` |
| R31.agc:95 | 37,3152 | `VATT1` | 00025 | `csmprec_ret` |
| R31.agc:96 | 37,3153 | `STORE BASEOTV` | 02120 | `csmprec_ret` |
| R31.agc:97 | 37,3154 | `EXIT` | 77776 | `csmprec_ret` |
| R31.agc:99 | 37,3155 | `REV83 CS FLAGWRD7` | 40103 | `rev83` |
| R31.agc:100 | 37,3156 | `MASK AVEGFBIT` | 74747 | `rev83` |
| R31.agc:101 | 37,3157 | `EXTEND` | 00006 | `rev83` |
| R31.agc:102 | 37,3160 | `BZF GETRVN` | 13326 | `rev83` |
| R31.agc:104 | 37,3161 | `CS FLAGWRD8` | 40104 | `rev83` |
| R31.agc:105 | 37,3162 | `MASK SURFFBIT` | 74744 | `rev83` |
| R31.agc:106 | 37,3163 | `EXTEND` | 00006 | `rev83` |
| R31.agc:107 | 37,3164 | `BZF R31SURF` | 13370 | `rev83` |
| R31.agc:109 | 37,3165 | `TC INTPRET` | 06037 | `rev83` |
| R31.agc:110 | 37,3166 | `RTB` | 77634 | `rev83_rtb` |
| R31.agc:111 | 37,3167 | `LOADTIME` | 21573 | `rev83_rtb` |
| R31.agc:112 | 37,3170 | `STCALL TDEC1` | 34041 | `rev83_intstall` |
| R31.agc:113 | 37,3171 | `INTSTALL` | 27414 | `rev83_intstall` |
| R31.agc:114 | 37,3172 | `VLOAD CLEAR` | 43175 | `intstall_ret` |
| R31.agc:115 | 37,3173 | `BASETHP` | 02170 | `intstall_ret` |
| R31.agc:116 | 37,3174 | `MOONFLAG` | 00263 | `intstall_ret` |
| R31.agc:117 | 37,3175 | `STOVL RCV` | 25535 | `intstall_ret` |
| R31.agc:118 | 37,3176 | `BASETHV` | 02105 | `intstall_ret` |
| R31.agc:119 | 37,3177 | `STODL VCV` | 15543 | `intstall_ret` |
| R31.agc:120 | 37,3200 | `BASETIME` | 02114 | `intstall_ret` |
| R31.agc:121 | 37,3201 | `BOF SET` | 43014 | `intstall_ret` |
| R31.agc:122 | 37,3202 | `MOONTHIS` | 04344 | `intstall_ret` |
| R31.agc:123 | 37,3203 | `+2` | 77205 | `intstall_ret` |
| R31.agc:124 | 37,3204 | `MOONFLAG` | 00063 | `intstall_ret` |
| R31.agc:125 | 37,3205 | `SET` | 77614 | `intstall_ret` |
| R31.agc:126 | 37,3206 | `INTYPFLG` | 01473 | `intstall_ret` |
| R31.agc:127 | 37,3207 | `STCALL TET` | 35517 | `intstall_ret` |
| R31.agc:128 | 37,3210 | `INTEGRVS` | 27107 | `intstall_ret` |
| R31.agc:129 | 37,3211 | `OTHCONIC VLOAD` | 77775 | `othconic` |
| R31.agc:131 | 37,3212 | `RATT` | 00001 | `othconic` |
| R31.agc:132 | 37,3213 | `STOVL RONE` | 26207 | `othconic` |
| R31.agc:133 | 37,3214 | `VATT` | 00007 | `othconic` |
| R31.agc:134 | 37,3215 | `STCALL VONE` | 36215 | `othconic` |
| R31.agc:135 | 37,3216 | `INTSTALL` | 27414 | `othconic` |
| R31.agc:136 | 37,3217 | `SET DLOAD` | 71214 | `intstall_ret2` |
| R31.agc:137 | 37,3220 | `INTYPFLG` | 01473 | `intstall_ret2` |
| R31.agc:138 | 37,3221 | `TAT` | 00015 | `intstall_ret2` |
| R31.agc:139 | 37,3222 | `OTHINT STORE TDEC1` | 00041 | `othint` |
| R31.agc:140 | 37,3223 | `VLOAD CLEAR` | 43175 | `othint` |
| R31.agc:141 | 37,3224 | `BASEOTP` | 02140 | `othint` |
| R31.agc:142 | 37,3225 | `MOONFLAG` | 00263 | `othint` |
| R31.agc:143 | 37,3226 | `STOVL RCV` | 25535 | `othint` |
| R31.agc:144 | 37,3227 | `BASEOTV` | 02120 | `othint` |
| R31.agc:145 | 37,3230 | `STODL VCV` | 15543 | `othint` |
| R31.agc:146 | 37,3231 | `BASETIME` | 02114 | `othint` |
| R31.agc:147 | 37,3232 | `BOF SET` | 43014 | `othint` |
| R31.agc:148 | 37,3233 | `MOONTHIS` | 04344 | `othint` |
| R31.agc:149 | 37,3234 | `+2` | 77236 | `othint` |
| R31.agc:150 | 37,3235 | `MOONFLAG` | 00063 | `othint` |
| R31.agc:151 | 37,3236 | `STCALL TET` | 35517 | `othint` |
| R31.agc:152 | 37,3237 | `INTEGRVS` | 27107 | `othint` |
| R31.agc:153 | 37,3240 | `COMPDISP VLOAD VSU` | 52375 | `compdisp` |
| R31.agc:154 | 37,3241 | `RATT` | 00001 | `compdisp` |
| R31.agc:155 | 37,3242 | `RONE` | 02207 | `compdisp` |
| R31.agc:156 | 37,3243 | `RTB PDDL` | 65234 | `compdisp` |
| R31.agc:157 | 37,3244 | `NORMUNX1` | 21724 | `compdisp` |
| R31.agc:158 | 37,3245 | `36D` | 00045 | `compdisp` |
| R31.agc:159 | 37,3246 | `SL*` | 77657 | `range_rate` |
| R31.agc:160 | 37,3247 | `0,1` | 20201 | `range_rate` |
| R31.agc:161 | 37,3250 | `STOVL RANGE` | 26201 | `range_rate` |
| R31.agc:162 | 37,3251 | `VATT` | 00007 | `range_rate` |
| R31.agc:163 | 37,3252 | `VSU DOT` | 50251 | `range_rate` |
| R31.agc:164 | 37,3253 | `VONE` | 02215 | `range_rate` |
| R31.agc:165 | 37,3254 | `SL1` | 77752 | `range_rate` |
| R31.agc:166 | 37,3255 | `STOVL RRATE` | 26203 | `range_rate` |
| R31.agc:167 | 37,3256 | `RONE` | 02207 | `range_rate` |
| R31.agc:168 | 37,3257 | `UNIT PDVL` | 63256 | `range_rate` |
| R31.agc:169 | 37,3260 | `UNITZ` | 06514 | `range_rate` |
| R31.agc:170 | 37,3261 | `CALL` | 77624 | `range_rate` |
| R31.agc:171 | 37,3262 | `CDU*NBSM` | 47661 | `range_rate` |
| R31.agc:172 | 37,3263 | `VXM PUSH` | 41505 | `cdunbsm_ret` |
| R31.agc:173 | 37,3264 | `REFSMMAT` | 01734 | `cdunbsm_ret` |
| R31.agc:174 | 37,3265 | `VPROJ VSL2` | 72431 | `cdunbsm_ret` |
| R31.agc:175 | 37,3266 | `0D` | 00001 | `cdunbsm_ret` |
| R31.agc:176 | 37,3267 | `BVSU UNIT` | 53445 | `cdunbsm_ret` |
| R31.agc:177 | 37,3270 | `6D` | 00007 | `cdunbsm_ret` |
| R31.agc:178 | 37,3271 | `PDVL VXV` | 47315 | `cdunbsm_ret` |
| R31.agc:179 | 37,3272 | `0D` | 00001 | `cdunbsm_ret` |
| R31.agc:180 | 37,3273 | `VONE` | 02215 | `cdunbsm_ret` |
| R31.agc:182 | 37,3274 | `VXV DOT` | 50235 | `cdunbsm_ret` |
| R31.agc:183 | 37,3275 | `0D` | 00001 | `cdunbsm_ret` |
| R31.agc:184 | 37,3276 | `12D` | 00015 | `cdunbsm_ret` |
| R31.agc:185 | 37,3277 | `PDVL` | 77715 | `cdunbsm_ret` |
| R31.agc:186 | 37,3300 | `DOT SIGN` | 75241 | `cdunbsm_ret` |
| R31.agc:187 | 37,3301 | `6D` | 00007 | `cdunbsm_ret` |
| R31.agc:188 | 37,3302 | `12D` | 00015 | `cdunbsm_ret` |
| R31.agc:189 | 37,3303 | `SL2 ACOS` | 65512 | `cdunbsm_ret` |
| R31.agc:190 | 37,3304 | `STOVL RTHETA` | 26205 | `acos_ret` |
| R31.agc:191 | 37,3305 | `0D` | 00001 | `acos_ret` |
| R31.agc:192 | 37,3306 | `DOT BPL` | 51041 | `acos_ret` |
| R31.agc:193 | 37,3307 | `6D` | 00007 | `acos_ret` |
| R31.agc:194 | 37,3310 | `+5` | 77315 | `acos_ret` |
| R31.agc:195 | 37,3311 | `DLOAD DSU` | 45345 | `acos_ret` |
| R31.agc:196 | 37,3312 | `DPPOSMAX` | 06530 | `acos_ret` |
| R31.agc:197 | 37,3313 | `RTHETA` | 02205 | `acos_ret` |
| R31.agc:198 | 37,3314 | `STORE RTHETA` | 02205 | `acos_ret` |
| R31.agc:199 | 37,3315 | `EXIT` | 77776 | `acos_ret` |
| R31.agc:201 | 37,3316 | `CA BIT5` | 34747 | `exit_ret` |
| R31.agc:202 | 37,3317 | `MASK EXTVBACT` | 71044 | `exit_ret` |
| R31.agc:203 | 37,3320 | `EXTEND` | 00006 | `exit_ret` |
| R31.agc:204 | 37,3321 | `BZF ENDEXT` | 15472 | `exit_ret` |
| R31.agc:206 | 37,3322 | `CS EXTVBACT` | 41044 | `exit_ret` |
| R31.agc:207 | 37,3323 | `MASK BIT12` | 74740 | `exit_ret` |
| R31.agc:208 | 37,3324 | `ADS EXTVBACT` | 27044 | `exit_ret` |
| R31.agc:209 | 37,3325 | `TCF REV83` | 13155 | `exit_ret` |
| R31.agc:211 | 37,3326 | `GETRVN CA PRIO22` | 37710 | `getrvn` |
| R31.agc:212 | 37,3327 | `TC PRIOCHNG` | 05146 | `getrvn` |
| R31.agc:213 | 37,3330 | `TC INTPRET` | 06037 | `prio_ret` |
| R31.agc:214 | 37,3331 | `VLOAD SETPD` | 40375 | `getrvn_i` |
| R31.agc:215 | 37,3332 | `RN` | 01221 | `getrvn_i` |
| R31.agc:216 | 37,3333 | `0` | 00001 | `getrvn_i` |
| R31.agc:217 | 37,3334 | `STOVL RONE` | 26207 | `getrvn_i` |
| R31.agc:218 | 37,3335 | `VN` | 01227 | `getrvn_i` |
| R31.agc:219 | 37,3336 | `STOVL VONE` | 26215 | `getrvn_i` |
| R31.agc:220 | 37,3337 | `V(CSM)` | 01726 | `getrvn_i` |
| R31.agc:221 | 37,3340 | `PDVL PDDL` | 65315 | `getrvn_i` |
| R31.agc:222 | 37,3341 | `R(CSM)` | 01720 | `getrvn_i` |
| R31.agc:223 | 37,3342 | `PIPTIME` | 01235 | `getrvn_i` |
| R31.agc:224 | 37,3343 | `EXIT` | 77776 | `getrvn_i` |
| R31.agc:225 | 37,3344 | `CA PRIO3` | 35015 | `getrvn_exit` |
| R31.agc:226 | 37,3345 | `TC PRIOCHNG` | 05146 | `getrvn_exit` |
| R31.agc:227 | 37,3346 | `TC INTPRET` | 06037 | `prio_ret2` |
| R31.agc:228 | 37,3347 | `BOFF VLOAD` | 77214 | `getrvn_i2` |
| R31.agc:229 | 37,3350 | `MUNFLAG` | 03347 | `getrvn_i2` |
| R31.agc:230 | 37,3351 | `GETRVN2` | 77363 | `getrvn_i2` |
| R31.agc:232 | 37,3352 | `VXM VSR4` | 60505 | `getrvn_i2` |
| R31.agc:233 | 37,3353 | `REFSMMAT` | 01734 | `getrvn_i2` |
| R31.agc:234 | 37,3354 | `PDVL` | 77715 | `getrvn_i2` |
| R31.agc:235 | 37,3355 | `VXM VSL1` | 76505 | `vxm_vsl1` |
| R31.agc:236 | 37,3356 | `REFSMMAT` | 01734 | `vxm_vsl1` |
| R31.agc:237 | 37,3357 | `PUSH SETPD` | 40206 | `vxm_vsl1` |
| R31.agc:238 | 37,3360 | `0` | 00001 | `vxm_vsl1` |
| R31.agc:239 | 37,3361 | `GOTO` | 77650 | `vxm_vsl1` |
| R31.agc:240 | 37,3362 | `COMPDISP` | 77240 | `vxm_vsl1` |
| R31.agc:242 | 37,3363 | `GETRVN2 CALL` | 77624 | `getrvn2` |
| R31.agc:243 | 37,3364 | `INTSTALL` | 27414 | `getrvn2` |
| R31.agc:244 | 37,3365 | `CLEAR GOTO` | 52014 | `intstall_ret3` |
| R31.agc:245 | 37,3366 | `INTYPFLG` | 01673 | `intstall_ret3` |
| R31.agc:246 | 37,3367 | `OTHINT` | 77222 | `intstall_ret3` |
| R31.agc:247 | 37,3370 | `R31SURF TC INTPRET` | 06037 | `r31surf` |
| R31.agc:248 | 37,3371 | `RTB` | 77634 | `r31surf_rtb` |
| R31.agc:249 | 37,3372 | `LOADTIME` | 21573 | `r31surf_rtb` |
| R31.agc:250 | 37,3373 | `STCALL TDEC1` | 34041 | `r31surf_lemprec` |
| R31.agc:251 | 37,3374 | `LEMPREC` | 27057 | `r31surf_lemprec` |
| R31.agc:252 | 37,3375 | `GOTO` | 77650 | `lemprec_ret2` |
| R31.agc:253 | 37,3376 | `OTHCONIC` | 77211 | `lemprec_ret2` |
| R31.agc:254 | 37,3377 | `MUNG? CS FLAGWRD6` | 40102 | `mung` |
| R31.agc:255 | 37,3400 | `MASK MUNFLBIT` | 74744 | `mung` |
| R31.agc:256 | 37,3401 | `EXTEND` | 00006 | `mung` |
| R31.agc:257 | 37,3402 | `BZF GETRVN` | 13326 | `mung` |
| R31.agc:259 | 37,3403 | `ONEBASE TC INTPRET` | 06037 | `onebase` |
| R31.agc:260 | 37,3404 | `RTB GOTO` | 52034 | `onebase_rtb` |
| R31.agc:261 | 37,3405 | `LOADTIME` | 21573 | `onebase_rtb` |
| R31.agc:262 | 37,3406 | `DOCMBASE` | 77144 | `onebase_rtb` |
| R31.agc:264 | 37,3407 | `V16N54 VN 1654` | 04066 | `K_V16N54` |

## Evidence

Hashes are in `evidence/lm-r31/SHA256.json`. Bulk files are assets of the
`evidence-lm-r31` release (`python3 tools/evidence.py fetch lm-r31`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_r31_source.py` reassembles the unchanged Luminary099
  with the pinned yaYUL (the fixture INTERPRETER). The rope equals the
  certified fixed-data rope.
- **Oracle.** `tools/lm_r31_reference.c` (GPL, host only) runs the
  unchanged Luminary099 (`deps/lm-rtb-fixture/MAIN.agc.bin`, equal to the
  certified rope) in the pinned yaAGC hardware-DV variant from 21504
  entries, 512 per routine, to the same stops.
  - The entries sit over poisoned debris with any vectors, times, matrices
    and flags, so AVEGFLAG, SURFFLAG, MUNFLAG, MOONTHIS and the EXTVBACT
    bits take both values.
  - EB is E7 three times in four.
  - They cover every VAC, MODE, PUSHLOC, IRQ and allowed SUPERBNK setting.

  `tools/lm_r31_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`lm-r31-source-stream.json`, SHA256
  `4459efd3...e49f84fa`) and archives the stream
  (`lm-r31-reference.snapshots.bin.xz`). `lm-r31-proof.json` lists the stops
  per routine. Inverting the BPL, changing SURFFBIT, dropping the SL1 or
  dropping the ADS into EXTVBACT each fail it.
- **Host and sanitizer results.** Run `make test-lm-r31 sanitize-lm-r31`.
  The strict build and the ASAN/UBSAN build each pass 22053 checks:
  - all 21504 frozen digests;
  - every run ending at one of the stops;
  - 462 admission guards: EB, BB, FB, ZERO, 16-bit words, SUPERBNK and IRQ
    for every routine, and VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and
    PUSHLOC for the interpretive ones.

  See `lm-r31-checks.log`.
- **C64 execution results.** Run `make vice-lm-r31`. Six PRGs each register
  only the routines their cases reach, built with `-fno-inline-functions`:
  - group 1: COMPDISP (NORMUNX1) (128 cases);
  - group 2: RANGE and RRATE (UNIT) (128 cases);
  - group 3: the return from CDU*NBSM (VXM, VPROJ, BVSU, SIGN) (128 cases);
  - group 4: GETRVN's first REFSMMAT rotation and GETRVN2 (128 cases);
  - group 5: the second rotation (128 cases);
  - group 6: every other entry (518 cases).

  Each keeps the image in the RAM under KERNAL ROM (`$01=$35`). The case
  generator is integer-only, so the C64 builds the same entry images as the
  host. In VICE (PAL, stock ROMs) the six PRGs pass 128, 128, 128, 128, 128
  and 518 checks with no failures (`lm-r31-results.json`).

## Limits

- LOADTIME, LEMPREC, CSMPREC, INTSTALL, INTEGRVS, CDU*NBSM, ARCCOS (ACOS),
  GOMARKF and the Executive and Waitlist entries belong to other files and
  are stops here.
- ONEBASE's right-hand GOTO DOCMBASE (`37,3406`) runs only when LOADTIME
  returns through DANZIG. Native runs stop at LOADTIME, so that word is
  mapped but not executed natively. DOCMBASE itself is an entry.
- Numerical validity is the original's. No wider claim is made.
