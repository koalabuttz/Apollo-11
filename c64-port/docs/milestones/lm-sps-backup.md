---
id: lm-sps-backup
title: Translate Luminary SPS_BACK-UP_RCS_CONTROL on the transfer ABI
date: 2026-10-14T00:00Z
lane: F
---
Luminary099 SPS_BACK-UP_RCS_CONTROL.agc (SPSRCS, the docked RCS jet law the P and Q,R axis autopilots call through IBNKCALL) is native C on the control-transfer ABI as 1 routine compiled by `tools/interp_compile.py`, every word of the file (82). The earlier hand-written `src/lm_sps_rcs.c` was not a transfer-ABI routine, so its certificate failed `tools/cert_precheck.py` (SPSRCS reached from other files but not registered) and was withdrawn (#85); this translation registers SPSRCS at 21,3727. All 512 cases equal the original program in yaAGC, with FIXLOC and NEWJOB free at entry (`any_fixloc`, `any_newjob`: SPSRCS runs in T5RUPT over any job), Q = ISWRETRN in half of the cases, and E and EDOT wide enough that the coast zone's `BZMF POSTHRST` takes both outcomes (review #386); the host and sanitizer tests pass 36 frozen cases, and 1 C64 diagnostic passes 9 cases in VICE that take every branch arc the host cases take. The certificate is drafted as pending, awaiting the certificate reviewer. See `docs/LM-SPS-BACKUP.md`.
