---
id: lm-ags-init
title: Translate Luminary AGS_INITIALIZATION as transfer-ABI routines
date: 2026-10-12T19:00Z
lane: F
---
All of Luminary099 AGS_INITIALIZATION (R47, bank 32, 138 words) runs as twenty-two native routines of the control-transfer ABI, fifteen basic and seven interpretive: AGSINIT and its display returns, AGSVCALC's state vector extrapolation through LOADTIME, LEMPREC, CSMPREC and SCALEVEC (VLOAD/MXV/VXSC/VAD/RTB VECSGNAG/LXA/SXA/RVQ) to AGSBUFF and the AGS clock, the AGS downlist delay, the IMU zero (CKSTALL, IMUZERO, IMUSTALL) and AGSEND. All 16,896 cases equal the original program (on yaAGC with the hardware divide) in all 2,048 erasable words and the interrupt-enable bit (strict and ASAN/UBSAN). Every key, instruction, order pair and reachable decision outcome is taken, every word is owned and its operand checked against the listing and rope, and 128 C64 rows chosen by outcome pass in VICE. See `docs/LM-AGS-INIT.md`.
