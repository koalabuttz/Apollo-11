---
id: lm-lunar-landing
title: Translate Luminary THE_LUNAR_LANDING as transfer-ABI routines
date: 2026-10-12T19:30Z
lane: F
---
All of Luminary099 THE_LUNAR_LANDING (P63 in bank 32 and P68's LANDJUNK in bank 31, 223 words) runs as thirty-one native routines of the control-transfer ABI, twenty-one basic and ten interpretive: P63's set-up and flags, the ignition algorithm (IGNALG, IGNALOOP and the DDUM iteration with its pushdown arithmetic, DDUMGOOD's TIG and out-of-plane distance), the astronaut clock, the KALCMANU set-up through PFLITEDB and R60LEM, the LR antenna test and the hand-off to BURNBABY, and P68's landing-site set-up. All 15,872 cases equal the original program (on yaAGC with the hardware divide) in all 2,048 erasable words, channels 12 and 33 and the interrupt-enable bit (strict and ASAN/UBSAN). Every key, instruction, order pair and reachable decision outcome is taken, every word is owned and its operand checked against the listing and rope, and 120 C64 rows chosen by outcome pass in VICE in two PRGs. See `docs/LM-LUNAR-LANDING.md`.
