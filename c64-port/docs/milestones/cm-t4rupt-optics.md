---
id: cm-t4rupt-optics
title: Run the CM optics monitor and drive of T4RUPT as native routines
date: 2026-10-13T15:40Z
lane: A
---
Comanche055 T4RUPT_PROGRAM.agc from OPTMON to the end (06,2765-3262) and OPTDRIVE (10,2000-2202) are native as `cm_t4rupt_optics_basic`, compiled by `tools/basic_compile.py`. The milestone covers the optics monitor, the zero-optics sequence and the optics CDU drive. All 4,096 cases equal the original program in yaAGC in all 2,048 erasable words and channels 12, 14, 30 and 33. The 274 frozen cases pass on the host and under the sanitizer. In VICE, 46 cases pass and take every branch arc the host cases take. With `t4rupt-core` and `cm-t4rupt-imu`, every word of the file is native. See `docs/CM-T4RUPT-OPTICS.md`.
