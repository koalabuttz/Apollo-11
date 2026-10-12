---
id: lanec-di-cert
title: Draft the DISPLAY_INTERFACE_ROUTINES full-file certificates
date: 2026-10-12T20:00Z
lane: C
---
Pending complete-coverage certificates for Luminary099 and Comanche055 DISPLAY_INTERFACE_ROUTINES.agc, for the auditor's review. `tools/lanec/wholefile.py` assigns every assembled word (670 LM, 667 CM) to one of the six Lane C display milestones or to the file's constant table, and shows that every instruction word (612 LM, 609 CM) is executed by the unchanged original program in Virtual AGC under cases the native code matches. The evidence also covers 17,632 oracle cases (strict and ASan/UBSan) and fresh VICE runs of all six frozen subsets. See `docs/LANEC-DI-CERT.md`.
