---
id: kalcmanu-steering
title: Translate KALCMANU_STEERING
date: 2026-10-12T18:00Z
lane: D1
---
All of Luminary099 and Comanche055 KALCMANU_STEERING (159 and 179 words: NEWDELHI, NEWANGL, the CDU increment loop, TIMECHK, MANUSTAT with RATEBIAS, UPDTCALL, MANUSTOP and the Comanche loads) are native C on the control-transfer ABI, as 40 routines. `tools/interp_compile.py` now also compiles INDEX by the hardware's whole-word add, MSU, `TC Q` back into the routine (including TIMECHK's INCR Q returns), and calls that take inline words without returning (POSTJUMP). All 20,480 cases equal the original programs in yaAGC. The strict and sanitizer host tests pass 648 frozen cases, and two C64 diagnostics pass 65 cases in VICE. The host vectors and C64 cases are chosen to reach every branch outcome that any case reaches; the cases include edge values and joint TIME/TM states. Both files have complete certificates drafted for the certificate reviewer. See `docs/KALCMANU-STEERING.md`.
