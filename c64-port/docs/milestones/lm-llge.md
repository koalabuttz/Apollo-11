---
id: lm-llge
title: Translate Luminary LUNAR_LANDING_GUIDANCE_EQUATIONS
date: 2026-10-13T20:00Z
lane: F
---
Luminary099 LUNAR_LANDING_GUIDANCE_EQUATIONS.agc (the P63 to P67 landing guidance: the guidance pass, P64 redesignation, the time-to-go and target logic with ROOTPSRS, the P65 to P67 starts, RODTASK and the guidance displays) is native C on the control-transfer ABI: all 991 words, as 249 routines compiled by `tools/interp_compile.py` on the shared I/O channel model (READ CHAN30/31, WOR CHAN13). All 127,488 cases equal the original program in yaAGC, channels 13, 30 and 31 included. The host and sanitizer tests pass 3,984 frozen cases, and 47 C64 diagnostics pass 384 cases in VICE (32 truncated), taking every host branch arc. The file is complete; the second reviewer's record is `docs/audits/rev2-lm-llge.md`. See `docs/LM-LLGE.md`.
