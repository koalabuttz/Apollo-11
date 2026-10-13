---
id: lanec-pin-transfer
title: Register PINBALL_GAME_BUTTONS_AND_LIGHTS on the transfer ABI
date: 2026-10-12T22:30Z
lane: C
---
Every point at which a Pinball module is entered is a routine of the control-transfer ABI at its physical address, in both Luminary099 (273) and Comanche055 (268): every label but the internal subroutines, every external-callee return, and the BANKCALL return points through SWRETURN; routines ending at a return cell (TC WDRET, TC DSEXIT, TC Q, ...) take that one transfer. `tools/lanec/routines.py pin` generates the tables and checks them against the listings. 2,096 round trips (the genuine external callers, and every routine from its own entry chained through the routines that follow it) give 1,232 complete states identical to the unchanged original programs in Virtual AGC and 864 chains refused unchanged where they leave the modules' domains, in strict and ASan/UBSan builds; 1,530 entry states with a broken register contract are refused unchanged (`pin-refuse`). On the C64, one program per module and mission registers INTER-BANK and that module's points: 108 programs run 575 round-trip rows, picked to take every host branch arc, on a VICE PAL C64 (22 modules' programs do not fit below $C000). Luminary's SWRETURN/ISWRETRN accept Q = 3 after `TC Q`. See `docs/LANEC-PIN-TRANSFER.md`.
