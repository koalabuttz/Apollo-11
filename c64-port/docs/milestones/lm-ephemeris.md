---
id: lm-ephemeris
title: Translate Luminary lunar and solar ephemerides (LSPOS/LUNPOS, LUNVEL)
date: 2026-10-11T23:17Z
lane: B
---
Native C for all of Luminary099 LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES (LSPOS/LUNPOS and LUNVEL, 74 words) as a chain of nine steps between original NEWOPS frontiers, with new compiled interpretive dispatch paths (`src/lm_interp_steps.c`) over the existing frozen arithmetic bodies. The final 2048-word image, IRQ and SUPER equal the pinned yaAGC oracle for all 65536 sampled entry states from every actual caller (strict and ASAN/UBSAN), 152 intermediate phase frontiers equal original NEWOPS images, 74 assembled words map to native literals after a fresh yaYUL assembly, and four C64 phase diagnostics pass 54 frozen steps in VICE, each step dispatched through the control-transfer trampoline; LSPOS, LUNVEL and the phase frontiers are registered interpretive routines (`lm_ephemeris_routines`), and all 65536 sampled entries run through the trampoline on the host. See `docs/LM-EPHEMERIS.md`.
