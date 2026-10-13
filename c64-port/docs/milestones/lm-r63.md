---
id: lm-r63
title: Translate Luminary R63
date: 2026-10-13T06:00Z
lane: F
---
Luminary099 R63.agc (Verb 89, the rendezvous final attitude) is native C on the control-transfer ABI as 30 routines compiled by `tools/interp_compile.py`, every word of the file. Every word after a DANZIG is a routine of its own (`resume_keys`). All 15,360 cases equal the original program in yaAGC, none refused. The host and sanitizer tests pass 960 frozen cases, and one C64 diagnostic passes 34 cases in VICE that take all 408 branch arcs the host cases take. The certificate is drafted for review. See `docs/LM-R63.md`.
