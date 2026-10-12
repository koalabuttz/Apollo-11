---
id: cm-t4rupt-imu
title: Run the CM IMU monitor of T4RUPT as native routines
date: 2026-10-12T23:30Z
lane: A
---
Comanche055 T4RUPT_PROGRAM.agc from IMUMON to 90SECS (06,2174-2764), with ZOPFIN3 that LAMPTEST shares, is native as `cm_t4rupt_imu_routines`: the channel 30 and channel 33 monitors with their bit-change handlers, SETISSW, the gimbal lock check, CAGESUB and LAMPTEST. All 4,736 cases equal the original program in yaAGC in all 2,048 erasable words, SUPERBNK and channels 11, 12, 14, 30 and 33. The original executed every instruction of the range in that comparison. The 9,577 strict and sanitizer checks pass, and 300 cases, which run every branch outcome the host cases run, match in VICE. OPTMON remains; the file stays partial. See `docs/CM-T4RUPT-IMU.md`.
