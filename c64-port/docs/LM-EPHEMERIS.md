# Translate Luminary lunar and solar ephemerides (LSPOS/LUNPOS, LUNVEL)

Lane B. Milestone `lm-ephemeris`.

## Scope

`Luminary099/LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc`, all of it: LSPOS
(=LUNPOS) at bank 15 `03664` and LUNVEL at `03775`, 74 assembled words. The
file has no other executable code. Nothing is left in it.

`lm_ephemeris(state, io)` runs from the original interpretive entry frontier
(NEWOPS with LOC `03664` or `03775`, FBANK `032000`) to the caller's NEWOPS
frontier selected by QPRET, and returns 1, or 2 at the original UNIT SQRT abort
frontier, or 0 with the image unchanged when the entry is outside the
contract. Admitted entries: canonical 2048-word image, coherent EB/FB/BB with
BANKSET = BB, ZERO/EDOP/NEWJOB/INTBIT15/MODE zero, one of the five VACs, all
words 15-bit, and QPRET one of the actual callers: ORBITAL_INTEGRATION
`022563`, `023273`, `023373`, P51-P53 `030512` and S-band `065627` (under SUPER
`0100`) for LSPOS; `023417` for LUNVEL.

### How it is translated

Every interpretive pair is compiled. `src/lm_interp_steps.c` holds native
translations of the fixed paths of Luminary099 INTERPRETER.agc that LSPOS
reaches (DANZIG, NEWOPS, OPJUMP, direct/indexed/misc/unary address formation,
STORE and STORE,2, DLOAD, SLOAD, PDDL, SETPD, VDEF, AXT, INCR, SWITCHES with
SWBRANCH, BZE, BPL, GOTO and RVQ, and the DDV and GSHIFT prologues). The
caller picks each path at compile time and passes the order and operand words
of its own source line as literal constants; those words only reproduce the
original bookkeeping (EDOP, CYR, ADDRWD, POLISH, LOC, banks, A/L/Q). Nothing is
fetched from or decoded out of code memory. Arithmetic runs in the existing
frozen native bodies: time downscale and scalar shifts, TAD, the day DDV,
DMP/DAD/DSU, SINE/COSINE, MXV and UNIT. Constants (KONMAT, RATESP, VAL67,
CSTODAY, RCB-13) come from the certified fixed-data tables.

The routine is split into nine steps between original NEWOPS frontiers
(`enum lm_ephemeris_phase`, `lm_ephemeris_step`, and the `lme_*` entry points).
The image carries every continuation, so each step can become its own overlay
module later.

Original details the translation reproduces, found by stepping the oracle:
MASK reads without the editing write-back; BRANCH treats -0 as zero (CCS
falls through to the next word); EBANK-window addresses go through EB; yaAGC
overflow-corrects L at every instruction boundary; `TC Q` leaves Q = 3; the
frozen GENSHIFT kernels need the qualified descriptor `020200+count`
(`020616` for SR 14D), so `lme_shift` stages it and restores the actual
bank-15 ADDRWD word, which gives the same count, direction and debris.

### Interpreter ABI

`lm_interp_steps` is new shared interpreter code. Lane A owns the
interpreter ABI and should review it; it adds no runtime entry points and
changes no existing file.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_ephemeris_source.py`, `evidence/lm-ephemeris/lm-ephemeris-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| 98 | 15,3664 | `LSPOS SETPD SR` | 54201 | `lme_entry` |
| 99 | 15,3665 | `0` | 00001 | `lme_entry` |
| 100 | 15,3666 | `14D` | 20617 | `lme_entry` |
| 101 | 15,3667 | `TAD DDV` | 56371 | `lme_entry` |
| 103 | 15,3670 | `TEPHEM` | 01707 | `lme_entry` |
| 104 | 15,3671 | `CSTODAY` | 12024 | `lme_entry` |
| 105 | 15,3672 | `STORE TIMEP` | 00031 | `lme_entry` |
| 106 | 15,3673 | `AXT,1 AXT,2` | 77170 | `lme_entry` |
| 107 | 15,3674 | `0` | 00000 | `lme_entry` |
| 108 | 15,3675 | `0` | 00000 | `lme_entry` |
| 109 | 15,3676 | `CLEAR` | 77614 | `lme_entry` |
| 110 | 15,3677 | `FREEFLAG` | 00274 | `lme_entry` |
| 111 | 15,3700 | `POSITA DLOAD` | 77745 | `lme_posita` |
| 112 | 15,3701 | `KONMAT +2` | 12004 | `lme_posita` |
| 113 | 15,3702 | `STORE GTMP` | 00027 | `lme_posita` |
| 114 | 15,3703 | `POSITB DLOAD DMP*` | 40745 | `lme_posita` |
| 115 | 15,3704 | `TIMEP` | 00031 | `lme_posita` |
| 116 | 15,3705 | `VAL67 +4,1` | 12050 | `lme_posita` |
| 118 | 15,3706 | `SL DAD*` | 42661 | `lme_posita` |
| 119 | 15,3707 | `8D` | 20211 | `lme_posita` |
| 120 | 15,3710 | `VAL67 +2,1` | 12046 | `lme_posita` |
| 121 | 15,3711 | `SIN DMP*` | 40756 | `lme_posita` |
| 122 | 15,3712 | `VAL67,1` | 12044 | `lme_posita` |
| 123 | 15,3713 | `DAD INCR,1` | 62015 | `lme_posita` |
| 124 | 15,3714 | `GTMP` | 00027 | `lme_posita` |
| 125 | 15,3715 | `DEC -6` | 77771 | `lme_posita` |
| 126 | 15,3716 | `STORE GTMP` | 00027 | `lme_posita` |
| 127 | 15,3717 | `BOFSET` | 77614 | `lme_posita` |
| 128 | 15,3720 | `FREEFLAG` | 00054 | `lme_posita` |
| 129 | 15,3721 | `POSITB` | 33703 | `lme_posita` |
| 130 | 15,3722 | `POSITD DLOAD DMP*` | 40745 | `lme_positd` |
| 131 | 15,3723 | `TIMEP` | 00031 | `lme_positd` |
| 132 | 15,3724 | `RATESP,2` | 65747 | `lme_positd` |
| 133 | 15,3725 | `SL DAD*` | 42661 | `lme_positd` |
| 134 | 15,3726 | `5D` | 20206 | `lme_positd` |
| 135 | 15,3727 | `RATESP +6,2` | 65741 | `lme_positd` |
| 136 | 15,3730 | `DSU` | 77625 | `lme_positd` |
| 137 | 15,3731 | `GTMP` | 00027 | `lme_positd` |
| 138 | 15,3732 | `STORE STMP,2` | 10021 | `lme_positd` |
| 139 | 15,3733 | `SLOAD INCR,2` | 63135 | `lme_positd` |
| 140 | 15,3734 | `X2` | 00050 | `lme_positd` |
| 141 | 15,3735 | `DEC -2` | 77775 | `lme_positd` |
| 142 | 15,3736 | `DAD BZE` | 53015 | `lme_positd` |
| 143 | 15,3737 | `RCB-13` | 12026 | `lme_positd` |
| 144 | 15,3740 | `POSITE` | 33770 | `lme_positd` |
| 145 | 15,3741 | `BPL` | 77644 | `lme_positd` |
| 146 | 15,3742 | `POSITA` | 33700 | `lme_positd` |
| 147 | 15,3743 | `POSITF DLOAD DSU` | 45345 | `lme_positf` |
| 148 | 15,3744 | `STMP` | 00021 | `lme_positf` |
| 149 | 15,3745 | `STMP +4` | 00025 | `lme_positf` |
| 150 | 15,3746 | `SIN PDDL` | 65356 | `lme_positf` |
| 151 | 15,3747 | `STMP` | 00021 | `lme_positf` |
| 152 | 15,3750 | `SIN PDDL` | 65356 | `lme_positf` |
| 153 | 15,3751 | `STMP` | 00021 | `lme_positf` |
| 154 | 15,3752 | `COS VDEF` | 55546 | `vdef` |
| 155 | 15,3753 | `MXV UNIT` | 53521 | `lme_moon_unit` |
| 156 | 15,3754 | `KONMAT` | 12002 | `lme_moon_unit` |
| 157 | 15,3755 | `STORE VMOON` | 02723 | `lme_moon_unit` |
| 158 | 15,3756 | `DLOAD PDDL` | 65345 | `lme_sun_angles` |
| 159 | 15,3757 | `KONMAT +2` | 12004 | `lme_sun_angles` |
| 160 | 15,3760 | `STMP +2` | 00023 | `lme_sun_angles` |
| 161 | 15,3761 | `SIN PDDL` | 65356 | `lme_sun_angles` |
| 162 | 15,3762 | `STMP +2` | 00023 | `lme_sun_angles` |
| 163 | 15,3763 | `COS VDEF` | 55546 | `vdef` |
| 164 | 15,3764 | `MXV UNIT` | 53521 | `lme_sun_unit` |
| 165 | 15,3765 | `KONMAT` | 12002 | `lme_sun_unit` |
| 166 | 15,3766 | `STORE VSUN` | 02715 | `lme_sun_unit` |
| 167 | 15,3767 | `RVQ` | 77616 | `lme_sun_unit` |
| 169 | 15,3770 | `POSITE DLOAD` | 77745 | `lme_positd` |
| 170 | 15,3771 | `KONMAT +2` | 12004 | `lme_positd` |
| 171 | 15,3772 | `STORE GTMP` | 00027 | `lme_positd` |
| 172 | 15,3773 | `GOTO` | 77650 | `lme_positd` |
| 173 | 15,3774 | `POSITD` | 33722 | `lme_positd` |
| 174 | 15,3775 | `LUNVEL RVQ` | 77616 | `lme_entry` |

## Evidence

Hashes: `evidence/lm-ephemeris/SHA256.json`. Bulk files are assets of the `evidence-lm-ephemeris` release
(`python3 tools/evidence.py fetch lm-ephemeris`).

- Source-to-native map and independent assembly comparison:
  `python3 tools/lm_ephemeris_source.py` reassembles the unchanged file with the
  pinned yaYUL (rope equal to the certified fixed-data rope), requires the
  fresh line/word map to equal the retained `evidence/ephemeris-source-map.json`
  and maps all 74 words to native literals in 8 functions.
- Oracle: `tools/lm_ephemeris_reference.c` (GPL, host only) runs the unchanged
  Luminary099 in the pinned yaAGC hardware-DV variant from 65536 sampled entry
  states (`tests/lm_ephemeris_cases.c`: every caller, both IRQ settings, SUPER,
  all five VACs, EBANK, edge values of MPAC and TEPHEM, LUNVEL with overflowed
  L) to the caller frontier. `tools/lm_ephemeris_differential.py` checks the
  native producer against every row and pins the stream identity
  (`lm-ephemeris-source-stream.json`, SHA256 `c3617d86...a5c466`, 269,090,816
  bytes; all 65536 states return).
- Host and sanitizer results: `make test-lm-ephemeris sanitize-lm-ephemeris`.
  Strict and ASAN/UBSAN builds each pass 16 frozen whole-routine states, 21
  admission guards, 54 frozen phase steps and the full 65536-state stream
  (`lm-ephemeris-checks.log`).
- Phase frontiers: `tools/freeze_lm_ephemeris_phases.py` requires each of the
  152 native frontiers of the 16 frozen states to equal, in order, an original
  NEWOPS image of the oracle, then freezes 54 steps in three groups.
- C64 execution results: `make vice-lm-ephemeris`. The whole routine links to
  about 51 KB of 6502 code, more than one PRG holds beside two images, so
  three PRGs each link one phase group and keep both images in the RAM under
  KERNAL ROM (`$01=$35`). In VICE (PAL, stock ROMs): group 1 (entry and the
  POSITB loop) 12 steps, group 2 (POSITD to the solar angles) 30 steps, group 3
  (MXV/UNIT/STORE and RVQ) 12 steps, no failures (`lm-ephemeris-results.json`).

## Limits

- The C64 check runs each phase separately; the whole chain runs on the host.
  Running it as one program needs overlay ABI 2 modules (the nine phases are
  the module entry points).
- Validity of the ephemeris polynomials is the original's; no widened claim.
