---
id: cm-rcs-dap
title: Translate Comanche RCS-CSM_DIGITAL_AUTOPILOT (CM RCS DAP T5 cycle, needles, hand controller)
date: 2026-10-12T19:30Z
lane: G
---
Native C for all of Comanche055 RCS-CSM_DIGITAL_AUTOPILOT (bank 21 `2105`-`3247`, 611 words). This covers the RCSATT T5 cycle and its REDORCS restart, from initialisation through the hand controller, manual rates and free drift. It also covers the subroutines other files call: NEEDLER, NEEDLE11, NEEDLES, ZEROJET and STICKCHK. Every entry runs to RESUME, IBNKCALL, NOVAC, the MYSUBS calls, the AUTOMATIC_MANEUVERS transfers or the caller's return. The final image and channels 12/13/14 equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN. All 611 words, 87 fixed constants, the 2CADRs, the INDEX tables, the TC Q dispatch and the frontier returns are checked against the listing and the rope. A C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-RCS-DAP.md`.
