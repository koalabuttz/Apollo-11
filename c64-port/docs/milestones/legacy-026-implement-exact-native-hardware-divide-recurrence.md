---
id: legacy-026-implement-exact-native-hardware-divide-recurrence
title: Implement exact native hardware divide recurrence
seq: 26
---

The independently authored fixed-point recurrence matches 327,680 isolated original hardware-DV instruction states, including raw register overflow and signed zero. Full-state sanitizers and all 255 frozen C64 register-pair checks pass. See `docs/HARDWARE-DIVIDE.md`.

