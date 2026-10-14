---
id: lm-rcsmon
title: Translate Luminary RCS_FAILURE_MONITOR on the transfer ABI
date: 2026-10-13T20:30Z
lane: F
---
Luminary099 RCS_FAILURE_MONITOR.agc (RCSMON, the T4RUPT channel-32 jet-failure check) is native C on the control-transfer ABI as 2 routines compiled by `tools/interp_compile.py`, every word of the file, so the whole-program flight run reaches it. All 1,024 cases equal the original program in yaAGC; the host and sanitizer tests pass 72 frozen cases, and 1 C64 diagnostic passes 17 cases in VICE that take all 220 branch arcs the host cases take. The domain includes NEWJOB -0 and any FIXLOC (#285). The certificate is drafted for review. See `docs/LM-RCSMON.md`.
