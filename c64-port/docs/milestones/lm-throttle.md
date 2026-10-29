---
id: lm-throttle
title: Translate Luminary THROTTLE_CONTROL_ROUTINES and FASTCHNG as transfer-ABI routines
date: 2026-10-12T15:10Z
lane: F
---
All of Luminary099 THROTTLE_CONTROL_ROUTINES (bank 31, 130 words: THROTTLE, AFDUMP, the critical-thrust bands, DOIT with THRUST and the channel 14 thrust drive, the FWEIGHT compensation, FLATOUT, WHATOUT, MASSMULT) and the LUNAR_LANDING_GUIDANCE_EQUATIONS FASTCHNG it calls run as nine native routines of the control-transfer ABI, with each source instruction one yaAGC-qualified `agc_image_ops` call. All 16,384 cases from THROTTLE, THROTTLE +3, FLATOUT and WHATOUT to the caller's return equal the original program in channel 14 and all 2,048 erasable words (strict and ASAN/UBSAN), every reachable branch is taken, every assembled word maps to its native owner after a fresh yaYUL assembly, and 256 C64 rows chosen to take every entry, outcome and branch the host cases take pass in VICE. See `docs/LM-THROTTLE.md`.
