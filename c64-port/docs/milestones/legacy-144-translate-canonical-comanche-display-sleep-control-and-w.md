---
id: legacy-144-translate-canonical-comanche-display-sleep-control-and-w
title: Translate canonical Comanche display sleep control and wake continuation
seq: 144
---

CM GOSLEEPS and UPENT2 preserve all 6,144 original complete states, including mission-specific ITEMP1 and L effects. Strict tests cover 39 unchanged invalid boundaries; full sanitizer output and 64-state C64 proof pass. Canonical scheduler JOBSLEEP and rendering bodies remain separate qualifications. See `docs/COMANCHE-MAKEPLAY-SLEEP.md`.

