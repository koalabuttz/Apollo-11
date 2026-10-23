# Second review: cm-reentry, cm-integration-init, cm-stable-orbit outside_off_keys (#789)

Second certificate reviewer, 2026-10-23. Lane B's #789 (merged f3ac4305, head 307d7bc0; I reviewed 949ba7cf, and 307d7bc0 changes only the four specs' docs) gives four specs the `outside_off_keys` narrowing I accepted on #781 ([rev2-cm-conics-tff-powered-781.md](rev2-cm-conics-tff-powered-781.md)), and refreezes them. **Verdict: accepted and re-cited** (Comanche055 REENTRY_CONTROL.agc, INTEGRATION_INITIALIZATION.agc, STABLE_ORBIT.agc). The previous records are [rev2-cm-arms-679.md](rev2-cm-arms-679.md) (reentry, integration init) and [rev2-cm-stable-orbit-711.md](rev2-cm-stable-orbit-711.md). The fourth spec, lm-powered-subs (Luminary099), is reviewer 1's to re-cite.

## Code

- **Narrowing.** QPRET and the other outside CADRs stay off DANZIG and the file's keys. A basic entry whose LOC (under BANKSET) points at a file word, a registered key or DANZIG finds no order pending in EDOP, except for the RTB operand pairs. In flight QPRET is the word after a CALL, and no caller leaves an order pending on the file's own words (#553).
- **Unchanged.** The sources and the untaken arms in `tools/cert_arms_allow.py` are the same; only `cases_sha256` changes.
- **Domain text.** The `supported_domain` in port-coverage.json and the docs' Domain sections (307d7bc0) state the narrowing.

## Evidence

- **My salted runs at 949ba7cf** (`IRC_SALT=rev2-789`, 256 cases per routine):
  - cm-reentry: 100,352 cases PASS (392 routines);
  - cm-integration-init: 61,696 PASS (241);
  - cm-stable-orbit: 26,624 PASS (104);
  - lm-powered-subs: 9,984 PASS (39).
- **The lane's runs.** All cases equal yaAGC, VICE shows 0 failures with full edge cover, and CI is green.
