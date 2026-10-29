---
id: lm-p12
title: Translate Luminary P12
date: 2026-10-13T06:45Z
lane: F
---
Luminary099 P12.agc (powered ascent, and the P12INIT, COMMINIT and GUIDINIT set-up that P70 and P71 share) is native C on the control-transfer ABI as 91 routines compiled by `tools/interp_compile.py`, every word of the file. Every word after a DANZIG is a routine of its own (`resume_keys`). All 46,592 cases equal the original program in yaAGC, none refused. The host and sanitizer tests pass 1,456 frozen cases, and 7 C64 diagnostics pass 112 cases in VICE that take all 1,015 branch arcs the host cases take. The certificate is drafted for review. See `docs/LM-P12.md`.
