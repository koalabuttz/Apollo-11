---
id: lanec-pin-gaps
title: Translate Pinball verb 07 (DSPDPDEC) and CHANLOAD
date: 2026-10-12T00:40Z
lane: C
---
Native C for the Pinball paths no earlier milestone covered, in both Luminary099 and Comanche055: verb 07 double-precision decimal display (DSPDPDEC through ENDDPDEC, and DSP2DEC through END2DEC), the channel load for channel nouns (CHANLOAD), and the NVSUBB/LOADLV1 lead-ins. 8,704 complete states (all 2,048 erasable words, central registers, I/O channels 0-77, superbank and IRQ) are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 224 of them (every entry, both missions) match the oracle on a VICE PAL C64. Adds `lanec_ad`, `lanec_erasable`, `lanec_rewrite` and `lanec_write` to the shared Lane C helpers. See `docs/LANEC-PIN-GAPS.md`.
