---
id: legacy-016-compose-native-p66-radial-velocity-and-predicted-altitud
title: Compose native P66 radial velocity and predicted altitude from verified arithmetic
seq: 16
---

The P66 radial group now computes half-unit radius, radial velocity and predicted altitude with original VAC, signed-zero and scratch effects. All 4,952 complete 134-word original-assembly checkpoints agree; full-prefix host and sanitizer checks pass. Six C64 images verify 203 complete stage outputs from exact original pre-UNIT states. The full-prefix C64 diagnostic exceeds RAM, so whole-program overlay integration remains required. See `docs/RADIAL.md`.

