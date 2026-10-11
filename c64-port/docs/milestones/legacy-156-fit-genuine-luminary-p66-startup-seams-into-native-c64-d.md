---
id: legacy-156-fit-genuine-luminary-p66-startup-seams-into-native-c64-d
title: Fit genuine Luminary P66 startup seams into native C64 disk overlays
seq: 156
---

STARTP66 setup, bias and vertical stages now load through fixed source-call seams without an added Executive poll. Strict and full sanitizer runs match 2,816 complete original states. A stock C64 disk proof passes 88 complete checkpoints across 32 cases; all eight application service payloads fit, with three startup stages executed in this packet. See `docs/P66-STARTUP-OVERLAYS.md`.

