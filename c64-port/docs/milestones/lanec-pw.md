---
id: lanec-pw
title: Run the earlier Pinball modules through the Lane C harness
date: 2026-10-12T22:30Z
lane: C
---
Every earlier native Pinball module of Luminary099 and Comanche055 (the CHARIN family, decimal and octal displays, scale factors, loads, monitor, NVSUB and verb/noun machinery, lamp test, operator requests) now runs through the Lane C harness in 92 `pw-*` suites, drawing states from each module's own generator over its whole domain: 108,043 of 108,063 cases' complete state (all 2,048 erasable words, central registers, I/O channels 0-77, superbank and IRQ) is identical to the unchanged original programs in Virtual AGC, run to the point where the module stops, in strict and ASan/UBSan builds, and the other 20 are refused unchanged. These are the cases the Pinball transfer ABI and full-file certificates build on. See `docs/LANEC-PW.md`.
