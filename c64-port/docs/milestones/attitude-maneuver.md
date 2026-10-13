---
id: attitude-maneuver
title: Translate ATTITUDE_MANEUVER_ROUTINE and ANGLFIND
date: 2026-10-12T19:00Z
lane: D1
---
All of Luminary099 ATTITUDE_MANEUVER_ROUTINE and Comanche055 ANGLFIND (480 and 479 words: KALCMAN3 and SECAD, gimbal lock avoidance, the maneuver axis methods, MXM3, TRANSPOS, CDUTODCM, DELCOMP, DCMTOCDU and VECOFANG) are native C on the control-transfer ABI, as 83 routines. MXV and VXM matrix operands and LXC now compile. All 42,496 cases equal the original programs in yaAGC. The strict and sanitizer host tests pass 1,170 frozen cases, and 22 C64 diagnostics pass 556 cases in VICE that reach all 3,153 host branch outcomes; six of them are segment rows that stop at the next CHANG2 resume point (spec `c64_segments`). Both files have accepted complete certificates. See `docs/ATTITUDE-MANEUVER.md`.
