---
id: legacy-116-adopt-source-selected-native-continuations-without-repea
title: Adopt source-selected native continuations without repeating scheduling
seq: 116
---

Verified scheduler adoption preserves original source state and queued interrupts. Host and sanitizer checks cover 384 full states and 11 rejection cases; stock-ROM C64 proofs cover 256 full states. The shared compiled guidance continuation supports qualified source LOC+1 routing. This certifies these scheduler seams, not the complete Executive or either application. See `docs/NATIVE-SOURCE-DISPATCH.md`.

