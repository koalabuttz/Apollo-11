---
id: cert-arms
title: Check that every untaken branch arm of a certified spec is triaged
date: 2026-10-19T01:00Z
lane: Certificate fixes
---
A certificate shows that the frozen host cases equal yaAGC, but a branch arm no case takes is never compared. `tools/cert_arms.py measure SPEC` rebuilds a spec's frozen cases, runs them on its generated C under gcov, and records every untaken arm (a BZF/BZMF, CCS, BOV/BOVB, interpretive branch outcome, or a word no case executes) in `tools/cert_arms_allow.py`, which sits beside the specs. Each arm has a class and a reason: IMPOSSIBLE, OFFNOM (alarms, aborts, restarts, failed sensors), FLIGHT (ordinary flight reaches it, so a case is still owed) or UNSURE. The ledgers-and-coverage CI job runs `tools/cert_arms.py check`, which fails on an untriaged arm and warns on open FLIGHT and UNSURE arms and on records measured on older frozen cases or sources. All 81 certified specs are measured: 498 untaken arms: 425 IMPOSSIBLE, 36 OFFNOM, 36 FLIGHT and 1 UNSURE. Each FLIGHT reason says how flight reaches the arm, so the owning lane knows which case to add.
