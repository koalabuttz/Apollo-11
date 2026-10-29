# Translate ASCENT_GUIDANCE

Lane D1. Milestone `lm-ascent`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/ASCENT_GUIDANCE.agc` (`src/lm_ascent.c`) is native C in full. It is the LM's ascent and abort guidance. SERVICER enters it at ATMAG on each pass of P12, P42, P70 and P71. ASCENT computes the state relative to the target orbit: radial rate, crossrange, the velocity to be gained and the time to go. T2TEST, CMPONENT, AIMER and MAINLINE turn that into the commanded thrust direction and attitude. ASCTERM and the EXITVR exits update the displays (V06N63, V06N76, V06N33, V16N85). KEEPVR, ENGOFF and CUTOFF time and command engine shutdown. CHECKYAW, ANG1CHEK, OFFROT and YCOMP are the yaw and attitude checks.

## Scope

| Bank | Words | Code |
|---|---|---|
| 34 | 44 (34,3642-3715) | ATMAG |
| 30 | 371 (30,2321-3103) | ASCENT, MAINENG, T2TEST, CMPONENT, AIMER, MAINLINE, ASCTERM to ASCTERM4, CHECKALT, EXITVR, EXITVR1, CUTOFF2, TERMASC, the display and time constants, LOGSUB |
| 27 | 22 (27,3273-3320) | ANG1CHEK, OFFROT, YCOMP |
| 07 | 21 (07,2667-2713) | CHECKYAW |
| 05 | 4 (05,3502-3505) | SIN5DEG, 40FPS |
| 14 | 63 (14,2347-2445) | KEEPVR, KEEPVR1, ENGOFF, ENGOFF1, CUTOFF, CUTOFF1 |
| 33 | 6 (33,2200-2205) | KT1, PRLIMIT, MINABDV |

That is 531 words in 209 routines: every entry (ENGOFF1 and TERMASC too: the file's own TC TWIDDLE at 14,2375 and POSTJUMP at 14,2434 reach them, and the group 4 restart, PHASCHNG OCT 47014, re-establishes ENGOFF1), every return from an outside call, and every interpretive word after a DANZIG (`resume_keys`; `resume_orphans` also registers the words that only a resume reaches).

ENGOFF1 and its returns run as a waitlist task over whatever job it interrupts: FIXLOC may be a core set and NEWJOB -0 while DUMMYJOB idles. With `any_fixloc` and `any_newjob`, every basic routine that never names FIXLOC or NEWJOB takes those states, and part of its cases draw them (#403). `trusted_image` drops the entry checks' image scan in flight builds only.

Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines:
- basic: BANKCALL, CHANG2, DOWNFLAG, FINDVAC, IBNKCALL, PHASCHNG, POLY, POSTJUMP, SHORTMP, TPAGREE, TWIDDLE and UPFLAG, with their inline words, and TCF ENDOFJOB, GOTOPOOH and TASKOVER;
- interpretive: NEWOPS with LOC outside the file, and the interpreter's POODOO1 and ALARM1 exits.

RTB LOADTIME is native (`src/rtb_angles.c`). The interpretive orders are the compiled interpreter bodies.

## States

The cases start from random erasable states. Every routine is compared from any values of the words it reads. The one pin: at ENGOFF's `CCS MPAC +1` (14,2364), most cases have a time to cutoff of exactly 0 cs, the one-bit ENGOFFDT case, so the +0 arm is compared (`states`). A quarter of the cases put edge values (`edge_cases`) into MPAC, the top of the pushdown list and every operand the file names.

## Evidence

Hashes: `evidence/lm-ascent/SHA256.json`. Bulk files are assets of the `evidence-lm-ascent` release.

- **Source map.** `lm-ascent-source-map.json` lists every emitted word with its routine: 531 words, none untranslated.
- **Independent assembly comparison.** `make lm-ascent-differential`: 512 states per routine (107,008 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Each run goes from the routine's entry state to the file's boundary (`boundary` in `lm-ascent-proof.json`).
- **Host and sanitizer results.** `make test-lm-ascent sanitize-lm-ascent` runs the 3,344 frozen cases (every 32nd).
- **C64 execution results.** `make vice-lm-ascent` runs 29 diagnostics with 251 cases. 21 of them are runs cut short where the oracle stops too (spec `c64_cuts`, `tests/interp_cut.h`). They cover the arcs that only runs too long for one diagnostic take. Together the cases take every gcov branch arc the host cases take (1,734 in `lm-ascent-c64-cover.json`). The diagnostics build without inlining. The log gives each PRG's sha256.
