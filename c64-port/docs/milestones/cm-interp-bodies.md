---
id: cm-interp-bodies
title: Build the compiled interpretive bodies for Comanche055
date: 2026-10-12T16:00Z
lane: A
---
The 62 compiled interpretive order bodies and dispatch steps that Luminary099 code uses now also build for Comanche055 (`include/cm_interp.h`), so CM interpretive code can be ported as compiled steps over them. One shared source serves both programs. Each fixed address is spelled `IF_(luminary,comanche)`, and all 85 pairs are checked against both assembly listings; SQRTABRT's alarm exit and the fixed constant pool differ by program. 183,000 differential cases (3,000 for each of 61 bodies) equal the original Comanche055 program in yaAGC in all 2,048 words and the stop address. The 244 frozen vector cases also match on the host, under the sanitizer and in VICE. See `docs/CM-INTERP-BODIES.md`.
