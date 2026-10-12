---
id: imu-compensation
title: Run Luminary and Comanche IMU compensation as native routines
date: 2026-10-12T14:40Z
lane: D1
---
Every word of IMU_COMPENSATION_PACKAGE in both programs (282 Luminary099 words, 247 Comanche055 words, bank 06) is compiled native C on the control-transfer ABI: 1/PIPA, GCOMPZER, NBDONLY, LASTBIAS, 1/GYRO and GCOMP1, and the points where they resume after NOVAC, BANKCALL and IBNKCALL, as 20 routines. All 13,824 cases equal the original programs in all 2,048 erasable words and the interrupt-enable state: 10,240 runs from each routine to the point where control leaves the file, and 3,584 round trips from every genuine `TC BANKCALL` of 1/PIPA and GCOMPZER through the native INTER-BANK routines, Luminary NOVAC and SWRETURN back to the caller. The 38,450 strict and sanitizer checks pass, and three C64 diagnostics pass 373 frozen cases in VICE. A complete certificate for both files is drafted for the auditor. See `docs/IMU-COMPENSATION.md`.
