---
id: lm-initvel
title: Translate Luminary INITVEL (P34-35 required velocity, with LAMBERT)
date: 2026-10-12T16:00Z
lane: E
---
Native C for Luminary099 INITVEL (P34-35_P74-75, bank 11 02000-02274, 189 words). It includes HAVEGUES, INITVEL1-7 and INITVELX, and ORBITAL_INTEGRATION's SETITCTR (9 words) compiled inline. The routine is ten steps between original NEWOPS frontiers. It runs the lm-conic-lambert milestone's LAMBERT steps and stops at INTSTALL/INTEGRVS, which are not ported. The final 2048-word image, IRQ and SUPER equal the pinned yaAGC oracle for all 65536 sampled entries from the Luminary call returns and the integration returns (strict and ASAN/UBSAN). 7301 intermediate phase frontiers equal original NEWOPS images, and every assembled word maps to a native literal after a fresh yaYUL assembly. Four C64 phase diagnostics pass 49 frozen steps in VICE. It adds the shared VPROJ and fixed-TLOAD interpretive bodies. See `docs/LM-INITVEL.md`.
