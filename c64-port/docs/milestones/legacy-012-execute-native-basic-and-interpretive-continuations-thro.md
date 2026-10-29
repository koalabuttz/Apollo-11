---
id: legacy-012-execute-native-basic-and-interpretive-continuations-thro
title: Execute native basic and interpretive continuations through original context exchanges
seq: 12
---

Native C callbacks now enter and exit interpretation, yield at completed operation pairs, restore interpreter payloads, and resume through the original Executive lifecycle. All 140 full original-assembly checkpoints agree; host and sanitizer tests and the PAL C64 diagnostic pass, including masked repeats and an actual raster IRQ admission. This milestone supplies scheduling plumbing; remaining guidance bodies still need translation and binding. See `docs/INTERPRETER_NATIVE.md`.

