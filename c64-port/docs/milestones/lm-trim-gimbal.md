---
id: lm-trim-gimbal
title: Translate Luminary TRIM_GIMBAL_CONTROL_SYSTEM as transfer-ABI routines
date: 2026-10-12T19:00Z
lane: F
---
All of Luminary099 TRIM_GIMBAL_CONTROL_SYSTEM (banks 21 and 16, 388 words) runs as four native routines of the control-transfer ABI, with channel 12 in `lm_flight_io`. The routines are GTS (the trim gimbal control law, with GTSQRT and ROOTCYCL compiled inline), ACDT+C12, TIMEGMBL and its IBNKCALL return. 16,098 of 16,384 cases equal the original program in the outcome, channel 12 and all 2,048 erasable words (strict and ASAN/UBSAN). The remaining 286 GTS cases take the zero root with an INDEX SHFTFLAG outside BIT15-BIT2, where the original executes an arbitrary instruction; GTS refuses them with the image unchanged. Each case runs from its entry to CLOSEOUT, the caller's return, or ACDT+C12 reached through the native IBNKCALL. Every routine and reachable branch is taken, every assembled word maps to its native owner after a fresh yaYUL assembly, and every thirty-second case (512) passes in VICE. See `docs/LM-TRIM-GIMBAL.md`.
