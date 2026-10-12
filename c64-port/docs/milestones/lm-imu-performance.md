---
id: lm-imu-performance
title: Run the Luminary IMU performance tests (P07) as native routines
date: 2026-10-12T21:00Z
lane: D1
---
Every word of Luminary099 IMU_PERFORMANCE_TEST_2 and IMU_PERFORMANCE_TESTS_4 (585 words, bank 37) is compiled native C on the control-transfer ABI: 40 basic and 14 interpretive routines covering P07's gyro drift, PIPA and torque tests, the erasable return lists, ERTHRVSE and EARTHR, ESTIMS and the ALFLT/NORMLOP filter loop with its matrix orders. Of 27,648 cases, the 26,689 the native code runs equal the original program in all 2,048 erasable words and the interrupt state; the 959 refused are outside the documented domain and leave the image unchanged. The 55,414 strict, part-dispatch and sanitizer checks pass, and five C64 diagnostics pass 685 frozen cases in VICE; 63 sampled cases are host-only because they reach the ALFLT loop, which does not fit in C64 RAM, or need code from more than one part. A complete certificate for both files is drafted for the reviewer. See `docs/LM-IMU-PERFORMANCE.md`.
