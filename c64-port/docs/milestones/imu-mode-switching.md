---
id: imu-mode-switching
title: Run Luminary and Comanche IMU mode switching as native routines
date: 2026-10-12T16:00Z
lane: D1
---
Every word of IMU_MODE_SWITCHING_ROUTINES in both programs (660 Luminary099 words, 656 Comanche055 words, in fixed-fixed and banks 07, 04/07 and 37/26) is compiled native C on the control-transfer ABI. The 72 routines per mission cover ZEROICDU, IMUZERO, coarse and fine align, PIPUSE/PIPFREE, gyro torquing (IMUPULSE, STRTGYRO, 8192AUG), the mode ends, the optics and IMU stalls, GOMANUR, R02BOTH and P06/P05, and every point where they resume after an outside call. Channels 11-14 and the interrupt-enable state are part of the comparison. All 63,488 cases equal the original programs in all 2,048 erasable words, the interrupt state and the channels. That is 55,296 runs from each routine to the point where control leaves the file, and 8,192 round trips from every genuine `TC BANKCALL`/`IBNKCALL` of these routines through the native INTER-BANK routines. The yaAGC oracle drops only the engine's gyro, CDU drive and optics device block. The 127,452 strict and sanitizer checks pass, and nine C64 diagnostics pass 1,504 frozen cases in VICE. Round trips stop at SWRETURN/ISWRETRN entered with Q 3 until the INTER-BANK context fix merges. A complete certificate for both files is drafted for the auditor. See `docs/IMU-MODE-SWITCHING.md`.
