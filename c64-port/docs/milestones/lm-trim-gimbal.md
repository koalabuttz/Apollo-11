---
id: lm-trim-gimbal
title: Translate Luminary TRIM_GIMBAL_CONTROL_SYSTEM as transfer-ABI routines
date: 2026-10-12T19:00Z
lane: F
---
All of Luminary099 TRIM_GIMBAL_CONTROL_SYSTEM (banks 21 and 16, 388 words) runs as four native routines of the control-transfer ABI, with channel 12 in `lm_flight_io`. The routines are GTS (the trim gimbal control law, with GTSQRT and ROOTCYCL compiled inline), ACDT+C12, TIMEGMBL and its IBNKCALL return. 16,211 of 16,384 cases equal the original program in the outcome, channel 12 and all 2,048 erasable words (strict and ASAN/UBSAN). These include 138 zero-root DOSHIFT passes, where the INDEX SHFTFLAG'd CA BIT15 reads 04735 + ACENTRAL/2 (erasable memory, or a generated bank 21 and fixed-fixed table). The remaining 173 GTS cases index CA BIT15 into another order, which needs an offset acceleration above 8.6 deg/s^2; GTS refuses them with the image unchanged. DV is `ai_dv`, and the oracle runs every DV through yaAGC's SimulateDV hardware model. Each case runs from its entry to CLOSEOUT, the caller's return, or ACDT+C12 reached through the native IBNKCALL. Every routine and reachable branch is taken, every assembled word maps to its native owner after a fresh yaYUL assembly, and 256 C64 rows chosen to take every entry, outcome and branch the host cases take pass in VICE. See `docs/LM-TRIM-GIMBAL.md`.
