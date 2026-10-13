---
id: cm-p11
title: Translate Comanche055 P11
date: 2026-10-13T09:30Z
lane: A2
---
All of Comanche055 P11 (535 words: the Earth orbit insertion monitor, its attitude error display and the boost steering takeover) is native C on the control-transfer ABI, as 148 routines compiled whole-file by `tools/basic_compile.py` and `tools/interp_compile.py`. All 37,888 cases equal the original program in yaAGC; the host tests pass 592 frozen cases and eight C64 diagnostics pass 202 cases in VICE. See `docs/CM-P11.md`.
