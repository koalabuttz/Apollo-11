---
id: gimbal-lock
title: Gimbal lock avoidance as registered native routines
date: 2026-10-12T23:00Z
lane: D1
---
Luminary099 and Comanche055 GIMBAL_LOCK_AVOIDANCE (LOCSKIRT, which KALCMANU reaches by GOTO and BPL) are compiled by `tools/interp_compile.py` into 9 Luminary and 11 Comanche routines of the control-transfer ABI. The entry, the DELCOMP return and every word where the job resumes after CHANG2 are registered keys. All 10,240 cases equal the original programs in yaAGC. The host and sanitizer tests pass 320 frozen cases, and two C64 diagnostics pass 40 cases in VICE, with PRG hashes in the log. New certificates for both files are drafted to replace the legacy ones flagged in issue #129. See `docs/GIMBAL-LOCK.md`.
