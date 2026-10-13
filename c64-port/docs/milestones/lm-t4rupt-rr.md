---
id: lm-t4rupt-rr
title: Run the LM rendezvous radar monitor and GPMATRIX of T4RUPT as native routines
date: 2026-10-12T23:45Z
lane: A
---
Luminary099 T4RUPT_PROGRAM.agc from RRAUTCHK to the end (06,3006-3155) is native as `lm_t4rupt_rr_routines`: the rendezvous radar auto-mode and CDU fail monitor, the angle limit check and reposition, and GPMATRIX. All 3,456 cases equal the original program in yaAGC in all 2,048 erasable words, SUPERBNK and channels 11, 12, 14, 30 and 33. The original executed every instruction of the range in that comparison. The 10,379 strict and sanitizer checks pass, and 216 cases, which run every branch outcome the host cases run, match in VICE. With `t4rupt-core` and `lm-t4rupt-imu` every word of the file is native. See `docs/LM-T4RUPT-RR.md`.
