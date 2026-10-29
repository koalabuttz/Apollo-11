# Translate Luminary099 FINDCDUW--GUIDAP_INTERFACE

Lane A2 (restart, telemetry and support; the file moved here from lane F). Milestone `findcduw`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/FINDCDUW--GUIDAP_INTERFACE.agc` (`src/lm_findcduw.c`, bank 30) is native C in full. FINDCDUW turns the guidance's desired thrust and window directions into gimbal-trim and attitude commands for the DAP (CDUXD/CDUYD/CDUZD and their increments). Each routine runs the file from its entry, or from where it resumes after a subroutine, to the point where control leaves the file.

## Scope

| Luminary099 | Source | Leaves at |
|---|---|---|
| 30,3104 INITCDUW | UNFV/2 and UNWC/2 set to UNITX, RVQ | the caller (NEWOPS) |
| 30,3111 FINDCDUW -2, 30,3113 FINDCDUW, and the resumes after its CALLs | BOV, SETPD, STQ, EBANK and DAPBOOLS flags, CDUX-Z, RAND of channels 30 and 31, the thrust and window frame (UNIT, VXV, DOT), the gimbal-angle limits | the caller through QCDUWUSR, ALARM |
| 30,3165 PAUTNO +1 and its resumes | RTB NORMUNIT; RTB QUICTRIG; the commanded CDUs, filter and MGA check (ALARMMGA, NOATTCNT) | ALARM, the caller |
| CDUWXFR, TCQCDUW, FLTRSUB | the CDU-window transfer under INHINT, the restore of the user's EBANK, the two-pass filter (GAINFLTR indexed by NDXCDUW) | the caller |
| NB2CDUSP, DVBYCOSM, ARCTRGSP, SPARCSIN -1, SPARCSIN, LIMITSUB, ONESTO2S, 1TO2&TCQ | the matrix-to-CDU conversion and its single-precision trig, with returns saved in TEM2 and TEM4 | the caller through Q |
| NOATTCNT, ALARMMGA | `TC ALARM` with the alarm code | ALARM |

Callers enter at FINDCDUW -2 (P40-P47's CALLs, and the guidance equations), INITCDUW (BURN_BABY_BURN, P70-P71) and SPARCSIN -1 (a CADR in LUNAR_LANDING_GUIDANCE_EQUATIONS); all three are registered entries. Destinations outside the file (ALARM, IBNKCALL, NEWOPS with LOC outside the file) stop with `EXEC_UNMAPPED` until their files have routines.

Every word that can follow a DANZIG is a routine of its own (`resume_keys`): FINDCDUW runs in the guidance equations' job and INITCDUW in BURNBABY's or P70/P71's, so a DANZIG that finds NEWJOB positive stops at CHANG2 and resumes there. The file compiles to 75 routines.

## Native pieces

- **RTB NORMUNIT** (`lm_findcduw_normunit`, `src/lm_findcduw_rtb.c`): `rtb_vector`'s NORMUNIT front (the base address, VECAGREE, the 13-bit pre-shift of a small vector, POSTJUMP to UNIT+1), then `rtb_unit_phase`'s three phases, the existing UNIT+1 continuation (VSQSUB, SQRTSUB, the divide) to DANZIG. A DVOVF exit ends at DANZIG too; UNIT's alarm 1302 (a zero vector) is refused. The shared interpreter library is unchanged.
- **RTB QUICTRIG** (`lm_findcduw_quictrig`): `lm_powered_quick` to its TC A, its return to SWRETURN, then `lmi_swreturn` to DANZIG.
- **Channels 30 and 31**: PAUTNO's `RAND CHAN30` and `RAND CHAN31` read the input channels the caller latches for the run (`channel_inputs`, `include/agc_input_channels.h`), as yaAGC does (RAND: the overflow-corrected A AND the channel, sign-extended).

## Compiler and differential additions

See `docs/INTERP-COMPILE.md` (additions from FINDCDUW):

- `basic_fixed_index` also covers `INDEX K; MP T` (FLTRSUB's `INDEX NDXCDUW; MP GAINFLTR`); the spec lists GAINFLTR, DAZMAX, DAY/2MAX and DAXMAX with NDXCDUW 0 or 1, and SPARCSIN's `INDEX A; CS LIMITS` with A +-1.
- BOVB to the file's own basic code (`BOVB UNIT NOATTCNT`) without `'basic': 'full'`.
- `refuse_unextended`: the -0 arm of the CCS A after each RAND is the word after the next EXTEND, refused rather than compiled unextended.
- `index_carry16`: INDEX's end-around carry also on the C64, where `unsigned` is 16 bits (SPARCSIN's `INDEX A` with A = -1 refused there before).
- `image_w` and `trusted_image`: flight builds (RK_FIXED_IMAGE) address the fixed $7000 image and skip the erasable-image scan at each routine entry (about 121,000 6502 cycles a call; FINDCDUW runs every guidance pass in powered flight).
- `basic_edges`: the erasable words the basic code names take +-0, small and saturating values (every CCS, BZF and BZMF arm, TS overflow).

## Evidence

- **Source map.** `findcduw-source-map.json` lists all 441 words with the routine that compiles each; none is untranslated.
- **Independent assembly comparison.** `make findcduw-differential`: 256 states per routine (19,200 cases over 75 routines) equal yaAGC in Z, all 2,048 erasable words, the interrupt-enable state, channels 30 and 31 and the 31A/31B/32 trap flags. The cases run with the PNGCS control bit (channel 30 bit 10) and the auto-mode bit (channel 31 bit 14) set or clear, NEWJOB positive in a quarter of them (the CHANG2 stops), the counters seeded, edge vectors at the operands, OVFIND +0, +1 or -1, NDXCDUW 0 or 1, MODE -1 where the RTBs need a vector, and TEM2/TEM4 at the values their callers leave. The stops reached per routine are in `findcduw-proof.json`. Every 16th case is frozen in `tests/findcduw_vectors.h`.
- **Host and sanitizer results.** `make test-findcduw sanitize-findcduw` runs the 1,200 frozen cases.
- **C64 execution results.** `make vice-findcduw` runs fourteen diagnostics, one per group of routines, with 146 cases chosen so that they take all 1,957 branch arcs (gcov, every linked source) the 19,200 host cases take (`findcduw-c64-cover.json`). The image lives in the RAM under KERNAL at $e000; the PRGs build with `-fno-inline -fstatic-stack -mlto-zp=110`.
