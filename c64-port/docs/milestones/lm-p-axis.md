---
id: lm-p-axis
title: Translate Luminary P-AXIS_RCS_AUTOPILOT as transfer-ABI routines
date: 2026-10-12T17:00Z
lane: F
---
All of Luminary099 P-AXIS_RCS_AUTOPILOT (bank 16, 790 words) runs as fifteen native routines of the control-transfer ABI, with channels 12, 13 and 31 and the interrupt-enable bit in `lm_flight_io` (which gains channel 13). The routines are the T5 lead-in PAXIS, CHEKBITS's two returns, ACDT+C12's return, SUPERJOB, BACKP (rate derivation, trap filters, AOS, gimbal timers, translation, manual and rate command, the P-axis RCS law and jet selection) and its eight returns from calls that leave the file, and CALCPERR. 16,235 of 16,384 cases equal the original program in the outcome, channels 12 and 13, the interrupt-enable bit and all 2,048 erasable words (strict and ASAN/UBSAN); the other 149 are refused with the image unchanged (a 16,3403 entry with ROTINDEX outside 0-1, OLDPMIN -0, or a CALCPERR return that is not a TC into fixed memory). DV is `ai_dv` (the AGC's divide), and the oracle runs every DV through yaAGC's SimulateDV hardware model. Every routine and reachable branch is taken, every exit is taken, every assembled word maps to its native owner after a fresh yaYUL assembly, and 128 C64 rows chosen to take every entry, outcome and branch the host cases take pass in VICE. See `docs/LM-P-AXIS.md`.
