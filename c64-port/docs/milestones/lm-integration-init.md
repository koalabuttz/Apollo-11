---
id: lm-integration-init
title: Translate Luminary INTEGRATION_INITIALIZATION
date: 2026-10-12T20:00Z
lane: B
---
All of Luminary099 INTEGRATION_INITIALIZATION (623 words) is native C on the control-transfer ABI, as 120 routines compiled by `tools/interp_compile.py`. The state-vector moves behind RTB run as `lm_integration_move`. This covers every entry rendezvous navigation needs: INTSTALL, SETIFLGS, INTEGRV, INTWAKE/INTWAKE0, LEMCONIC/CSMCONIC and LEMPREC/CSMPREC. All 30,720 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 240 frozen cases, and 8 C64 diagnostics pass 240 cases in VICE. A complete certificate is drafted for the certificate reviewer. See `docs/INTEGRATION-INITIALIZATION.md`.
