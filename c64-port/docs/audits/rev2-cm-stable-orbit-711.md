# Second review: cm-stable-orbit 04,3204 pending DANZIG (#711)

Second certificate reviewer, 2026-10-19. The Certificate fixes lane's #711 (merged c92630da as a squash of eae05a81, taking over Lane D2's #697) makes STABLE_ORBIT's 04,3204 a routine of its own and refreezes cm-stable-orbit. **Verdict: accepted and re-cited** (Comanche055 STABLE_ORBIT.agc). The previous record is [rev2-recite-g541.md](rev2-recite-g541.md). #711's V56 / EXTENDED_VERBS change is reviewer 1's.

## Code

- **Why.** `RTB LOADTIME` with `DAD KT` on its right comes back at DANZIG with LOC at its operand word, 04,3204. That point was not a routine, so cm-stable-orbit bridged it to yaAGC on the host.
- **Change.** The spec's `pending_danzig` makes 04,3204 an entry. The cases set the mission's DANZIG marker (AGC_TRANSFER_DANZIG_MARKER, as the flight store has it) at LOC first, so the pending right-hand order resolves the way cm-conics' does. The supported_domain gains one sentence for that entry.

## Evidence

- **My runs at d2728589.** `interp_routine_differential.py cm-stable-orbit --cover-check` gives 26,624 cases, cases_match true (286b8127…) and 0 of 881 host arcs missing on C64. On the host, cm-stable-orbit now runs 04,3204 native.
- **eae05a81 against d2728589.** The spec, source and frozen vectors for cm-stable-orbit are unchanged. Every interp_specs.py change comes from base merges. The only differences are the supported_domain sentence and a rerun of evidence/cm-stable-orbit-differential.log (26,624 cases, 104 routines). The merged tree's evidence/cm-stable-orbit is byte-identical to eae05a81.
- **Untaken arms.** `tools/cert_arms_allow.py` gains no untaken arm for cm-stable-orbit.
- **The lane's runs.** All cases equal yaAGC, selfcheck is OK, and CI is green on eae05a81.
