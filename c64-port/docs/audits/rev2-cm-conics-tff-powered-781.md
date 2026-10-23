# Second review: cm-conics, cm-time-free-fall, cm-powered-subs refreeze (#781)

Second certificate reviewer, 2026-10-23. Lane B's #781 (merged 60b826a2, head 8b39a8fe) narrows the three Comanche055 specs' domains where the auditor's #553 salted reruns drew states no flight caller makes, and refreezes all three. **Verdict: accepted and re-cited** (Comanche055 CONIC_SUBROUTINES.agc, TIME_OF_FREE_FALL.agc, POWERED_FLIGHT_SUBROUTINES.agc). The previous records are [rev1-cm-conics-664.md](rev1-cm-conics-664.md) (conics, time of free fall) and [cm-powered-subs.md](cm-powered-subs.md).

## Code

- **EDOP at basic entries.** `interp_routine_differential.py` gains `seed_word` (irc_build's LCG jumped to one image word, with the IRC_COUNTERS redraw of 010-061) and `pending_on_key`. For `outside_off_keys` specs, a basic entry whose EDOP holds an order while LOC under BANKSET points at a file word, a registered key or DANZIG gets EDOP +0. The RTB operand pairs (`cp.pending`) keep their order. That is #553's ENDT(X) case with LOC at 27,3114: a state that runs the order on the file's own words, which no flight caller leaves.
- **cm-powered-subs** gains `outside_off_keys` (#553: SMCDURES's RVQ to a drawn QPRET at DANZIG).
- **cm-conics** NEWSTATE..TIMETHET PUSHLOC range adds vac+14. That is a widening, and it matches LAMENTER's "PL AT 12" (CONIC_SUBROUTINES.agc:1056).
- The domain text in port-coverage.json and in CM-CONICS.md, CM-TIME-FREE-FALL.md and CM-POWERED-SUBS.md states the restriction.

## Evidence

- **seed_word.** I compared it with a direct replay of tests/interp_case.c's generator over 200 random seeds, with and without counters, at 010, 023, 061, 062, 0164, 0165, 03777 and one random word: 0 mismatches.
- **My salted runs at 8b39a8fe** (`IRC_SALT=rev2-781`, 256 cases per routine): cm-conics 114,944 cases PASS (449 routines), cm-time-free-fall 31,488 PASS (123), cm-powered-subs 12,800 PASS (50).
- **The lane's runs.** All three pass the auditor's #553 salts. C64 coverage is complete: conics 4,641 of 4,641 edges and 2,162 of 2,162 gcov branches over 621 cases; time of free fall 2,206 and 1,049 over 192; powered subs 1,309 and 701 over 86. cert_precheck, cover_check and cert_arms report 0 failures, and CI is green.
- **Merged tree.** The evidence, specs, differential and sources of all three are unchanged between 8b39a8fe and 60b826a2.
