---
id: findcduw
title: Translate Luminary099 FINDCDUW--GUIDAP_INTERFACE
date: 2026-10-13T14:00Z
lane: A2
---
All of Luminary099 FINDCDUW--GUIDAP_INTERFACE (441 words, 30,3104-3774: INITCDUW, FINDCDUW with its PNGCS-auto branch PAUTNO, the CDU-window filter FLTRSUB, CDUWXFR and TCQCDUW, NB2CDUSP with DVBYCOSM, ARCTRGSP, SPARCSIN, LIMITSUB, ONESTO2S and the two alarm exits) is native C on the control-transfer ABI, as 75 routines compiled by `tools/interp_compile.py` (lane A2), every word after a DANZIG a CHANG2 resume routine. PAUTNO's two RTBs run through existing native bodies: NORMUNIT is `rtb_vector`'s NORMUNIT front then `rtb_unit`'s UNIT+1 phases, and QUICTRIG is `lm_powered_quick`; its RAND CHAN30/CHAN31 read the input channels the caller latches (`channel_inputs`). All 19,200 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 1,200 frozen cases, and fourteen C64 diagnostics pass 146 cases in VICE that take all 1,974 branch arcs the host cases take. See `docs/LM-FINDCDUW.md`.
