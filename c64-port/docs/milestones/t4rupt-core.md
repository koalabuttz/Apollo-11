---
id: t4rupt-core
title: Run the T4RUPT display and dispatch core of both missions as native routines
date: 2026-10-12T22:00Z
lane: A
---
T4RUPT_PROGRAM.agc's core is native in both missions: the 120 ms phase (NORMT4, CDRVE, DSPOUT and the DSPTAB scan to channel 10, HANG20, SETTIME4, the PRO key check), the T4JUMP dispatch to the monitors, and the 20 ms quick display (QUIKDSP, QUIKRUPT, NODSPY, SYNCT4, QUIKOFF) (`lm_t4rupt_core_routines`, `cm_t4rupt_core_routines`). Channels 10 and 32 join `agc_hardware`. All 4,416 cases equal the original programs in yaAGC in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channel 10. The original executed every reachable instruction of the core in that comparison. The 11,400 strict and sanitizer checks pass, and every sixteenth case matches in VICE. The monitors from IMUMON on remain; the file stays partial. See `docs/T4RUPT-CORE.md`.
