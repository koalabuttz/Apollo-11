---
id: interp-entry
title: Enter the interpreter from any caller (INTPRET, INTPRET +2, INTRSM)
date: 2026-10-12T19:00Z
lane: A
---
The interpreter tables of both missions now run INTPRET, INTPRET +2 and INTRSM for any caller, to NEWOPS, where the trampoline continues with the interpretive routine at LOC. Before this, INTPRET took only the E/CALL standard lead-in, and lanes registered their own entries (Lane C's EXTENDED_VERBS INTPRET). E/CALL's INTPRET keeps its composite path.

3,072 cases equal the original programs in yaAGC, in every erasable word and the 16-bit registers. Host, sanitizer and VICE runs pass, and both standard lead-in transfer suites pass unchanged. See `docs/INTERP-ENTRY.md`.
