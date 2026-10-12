# Translate Luminary099 FINDCDUW--GUIDAP_INTERFACE

Lane A2 (restart, telemetry and support; the file moved here from lane F). Milestone `findcduw`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/FINDCDUW--GUIDAP_INTERFACE.agc` (`src/lm_findcduw.c`, bank 30) is native C in full. FINDCDUW turns the guidance's desired thrust and window directions into gimbal-trim and attitude commands for the DAP (CDUXD/CDUYD/CDUZD and their increments). Each routine runs the file from its entry, or from where it resumes after a subroutine, to the point where control leaves the file.

## Scope

| Luminary099 | Source | Leaves at |
|---|---|---|
| 30,3104 INITCDUW | UNFV/2 and UNWC/2 set to UNITX, RVQ | the caller (NEWOPS) |
| 30,3111 FINDCDUW -2, 30,3113 FINDCDUW, and the resumes after its CALLs | BOV, SETPD, STQ, EBANK and DAPBOOLS flags, the thrust and window frame (UNIT, VXV, DOT), the gimbal-angle limits | the caller through QCDUWUSR, ALARM |
| 30,3165 PAUTNO +1 and its resumes | RTB NORMUNIT; RTB QUICTRIG; the commanded CDUs, filter and MGA check (ALARMMGA, NOATTCNT) | ALARM, the caller |
| CDUWXFR, TCQCDUW, FLTRSUB | the CDU-window transfer under INHINT, the restore of the user's EBANK, the two-pass filter (READ/RAND of channels 30 and 31, GAINFLTR indexed by NDXCDUW) | the caller |
| NB2CDUSP, DVBYCOSM, ARCTRGSP, SPARCSIN -1, SPARCSIN, LIMITSUB, ONESTO2S, 1TO2&TCQ | the matrix-to-CDU conversion and its single-precision trig, with returns saved in TEM2 and TEM4 | the caller through Q |
| NOATTCNT, ALARMMGA | `TC ALARM` with the alarm code | ALARM |

Callers enter at FINDCDUW -2 (P40-P47, LUNAR_LANDING_GUIDANCE_EQUATIONS, ASCENT_GUIDANCE), INITCDUW (BURN_BABY_BURN) and SPARCSIN -1 (a CADR in LUNAR_LANDING_GUIDANCE_EQUATIONS); all three are registered entries. Destinations outside the file (ALARM, IBNKCALL, NEWOPS with LOC outside the file) stop with `EXEC_UNMAPPED` until their files have routines.

## Native pieces

- **RTB NORMUNIT** (`lm_findcduw_normunit`, `src/lm_findcduw_rtb.c`): `rtb_vector`'s NORMUNIT front (the base address, VECAGREE, the 13-bit pre-shift of a small vector, POSTJUMP), then `lmi_unit1`, UNIT+1 of the interpreter's UNIT, to DANZIG. `lmi_unit` now calls `vecagree` then `lmi_unit1`, so its code is unchanged; the Comanche build maps it to `cmi_unit1`. UNIT's alarm 1302 (a zero vector) is refused.
- **RTB QUICTRIG** (`lm_findcduw_quictrig`): `lm_powered_quick` to its TC A, its return to SWRETURN, then `lmi_swreturn` to DANZIG.
- **Channels 30 and 31**: the routines read them from `lm_channel_io` (`include/lm_channel_io.h`: `p66_bias_io` plus the two channels), as yaAGC does (`RAND`: A AND the channel, sign-extended).

## Compiler extensions (`tools/interp_compile.py`)

- INDEX on a fixed operand (CA, CS, AD, MASK, MP): a switch over the rope words the index can reach, refusing any other address.
- READ and RAND of the channels a spec lists.
- A CCS arm landing on the word after an EXTEND is refused.
- BOVB into the file adds a basic start.
- `TC` to an erasable word above the central registers (a saved return, TC TEM2 / TC TEM4): a switch over the return points the routine has seen, otherwise `tc_e`.
- `tools/interp_routine_differential.py` takes single-mission specs, channel edits, an OVFIND domain (BOV decrements it), whole-routine states (NDXCDUW 0 or 1) and per-spec C64 flags (`-fno-inline` here) and C64 cases per routine (one here).

## Evidence

Hashes: `evidence/findcduw/SHA256.json`. Bulk files are assets of the `evidence-findcduw` release.

- **Source map.** `findcduw-source-map.json` lists all 441 words with the routine that compiles each; none is untranslated.
- **Independent assembly comparison.** `make findcduw-differential`: 512 states per routine (23,040 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. The cases run with channels 30 and 31 at their idle values or with the PNGCS control bit (channel 30 bit 10) or the auto-mode bit (channel 31 bit 14) set active, OVFIND +0, +1 or -1, NDXCDUW 0 or 1, MODE -1 where the RTBs need a vector, and TEM2/TEM4 and the filter indexes at the values their callers leave. The stops reached per routine are in `findcduw-proof.json`. Every 16th case is frozen in `tests/findcduw_vectors.h`.
- **Host and sanitizer results.** `make test-findcduw sanitize-findcduw` runs the 1,440 frozen cases (`findcduw-host.log`).
- **C64 execution results.** `make vice-findcduw` runs ten diagnostics, one per group of routines, with 45 cases, one per routine (`findcduw-vice.log`). The image lives in the RAM under KERNAL at $e000.
