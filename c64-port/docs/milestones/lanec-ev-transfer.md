---
id: lanec-ev-transfer
title: Register EXTENDED_VERBS on the transfer ABI
date: 2026-10-12T22:00Z
lane: C
---
Every labelled entry and every callee return of Luminary099 EXTENDED_VERBS is a routine of the control-transfer ABI at its physical address (313 basic, 15 interpretive keyed by LOC), and so is every Comanche055 entry the suites verify (241); `tools/lanec/routines.py ev` generates the tables and checks them against the listing. 468 round trips from Pinball's POSTJUMP GOEXTVB and the other BANKCALL/POSTJUMP callers, through INTER-BANK, the display interface, the LM Executive and the interpreter, match the unchanged original programs in Virtual AGC in strict and ASan/UBSan builds; 64 run on a VICE PAL C64, and 512 broken entry states are refused unchanged. See `docs/LANEC-EV-TRANSFER.md`.
