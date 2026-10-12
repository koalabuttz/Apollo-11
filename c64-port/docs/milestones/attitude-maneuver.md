---
id: attitude-maneuver
title: Translate ATTITUDE_MANEUVER_ROUTINE and ANGLFIND
date: 2026-10-12T19:00Z
lane: D1
---
All of Luminary099 ATTITUDE_MANEUVER_ROUTINE and Comanche055 ANGLFIND (480 and 479 words: KALCMAN3 and SECAD, gimbal lock avoidance, the maneuver axis methods, MXM3, TRANSPOS, CDUTODCM, DELCOMP, DCMTOCDU and VECOFANG) are native C on the control-transfer ABI, as 83 routines. MXV and VXM matrix operands and LXC now compile. All 42,496 cases equal the original programs in yaAGC. The strict and sanitizer host tests pass 705 frozen cases, and 19 C64 diagnostics pass 139 cases in VICE, chosen by branch-outcome cover; three routines on KALCMAN3's main path are too large for one diagnostic and are checked on the host only. Both files have complete certificates drafted for the certificate reviewer. See `docs/ATTITUDE-MANEUVER.md`.
