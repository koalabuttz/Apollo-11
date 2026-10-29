---
id: lm-tjet-law
title: Translate Luminary TJET_LAW as a transfer-ABI routine
date: 2026-10-12T16:30Z
lane: F
---
All of Luminary099 TJET_LAW (bank 17, 314 words: the RCS jet-firing time law, with the phase-plane zones 1-5, the rough laws, Z123COMP and RUFSETUP) runs as one native TJETLAW routine of the control-transfer ABI, with each source instruction one yaAGC-qualified `agc_image_ops` call. All 16,384 cases for the P, U and V axes, from TJETLAW to the caller's return, equal the original program in all 2,048 erasable words (strict and ASAN/UBSAN). Every reachable branch is taken, every assembled word maps to its native owner after a fresh yaYUL assembly, and 256 C64 rows chosen to take every entry, outcome and branch the host cases take pass in VICE. See `docs/LM-TJET-LAW.md`.
