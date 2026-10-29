---
id: resident-kernel
title: Thin resident Executive and Waitlist on the canonical image
date: 2026-10-13T04:15Z
lane: A
---
One thin resident Executive and Waitlist for both missions (`src/resident_kernel_body.inc`, `src/lm_kernel.c`, `src/cm_kernel.c`) translates EXECUTIVE.agc and WAITLIST.agc instruction by instruction onto the canonical image, with no gather/commit view and no domain refusals, as drop-in routine tables for the certified ones (`include/resident_kernel.h`). On every case of the six certified service milestones, 23,250 Luminary099 and 18,786 Comanche055 cases, each run equals the original program in yaAGC in all 2,048 erasable words and the device words, and equals the certified routines' final image word for word. The strict and sanitizer builds pass, and 400 LM and 329 CM rows, which take every kernel branch arc the host cases take, pass in VICE with the kernel on the image at $7000. The kernel links in 10.3 KB (LM) and 10.0 KB (CM) at -Oz, against 54-56 KB for the certified routines. See `docs/RESIDENT-KERNEL.md`.
