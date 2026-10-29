---
id: legacy-018-execute-bounded-native-disk-overlays-with-canonical-eras
title: Execute bounded native disk overlays with canonical erasable storage on the C64
seq: 18
---

A resident Executive and loader execute separately linked native C payloads from a D64 image, swap and reload them after ordinary C returns, reset transient BSS, and preserve the 2,048-word canonical source image and platform mappings. Host bounds and error checks, sanitizers and the stock PAL C64 disk-loading proof pass. The demonstrated payloads test the ABI; full mission module allocation, media handling and guidance integration remain unfinished. See `docs/OVERLAYS.md`.

