---
id: legacy-019-fix-source-exact-waitlist-positive-zero-control-flow-and
title: Fix source-exact WAITLIST positive-zero control flow and final-slot insertion
seq: 19
---

WAITLIST now skips interval additions on early CCS positive zero and follows the final positive-zero insertion route instead of raising a false full-queue alarm. The 256-case full-word regression detects 688 previous field errors and passes after the fix; the original 196-case suite remains unchanged and passes. Host, sanitizer and all 256 C64 states agree with original assembly. See `docs/WAITLIST_ZERO.md`.

