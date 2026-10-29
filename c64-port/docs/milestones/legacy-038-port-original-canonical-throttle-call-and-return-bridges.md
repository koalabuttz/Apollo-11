---
id: legacy-038-port-original-canonical-throttle-call-and-return-bridges
title: Port original canonical throttle call and return bridges
seq: 38
---

The actual source EXIT/BANKCALL and THDUMP/SWRETURN/INTPRET paths match all 8,192 complete erasable images. Strict and all-state sanitizer comparisons and 64 whole-image C64 checks pass, with unsupported-boundary no-mutation guards. See `docs/THROTTLE_BRIDGE.md`.

