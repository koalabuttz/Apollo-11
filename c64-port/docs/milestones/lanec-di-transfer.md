---
id: lanec-di-transfer
title: Register DISPLAY_INTERFACE_ROUTINES on the transfer ABI
date: 2026-10-12T21:00Z
lane: C
---
Every labelled entry and every callee return of Luminary099 and Comanche055 DISPLAY_INTERFACE_ROUTINES is an `exec_status` routine at its physical address (165 LM, 164 CM, generated and checked against the listings by `tools/lanec/routines.py di`). 996 round trips through `agc_transfer_run` (INTER-BANK, the display tables and the LM Executive) match the unchanged original programs in Virtual AGC in strict and ASan/UBSan builds. So do 1,136 of 1,164 Luminary099 and Comanche055 round trips through the display-start and MAKEPLAY routines alone (BOTHJOBS to BANKJUMP, the SAVELOCS return, fixed-fixed resolution; the other 28 are refused unchanged), and the 63 of them that take every branch arc run on a VICE PAL C64. 384 broken entry states are refused unchanged. See `docs/LANEC-DI-TRANSFER.md`.
