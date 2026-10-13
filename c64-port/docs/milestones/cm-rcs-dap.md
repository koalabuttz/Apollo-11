---
id: cm-rcs-dap
title: Translate Comanche RCS-CSM DIGITAL AUTOPILOT
date: 2026-10-13
lane: G
---
All of Comanche055 RCS-CSM_DIGITAL_AUTOPILOT (611 words) is native C on the control-transfer ABI, as 71 routines compiled whole-file by `tools/basic_compile.py`. All 4,352 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 136 frozen cases, and 2 C64 diagnostics pass 43 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-RCS-DAP.md`.
