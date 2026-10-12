---
id: attitude-maneuver
title: Translate ATTITUDE_MANEUVER_ROUTINE and ANGLFIND
date: 2026-10-12T19:00Z
lane: D1
---
All of Luminary099 ATTITUDE_MANEUVER_ROUTINE and Comanche055 ANGLFIND (480 and 479 words: KALCMAN3 and SECAD, gimbal lock avoidance, the maneuver axis methods, MXM3, TRANSPOS, CDUTODCM, DELCOMP, DCMTOCDU and VECOFANG) are native C on the control-transfer ABI, as 83 routines. MXV and VXM matrix operands and LXC now compile. All 42,496 cases equal the original programs in yaAGC. The strict and sanitizer host tests pass 708 frozen cases, and 13 C64 diagnostics pass 145 cases in VICE, chosen by branch-outcome cover. With the image under the KERNAL ROM every routine runs on the C64; three outcomes in the shared interpreter bodies are reached only by whole-maneuver runs too large for C64 RAM. Both files have complete certificates drafted for the certificate reviewer. See `docs/ATTITUDE-MANEUVER.md`.
