---
id: keyrupt-uprupt
title: Run the KEYRUPT and UPRUPT handlers of both missions as native routines
date: 2026-10-12T21:30Z
lane: A
---
Every word of KEYRUPT_UPRUPT.agc in Luminary099 and Comanche055 is native: the DSKY key handler KEYRUPT1 with KEYCOM and ACCEPTUP, and the uplink handler UPRUPT with its triple-redundancy test, ERROR RESET and lockout (`lm_keyrupt_routines`, `cm_keyrupt_routines`). Each ends where the original continues: at LODSAMPT, at NOVAC to start CHARIN, or at RESUME. Channel 15, the key code, joins `agc_hardware`. All 3,072 cases equal the original programs in yaAGC in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channel 11, and the original executed every reachable instruction of both files in that comparison. The 9,240 strict and sanitizer checks pass, and every sixteenth case matches in VICE. Complete certificates are drafted for both files. See `docs/KEYRUPT-UPRUPT.md`.
