---
id: cm-tvc-massprop
title: Translate Comanche TVCMASSPROP (MASSPROP and FIXCW mass properties)
date: 2026-10-12T14:35Z
lane: G
---
Native C for all of Comanche055 TVCMASSPROP (MASSPROP and its FIXCW entry, bank 05 `3366`-`3520`, 61 executable and 30 data words), compiled one original word at a time over the canonical 2048-word image with the qualified basic-instruction primitives. The final image and IRQ equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN, including every overflow detour (6299 states never return in either); all 91 assembled words map to native code and constants, checked against the rope; a C64 diagnostic passes 128 frozen oracle rows in VICE. Adds the Lane G oracle fixture (`tools/cm_flight_prepare_reference.py`). See `docs/CM-TVC-MASSPROP.md`.
