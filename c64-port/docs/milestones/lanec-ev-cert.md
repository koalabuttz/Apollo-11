---
id: lanec-ev-cert
title: Draft the Luminary099 EXTENDED_VERBS full-file certificate
date: 2026-10-12T17:30Z
lane: C
---
Pending complete-coverage certificate for Luminary099 EXTENDED_VERBS.agc, for the auditor's review. `tools/lanec/wholefile_ev.py` assigns every assembled word (994) to one of the eight Lane C extended-verb milestones or to the file's constants. It shows that every instruction word but one unreachable `TC ALM/END` is executed (778), and every interpretive word is read by the interpreter (55), in the unchanged original program in Virtual AGC under cases the native code matches. Every entry is a transfer-ABI routine (`lanec-ev-transfer`). The evidence also covers 27,412 oracle cases (strict and ASan/UBSan, including 468 round trips through `agc_transfer_run`), 512 refusals with the state unchanged, and fresh VICE runs of all nine frozen subsets (1,861 checks). See `docs/LANEC-EV-CERT.md`.
