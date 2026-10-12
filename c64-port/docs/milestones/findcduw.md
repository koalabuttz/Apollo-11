---
id: findcduw
title: Translate Luminary099 FINDCDUW--GUIDAP_INTERFACE
date: 2026-10-12T22:00Z
lane: A
---
All of Luminary099 FINDCDUW--GUIDAP_INTERFACE (441 words, 30,3104-3774: INITCDUW, FINDCDUW with its PNGCS-auto branch PAUTNO, the CDU-window filter FLTRSUB, CDUWXFR and TCQCDUW, NB2CDUSP with DVBYCOSM, ARCTRGSP, SPARCSIN, LIMITSUB, ONESTO2S and the two alarm exits) is native C on the control-transfer ABI, as 45 routines compiled by `tools/interp_compile.py` (lane A2). PAUTNO's two RTBs run through existing native bodies: NORMUNIT is `rtb_vector`'s NORMUNIT front then UNIT+1 of the shared interpreter library (`lmi_unit1`, split out of `lmi_unit`), and QUICTRIG is `lm_powered_quick`. The compiler gained fixed-memory INDEX, channel 30/31 READ and RAND, saved-return `TC` through an erasable word and BOVB into the file. All 23,040 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 1,440 frozen cases, and ten C64 diagnostics pass 45 cases in VICE. See `docs/LM-FINDCDUW.md`.
