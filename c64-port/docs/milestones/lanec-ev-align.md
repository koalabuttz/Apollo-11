---
id: lanec-ev-align
title: Translate the EXTENDED_VERBS alignment and radar-designate bodies
date: 2026-10-12T21:30Z
lane: C
---
Native C for the IMU zero, coarse and fine align, attitude error meter, rendezvous radar designate and zero, landing radar position and optics coarse-align verb bodies of EXTENDED_VERBS in Luminary099 (bank 43 and AURLOKON in bank 23) and Comanche055 (bank 43), with OP/INERT, RDRUSECK and CKLFTBTS, translated block by block. 5,056 complete states are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 241 of them (every entry, both missions) match the oracle on a VICE PAL C64. See `docs/LANEC-EV-ALIGN.md`.
