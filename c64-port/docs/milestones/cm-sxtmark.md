---
id: cm-sxtmark
title: CM sextant mark system as registered native routines
date: 2026-10-14T10:00Z
lane: D1
---
Comanche055 SXTMARK (SXTMARK, MARKRUPT, MARKDIF/DIFCHK, MARKCONT, MKREJECT, MKVB51, ENTANSWR and the rest, 315 words in banks 07 and 10) is compiled whole by `tools/basic_compile.py` into 42 routines of the control-transfer ABI. All 21,504 cases equal the original program in yaAGC, from flight-realistic VAC, MARKSTAT, QPRET and MKNDX states, with channel 16 (the mark buttons) a latched input. The host and sanitizer tests pass 679 frozen cases, and one C64 diagnostic passes 74 cases in VICE that reach all 345 branch outcomes and all 491 gcov arcs the host cases reach. A complete certificate is drafted for the reviewer. See `docs/CM-SXTMARK.md`.
