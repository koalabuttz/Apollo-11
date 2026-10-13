---
id: danzig-pending
title: Resolve a pending right-hand order at DANZIG in the transfer ABI
date: 2026-10-12T21:30Z
lane: A
---
The transfer ABI now has a point for an interpretive pair whose left order is RTB. When the RTB routine returns to DANZIG with the right order still in EDOP, and the program carries the mission's DANZIG marker table, the point is LOC under BANKSET. LOC then holds the RTB's operand word, and the file that owns the pair registers its right-order routine there. In yaAGC, 88 RTB-left pairs of both missions reach DANZIG with EDOP positive and LOC at the pair word +1. The 792 strict, sanitizer and VICE checks pass. Programs without the marker resolve DANZIG by Z as before. See `docs/DANZIG-PENDING.md`.
