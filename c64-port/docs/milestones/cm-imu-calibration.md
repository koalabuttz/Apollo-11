---
id: cm-imu-calibration
title: Translate Comanche055 IMU_CALIBRATION_AND_ALIGNMENT
date: 2026-10-12T23:00Z
lane: D1
---
All of Comanche055 IMU_CALIBRATION_AND_ALIGNMENT (1,077 words: the P07 IMU performance tests, the optimum prelaunch alignment and calibration filter, gyrocompassing, earth-rate compensation, P03 optical verification, FINETIME and CHKCOMED) is native C on the control-transfer ABI, as 398 routines compiled by `tools/interp_compile.py`. The compiler now reads input channels 3, 4 and 030-033 that the caller latches (`channel_inputs`, through `AGC_CHANNEL`), ends runs at spin waits (`idle`) and at the erasable program the test loads (`erasable_calls`), and guards indexed operands (`index_guard`). All 203,776 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 534 frozen cases, chosen to reach every branch outcome any case reaches, and 23 C64 diagnostics pass 476 cases in VICE that reach all but three of them. A complete certificate is drafted for the certificate reviewer. See `docs/IMU-CALIBRATION.md`.
