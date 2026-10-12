---
id: lm-t6rupt
title: Run the Luminary T6RUPT jet-timing programs as native routines
date: 2026-10-12T19:30Z
lane: A
---
Every word of Luminary099 T6-RUPT_PROGRAMS.agc is native: DOT6RUPT, T6JOBCHK and T6JOB, which stop and restart the TIME6 clock and fire the next jets, and the WRITEP, WRITEU and WRITEV writers the DAP calls. With them come the T6RUPT vector and the RESUME, NOQRSM and NOQBRSM interrupt exits (`lm_t6rupt_routines`). The jet channels 5 and 6 and channel 13 join `agc_hardware`. All 3,840 cases equal the original Luminary099 program in yaAGC in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channels 5, 6 and 13. The original executed every instruction of the file in that comparison except the self-loop a negative TIME6 would reach, which nothing produces. The 7,705 strict and sanitizer checks pass, and every sixteenth case matches in VICE. A complete certificate is drafted for T6-RUPT_PROGRAMS.agc. See `docs/LM-T6RUPT.md`.
