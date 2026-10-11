---
id: legacy-214-integrate-actual-native-p66-timer-dispatch-and-second-de
title: Integrate actual native P66 timer dispatch and second descent iteration
seq: 214
---

The111-module native C64 disk reproduces STARTP66, guidance, display and idle,100 actual timer pulses, original T3 RODTASK admission/TASKOVER resume and a second P66/display/idle iteration. All310 complete actual C64 source checkpoints and13926 clean strict/full-sanitizer source states match. Hardware stack77B, software785B, resident6f59 and171B control metadata fit the documented independent allocations. One-shot source task behavior and remaining application/device/recovery domains are explicit. See `docs/P66-APPLICATION-RECURRENCE.md`.

