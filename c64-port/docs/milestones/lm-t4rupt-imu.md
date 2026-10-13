---
id: lm-t4rupt-imu
title: Run the LM IMU monitor of T4RUPT as native routines
date: 2026-10-12T23:00Z
lane: A
---
Luminary099 T4RUPT_PROGRAM.agc from IMUMON to 90SECS (06,2172-3005) is native as `lm_t4rupt_imu_routines`: the channel 30 and channel 33 monitors with their bit-change handlers (temperature, turn-on, IMU and CDU fail, cage, operate, PIPA fail, rate alarms), SETISSW, the gimbal lock check, CAGESUB and LAMPTEST. Channels 12, 14, 30 and 33 join `agc_hardware`. All 4,736 cases equal the original program in yaAGC in all 2,048 erasable words, SUPERBNK and channels 11, 12, 14, 30 and 33. The original executed every instruction of the range in that comparison. The 9,582 strict and sanitizer checks pass, and 300 cases, which run every branch outcome the host cases run, match in VICE. RRAUTCHK and DAPT4S are `lm-t4rupt-rr`. See `docs/LM-T4RUPT-IMU.md`.
