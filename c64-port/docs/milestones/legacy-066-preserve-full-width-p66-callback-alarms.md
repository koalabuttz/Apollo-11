---
id: legacy-066-preserve-full-width-p66-callback-alarms
title: Preserve full-width P66 callback alarms
seq: 66
---

Native callback status now retains full16-bit alarm and fault values instead of the target short enum. Dedicated strict/sanitizer and10 actualC64 enabled/masked-interrupt transport checks preserve01302,01202,8003,ffff and native failure4, commit the qualified source boundary and stop without callback retry. Existing numerical, preemption and pending-request source hashes remain unchanged; this proves status transport rather than additional alarm arithmetic. See `docs/P66-STATUS.md`.

