---
id: lanec-ev-front
title: Translate the EXTENDED_VERBS front end and add the Lane C oracle harness
date: 2026-10-11T23:40Z
lane: C
---
Native C for the EXTENDED_VERBS front end of both Luminary099 and Comanche055: the extended verb fan (GOEXTVB, LST2FAN, all 60 slots), the extended-verb display grab and release (TESTXACT, SETXTACT, XACT0 and their NVSUB returns, XACTALM) and the operator-error exits (ALM/END, GOPIN, LM CHKPOOH). 5,760 complete states (all 2,048 erasable words, central registers, I/O channels 0-77, superbank and IRQ) are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 251 of them (every entry, both missions) match the oracle on a VICE PAL C64. Adds the shared Lane C harness (tools/lanec/) that later DSKY milestones reuse. See `docs/LANEC-EV-FRONT.md`.
