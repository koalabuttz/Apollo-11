# Translate AOTMARK

Lane D1 (IMU and hardware). Milestone `lm-aotmark`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/AOTMARK.agc` (`src/lm_aotmark.c`) is native C in full. It is the LM's AOT and COAS mark system: AOTMARK claims a VAC area for the marks and starts GETDAT, which displays the detent and star code (V01 N71), takes the detent's azimuth and elevation (or the COAS angles for detent 7), and builds the optic axis and the X and Y mark planes (OANB). MARKRUPT takes the X, Y and reject mark buttons (channel 16), storing the CDUs and time of each mark in the VAC area. MARKCHEX averages each mark pair's star vector in stable-member coordinates (AVESTAR, AVEIT). On the lunar surface, SURFSTAR uses the cursor and spiral angles (V06 N79) in place of the pairs. ENDMARK and MKRELEAS free the area.

## Scope

| Bank | Words | Code |
|---|---|---|
| 07 (AOTMARK1) | 439 (07,2000-2666) | AOTMARK, GETDAT, CODE1TO6/CODE7, OPTAXIS, YZCHK, GETMKS, MARKCHEX, AVESTAR, AVEIT, MARKRUPT and its key decode, SURFSTOR, CHANGEVB, DSPV6N79, SURFAGAN, MKRELEAS |
| 04 (AOTMARK2) | 32 (04,2536-2575) | OANB |
| 15 (P50S) | 40 (15,2000-2047) | SURFSTAR |

That is 511 words in 122 routines: every entry, every return from a BANKCALL display or POSTJUMP, and every interpretive word after a DANZIG (`resume_keys`).

Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines:
- basic: ALARM, BAILOUT1, BANKCALL (GOMARKF, GOMARK4 and the other display CADRs), ENDMARK, FINDVAC, GOTOPOOH, IBNKCALL, NOVAC, POODOO, POSTJUMP (DESCBITS), RESUME and WAITLIST, with their inline words;
- interpretive: CALL and STCALL to TRG*NBSM;
- CHANG2, when a DANZIG finds NEWJOB positive.

RTB CDULOGIC is native (`src/rtb_angles.c`). SIN, COS, UNIT, VXV, VXSC and the other orders are the compiled interpreter bodies. The compiler changes this file needed:
- channel 016 (NAVKEYIN) is a latched input like channels 3, 4 and 030-033;
- channels 1 and 2 are register operations;
- `index_carry` compiles MKVACFND's `INDEX MARKSTAT; TS 0 -1`, which assembles as DXCH 1777 and runs as TS VACnUSE;
- MKVB54's indexed fixed table is `basic_fixed_index`.

## States

The cases hold the erasables AOTMARK reads at values the flight program gives them. Random words there take paths flight never takes, such as indexing into the rope or the registers. The pinned values:
- **VAC areas.** VAC1USE to VAC5USE are 0 (busy) or their own address (free, as FRESH_START leaves them).
- **MARKSTAT.** Either 0, or the marks' VAC area +1 in the low 9 bits under any of flag bits 10-14. Past MARKRUPT's CCS MARKSTAT, it is never 0. At the AOTMARK entry, half the cases have MARKSTAT 0 (joint_states), so MKVACFND's search runs past every VAC area, busy or free, through to BAILOUT1. A quarter of all entry cases (131 of 512) have only VAC5 free, with EXTVBACT's bits 1-2 clear, so the search takes its last area (07,2024-2025).
- **FIXLOC.** Any value in the basic routines (`any_fixloc`): CHANGEVB runs under NOVAC with FIXLOC 0110, and MARKRUPT and MKRELEAS can interrupt it. Half of their cases put FIXLOC outside the VAC areas.
- **Counters and codes.** MARKCNTR is 0-5. AOTCODE holds a detent of 0-7 in bits 7-9 under a star code.
- **MKDEX.** In flight it is the mark verb index (0-3). On the surface (FLAGWRD8 bit 8), it is a mark slot: the area +1 plus 6 words a mark.
- **Star-vector code.** X1 is minus a slot, XYMARK is the VAC area, and PUSHLOC is shallow, as AVESTAR leaves them.
- **Entry state.** MKVACFND's A is a free area's address -1.

Elsewhere, `flow_states` and `resume_flow` start mid-file routines from the PUSHLOC, MODE and index registers the file's own runs reach them with.

## Evidence

Hashes: `evidence/lm-aotmark/SHA256.json`. Bulk files are assets of the `evidence-lm-aotmark` release.

- **Source map.** `lm-aotmark-source-map.json` lists every emitted word with its routine: 511 words, none untranslated.
- **Independent assembly comparison.** `make lm-aotmark-differential`: 512 states per routine (62,464 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. NEWJOB is positive in a quarter of them; those runs stop at CHANG2 at the first DANZIG, in the oracle as in the native code. TIME1, TIME2 and the CDUs vary in every case. A quarter of the cases put edge values into MPAC, the top of the pushdown list and every operand the file names.
- **Host and sanitizer results.** `make test-lm-aotmark sanitize-lm-aotmark` runs the 2,030 frozen cases: every 32nd, plus a greedy cover of every branch outcome any case reaches, counted per call site (spec `site_edges`): 18,693 outcomes (`outcome_cover` in `lm-aotmark-proof.json`).
- **C64 execution results.** `make vice-lm-aotmark` runs 21 diagnostics with 302 cases, one of them a run cut short where the oracle stops too (spec `c64_cuts`, `tests/interp_cut.h`), for the DDV arcs only a run too long for one diagnostic takes. They take every gcov branch arc the host cases take (1,638; `c64_gcov_branches` in the proof, `lm-aotmark-c64-cover.json`) and 18,691 of the 18,693 per-site outcomes. The two left are taken only by runs too long for one diagnostic: the negative CCS branch of `agc_image_ops.c:193` as called from 15,2007, and the overflow branch of `lm_interp_orders.c:290` as called from 15,2041 (JUSTOA-1). The C64 cases take both branches from other call sites. The diagnostics build without inlining (`c64_flags`). The log gives each PRG's sha256.
