---
id: lm-llge
title: Translate Luminary LUNAR_LANDING_GUIDANCE_EQUATIONS
date: 2026-10-13T20:00Z
lane: F
---
Luminary099 LUNAR_LANDING_GUIDANCE_EQUATIONS.agc (the P63 to P67 landing guidance: the guidance pass, P64 redesignation, the time-to-go and target logic with ROOTPSRS, the P65 to P67 starts, RODTASK and the guidance displays) is native C on the control-transfer ABI as 258 routines compiled by `tools/interp_compile.py`, except the 15 words that read channels 30 and 31 or write channel 13, which wait for an I/O channel model. The compiler gained indexed TCF/BZMF tables, a double INDEX into a fixed table, INDEX into an erasable table, TCF INTPRET through a subroutine, callee returns past the TC, never-returning callees, `TC A`, `INCR BBANK` and a 16-bit-safe INDEX carry (a C64-only fault the LLGE diagnostics found). All 118,017 compared cases (of 132,096; the 14,079 that reach a channel instruction are listed, not compared) equal the original program in yaAGC. The host and sanitizer tests pass 3,690 frozen cases, and 36 C64 diagnostics pass 313 cases in VICE. The file stays partial until the channel words are translated. See `docs/LM-LLGE.md`.
