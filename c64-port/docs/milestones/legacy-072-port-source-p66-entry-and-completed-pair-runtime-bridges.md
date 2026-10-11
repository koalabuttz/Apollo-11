---
id: legacy-072-port-source-p66-entry-and-completed-pair-runtime-bridges
title: Port source P66 entry and completed-pair runtime bridges
seq: 72
---

Native source-entry bridges preserve the original NEWOPS, EXIT, THDUMP and basic REGODSPR paths without adding polling points. Full original-state comparisons cover 16384 cases with sanitizer and 64 C64 cases. See `docs/P66-RUNTIME-BRIDGES.md`.

