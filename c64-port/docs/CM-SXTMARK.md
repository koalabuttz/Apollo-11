# Translate SXTMARK

Lane D1 (IMU and hardware). Milestone `cm-sxtmark`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Comanche055/SXTMARK.agc` (`src/cm_sxtmark.c`) is native C in full. It is the CM's sextant mark system: SXTMARK claims a VAC area for the marks and starts the mark display job (MKVB51: V51 "please mark", V50 N25 when all marks are in). MARKRUPT takes the mark, mark reject and key code inputs on channel 16 (NAVKEYIN), storing the CDUs, optics angles and time of each mark. A WAITLIST task (MARKDIF, DIFCHK) refuses a mark taken while the vehicle rate moved a CDU more than three bits, and MARKCONT files it: in the VAC area's next 7-word slot, in R21's mark buffers, or for P23's V59 calibration display. MKREJECT backs out the last mark. ENTANSWR, ENDMARKS and MKRELEAS end the marking and free the area.

## Scope

| Bank | Words | Code |
|---|---|---|
| 07 (SXTMARKE) | 287 (07,2002-2440) | SXTMARK, MARKOK, MKVACFND, MKRELEAS, MKRLEES, MARKRUPT and its key decode, MARKCONT, MARKET, MARK2, VACSTOR, MKREJECT, MRKREJCT, MKVB51, TERMSXT, ENTANSWR, ENDMARKS, MKVB5X, MKVB50, MARKIT |
| 10 (SXTMARK1) | 28 (10,2203-2236) | MARKDIF, DIFCHK |

The file is all basic code, compiled whole (`'basic': 'full'`, `tools/basic_compile.py`) into 42 routines: every word another file names, the file's own CADRs, and the returns the outside calls make past their first (CHECKMM's in-mode return and GOMARK4's verb 33 and enter returns, spec `entries`).

Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines: ALARM, BAILOUT, BANKCALL (KLEENEX, GOMARK4), CHECKMM, CLEARMRK, ENDMARK, GENTRAN, IBNKCALL (GOODEND, MARKCONT's caller), NOVAC, POSTJUMP (KEYCOM, TERM52), RESUME, SWRETURN, TASKOVER, UPFLAG and WAITLIST, with their inline words.

The compiler changes this file needed:
- `basic_compile.py` reads input channels as `tools/interp_compile.py` does under `channel_inputs`: RAND NAVKEYIN takes channel 016 from the caller's latches;
- an indexed MSU (DIFCHK's `INDEX MKNDX; MSU CDUX`) runs on the summed erasable address;
- MKVACFND's `INDEX MARKSTAT; TS 0 -1` (DXCH 1777 until the index is added) is an `index_words` entry, one TS VACnUSE per VAC area.

## States

The cases hold the erasables SXTMARK indexes at values the flight program gives them:
- **VAC areas.** VAC1USE to VAC5USE are 0 (busy) or their own address (free, as FRESH_START leaves them).
- **MARKSTAT.** Either 0, or the marks' VAC area in the low 9 bits under any of flag bits 10-14.
- **QPRET.** Each area's mark-slot pointer (its word 42D) is the area plus 7 words a mark taken, 0 to 5 marks.
- **MKNDX.** DIFCHK is entered with -1, 0 or 1 (it indexes CDUX to CDUZ).
- **Entry state.** MKVACFND's A is a free area's address -1.
- **Marks near the current CDUs (#255).** Half the cases at MARKDIF (with CDUCHKWD positive) and at DIFCHK hold mark CDUs 0 to ±4 bits from CDUX-CDUZ, so the accept path (on to MARKCONT) and the 121 alarm both run: at MARKDIF 305 of 512 cases accept.
- **No marks wanted (#255).** Two thirds of the cases at MARKCONT have MARKSTAT +0, with STATE+2 bit 14 (R21) on or off, EXTVBACT's marking bit and V59FLAG (STATE+5 bit 12) each on or off, so MARKET3's 122 alarm (179 cases) and the P23 calibration display both run.

## Evidence

Hashes: `evidence/cm-sxtmark/SHA256.json`. Bulk files are assets of the `evidence-cm-sxtmark` release.

- **Source map.** `cm-sxtmark-source-map.json` lists every emitted word with its routine: 315 words, none untranslated.
- **Independent assembly comparison.** `make cm-sxtmark-differential`: 512 states per routine (21,504 cases) equal yaAGC in Z, all 2,048 erasable words, channel 12 and the interrupt-enable state. TIME1, TIME2, the CDUs and channel 16 vary in every case; a quarter of the cases put edge values into every operand the file names.
- **Host and sanitizer results.** `make test-cm-sxtmark sanitize-cm-sxtmark` runs the 679 frozen cases: every 32nd, plus a greedy cover of every branch outcome any case reaches: 345 outcomes (`outcome_cover` in `cm-sxtmark-proof.json`).
- **C64 execution results.** `make vice-cm-sxtmark` runs one diagnostic with 74 cases. They reach all 345 outcomes the host cases reach, every gcov branch arc (491; `c64_gcov_branches` in the proof, strict `--cover-check` verified) and all 712 comparison edges (`c64_coverage` in the proof; summary in `cm-sxtmark-c64-outcomes.json`). The diagnostic builds without inlining (`c64_flags`). The log gives the PRG's sha256.
