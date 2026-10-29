---
id: lm-imu-performance
title: Run the Luminary IMU performance tests (P07) as native routines
date: 2026-10-12T21:00Z
lane: D1
---
Every word of Luminary099 IMU_PERFORMANCE_TEST_2 and IMU_PERFORMANCE_TESTS_4 (585 words, bank 37) is compiled native C on the control-transfer ABI: 43 basic and 125 interpretive routines covering P07's gyro drift, PIPA and torque tests, the erasable return lists, ERTHRVSE and EARTHR, ESTIMS and the ALFLT/NORMLOP filter loop with its matrix orders. The keys include 1STO2S's EXIT return, every word a job switch can resume at, FINETIME +1 and the two idle points (CHECKG1 and FINETIME +1), which end runs as continuations (issue #88). Of 64,512 cases, the 56,580 the native code runs equal the original program in all 2,048 erasable words and the interrupt state; the 7,932 refused are outside the documented domain and leave the image unchanged. The 129,258 strict, part-dispatch and sanitizer checks pass. Seven C64 diagnostics run 657 rows in VICE, as chained steps split at the keys each part leaves out (ALFLT split at ALKLP); those rows take every branch outcome the host stream takes over every linked native source. A complete certificate for both files is drafted for the reviewer. See `docs/LM-IMU-PERFORMANCE.md`.
