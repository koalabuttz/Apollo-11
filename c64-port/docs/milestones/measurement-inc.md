---
id: measurement-inc
title: Translate MEASUREMENT_INCORPORATION
date: 2026-10-14T04:40Z
lane: D2
---
All of Luminary099 and Comanche055 MEASUREMENT_INCORPORATION (384 and 385 words: INCORP1, NEWZCOMP and INCORP2, the rendezvous navigation filter's measurement update that P20-P25 run for every mark) are native C on the control-transfer ABI, as 160 routines per mission compiled by tools/interp_compile.py, with native RTB TPMODE and the MOVE* state-vector copies (the new src/cm_integration_moves.c for Comanche). The differential gained 'flow_from': every routine but the entries starts from the state the original program reaches there from INCORP1 or INCORP2, run on through the outside code it calls, with QPRET pinned to a fixed CADR so interpretive entries return where the run ends. All 163,840 cases equal the original programs in yaAGC, with all 314 resume points flowed. The strict and sanitizer host tests pass 2,560 frozen cases, and 19 C64 diagnostics pass 353 cases in VICE that take all 2,221 host branch outcomes. Both files have pending complete certificates for the certificate reviewer. See `docs/MEASUREMENT-INCORPORATION.md`.
