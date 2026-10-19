# Review of PR #664 (Lane B): #553 outside return CADRs, cm-conics and cm-time-free-fall refreeze

Head reviewed: **a91da9ffa** (rebased onto a5d59442; commits 13652ed41 for #553 and a91da9ffa for the VTARGTAG pin). The first head, 170c7816e, was also checked, as was the base, c16c3ec1f.

- **cm-time-free-fall (Comanche055/TIME_OF_FREE_FALL.agc): ACCEPT**
- **cm-conics (Comanche055/CONIC_SUBROUTINES.agc): REJECT.** Blockers: tools/interp_specs.py:1903 (R1 pin; CONIC_SUBROUTINES.agc:610/618) and docs/port-coverage.json:677 + docs/CM-CONICS.md:469 (domain). The PR body's "Closes #659" must also be removed before merge.

## Audit arms (uncovered-arms.md, #659)

| arm | status at a91da9ffa |
|---|---|
| cm_conics.c:5575 FLIGHT, `SLOAD BZE VTARGTAG TARGETV` (CONIC_SUBROUTINES.agc:1606-1608) | **Taken.** The goto arm runs in 127 of 2662 cases (5576-5577 executed). The pin `'12,3544': {'VTARGTAG': [0,'any']}` (interp_specs.py:1912) is flight-plausible: INITVEL copies NUMIT, and S40.9 passes 0. The `'any'` keeps the nonzero arm. Taken on the C64 too (selfcheck: 2163/2163). It was not taken on the first head. |
| cm_conics.c:1147 UNSURE, MODDONE `SIGN BZE BADX` (CONIC_SUBROUTINES.agc:671-672) | **Still not taken** (branch 1 taken 0; 1148-1149 `#####`). It needs XKEPNEW = 0. Lane B did not claim it; it stays open under #659. |

The other `#####` lines in cm_conics.c are 1906, 4510, 4635, 5056 and 5076 (the OFFNOM arms already in the audit) and `default: EXEC_OUT_OF_SCOPE` switch guards.

## Reproduced

All runs were in /home/claude/apollo-pr664 after `source deps/env.sh`.

- **#553 on the base, verified.**
  - `IRC_SALT=specs1017`: cm-conics FAILs 4 of 287,358, at 04,3724, 12,2042 (PERIODCH step cap), 12,2667 and 12,3547. cm-time-free-fall FAILs 1 of 78,720, at 27,3245.
  - All failures except 12,2042 are QPRET/Q landing on DANZIG or a key: the oracle has kind 0, the native kind 2.
  - Fresh salt `rv664-q7` on the base: cm-conics FAILs 4 (12,2362, 12,2410, 12,2710, 12,3534); tff PASS.
- **The PR fixes it, verified.**
  - First head: specs1017 and rv664-q7 PASS on both specs (287,360 / 78,720).
  - New head: cm-conics specs1017 and the fresh salt `rv664-q8` both PASS 287,360.
- **cm-conics at a91da9ffa, verified.**
  - Differential: PASS 114,944.
  - Case-set sha 4d0d098d… equals the proof.
  - `make test-cm-conics sanitize-cm-conics`: 3592 checks, 0 failures; matches host.log.
  - `cert_selfcheck.py cm-conics`: OK, no gaps. All 4213 rows re-run on yaAGC equal their frozen digests; gcov host 2163 / C64 2163, missing 0.
  - `make cm-conics-vice`: 29 PRGs, 621 checks, 0 failures. All 29 PRG SHA-256s and every PASS line equal cm-conics-vice.log.
- **The missing edge (4641 of 4642), verified.**
  - With my own clang trace-pc-guard/trace-cmp build, the missing feature is a trace-cmp outcome at src/lm_interp_orders.c:651 (`ddv_bddv`, via cm_interp_orders.c): the `c==2` compare's "less" outcome. That means DDV with divisor major +0 and dividend major +0, not -0.
  - Only 3 cases take it: 100858, 101560 and 101620. All are in `c64_too_big_cases`, at TIMERAD 12,3556 and 12,3566.
  - Both outcomes go to the same code, and the C64 takes the -0 one, so no control-flow edge or gcov branch is missing. Lane B's claim is correct.
  - Flight-reachability (inferred): plausible but rare, since it needs two DP operands under 2^-14.
- **cm-time-free-fall, verified.**
  - Identical between the two heads: no tff file changed.
  - On a91da9ffa: host and sanitizer 984/984; differential PASS 31,488; cases sha 39090fe8… equals the proof.
  - On the first head: `make cm-time-free-fall-differential` rc=0 (vectors and groups reproduce); 8 PRG hashes equal the vice.log; VICE 8/8 PASS (192 checks).
- **Certificate checks, verified.**
  - `cert_precheck` on both files: PASS (0 FAIL; 1 WARN: no vice manifest cached).
  - `ledgers.py --validate`: PASS.
  - Every port-coverage.json evidence hash for both milestones is OK.
  - `port_coverage.py` FAILs only on other milestones' unrestored evidence.
- **Arc diffs (my gcov -l, per object).**
  - tff, base → PR: 0 lost; 1 gained (lm_interp_orders.c:326).
  - conics, first head → a91da9ffa: 0 lost; 1 gained (cm_conics.c:5575).
  - conics, base → a91da9ffa: 9 lost.
    - agc_transfer.c:9, 14, 95: logical < 02000, the superbank FB and find(NONE). About 31 base cases ran a random order at DANZIG into erasable or superbank Z.
    - agc_transfer.c:123: the DANZIG marker with negative EDOP. Base hit it 132 times; it is gone because of the new CHECKCTR `023: [0]` pin.
    - cm_conics.c:206: the indexed `TS ITERCTR` refusal guard.
    - lm_interp_orders.c:731-732: SQRTSUB's third CCS = -0 after +0,+0, in 1 base case.
    - None of these is a cm_conics.c flight arm. 732 is in the shared interpreter source (inferred: not flight-relevant, since -0 in MPAC+2 under zero majors).
- **Scope.**
  - 13652ed41 and a91da9ffa touch only tools/interp_routine_differential.py, interp_specs.py, the two milestones' vectors, groups and evidence, and the docs. No src/ change.
  - The diff from the first head also shows src/lm_conic_checkctr_routines.c, flight_store_build.py and others. These come from the rebase onto a5d59442, not from this PR.

## Blocking (cm-conics)

1. **The R1 pin is on the wrong word and its rationale is false.** tools/interp_specs.py:1903: `'vac+041': [0o1000, 0o4000, 0o20000, 0o37777]` for a <= 0o2077. AGC: CONIC_SUBROUTINES.agc:610 (`STOVL R1`) and 618 (`DMP SL3 R1`), then DDV R1 at 12,2077.
   - R1 is 32D (vac+040); vac+041 (33D) is its low word. Pinning the low word does not keep R1 "above 1/64", and the high word is still random.
   - "Above 1/64 as an orbit's radius is" is also wrong. R1 is scaled at 2^29 m (Earth) or 2^27 m (Moon), so 1/64 is 8389 km or 2097 km. LEO (high word about 0o310) and lunar orbit (about 0o341) are both below 1/64.
   - Verified with variants on the first head's code, specs1017, 287,360 cases:
     - removing the pin FAILs 1 case at 12,2072 (oracle kind 9, native kind 2), so the pin really is what masks the #553 failure;
     - `'vac+040': [0o310, 0o340]` (the flight high words) PASSes.
   - **Lane must:** pin the high word vac+040 to flight radii (Earth parking orbit through lunar orbit, for both missions' scalings); fix the comment; refreeze; re-run specs1017 plus a fresh salt.
2. **The supported_domain does not state the PR's input restrictions.** docs/port-coverage.json:677 and docs/CM-CONICS.md:469 still say "Other conic erasables take any value". They do not mention:
   - the R1 bound (blocker 1);
   - TAU. high word ±0 (|TAU.| < 163.84 s), which now applies at KEPLERN's flight entry 12,2042 (interp_specs.py:1905). Flight TAU. there, the time since rectification, can be larger. This is inferred from ORBITAL_INTEGRATION.agc:103/106; coverage is still complete;
   - CHECKCTR..NEWSTATE EDOP = +0 (:1935);
   - NEWSTATE..TIMETHET PUSHLOC vac+0..+12 (:1926);
   - VTARGTAG 0 in half the cases at 12,3544 (:1912);
   - the 12,3016 MODE/PUSHLOC pin (:1908).

   **Lane must:** list each one in both texts with its flight justification, and drop "Other conic erasables take any value".

**PR housekeeping (must fix before merge, not a certificate defect).** The PR body says "Closes #553" and **"Closes #659"**. #659 is Lane G's multi-spec issue (cm-entry-dap, cm-integration-init, cm-p61-p67, cm-powered-subs, cm-reentry), and 1147 is still open. Merging would close it wrongly, so change it to "Refs #659". No commit message says "closes #659": a91da9ffa's title is "cm-conics: LAMBERT's VTARGTAG 0 arm (#659)".

## Not blocking

- **The `outside_off_keys` tool change** (interp_routine_differential.py, +7 lines) is correct, default-off, and set only by these two specs (interp_specs.py:1575, 1863).
  - Neither file registers keys outside its code, so in practice it only excludes DANZIG as a return CADR. That is not flight-reachable: QPRET is the word after a CALL.
  - It applies to the shared case image, so the native and yaAGC sides get it the same way.
  - It does not hide a native defect. The failing cases were the native DANZIG marker (src/agc_transfer.c:106, kind 2) against yaAGC continuing.
- **12,3016 pins PUSHLOC to vac+2** (interp_specs.py:1908). In flight it is vac+4 after the PDDL at 12,3013. The GETX joint states cover the DDV arms either way. Cosmetic; it should say so, or use vac+4.
- **1REV and GETX+13 joint states** (:1945-1957) are edge-argument states for SQRT and DDV. They add coverage and are flight-plausible.
- **The missing trace-cmp feature** at lm_interp_orders.c:651, see Reproduced. It is not a control-flow edge.
- **`independent_review`** still cites docs/audits/rev2-recite-b506.md, an older audit.
- **cm-powered-subs**, named in the #553 comment, is not addressed by this PR and stays open.
- **cm-time-free-fall**: its domain text (docs/CM-TIME-FREE-FALL.md) adds only the QPRET sentence, which is accurate. Nothing blocking.

## Verified vs inferred

- **Verified by running:** everything under Reproduced, the R1 variant runs, the gcov and arc diffs, the missing-edge location, and the PR body and commit text (REST API).
- **Inferred from the AGC source:**
  - the flight TAU. range at KEPLERN entry;
  - the flight-reachability of the DDV +0/+0 feature and of SQRTSUB -0;
  - the plausibility of VTARGTAG=0, which follows the audit's P40-P47.agc / P34-35 reading.

---

# Round 3 (8fea1e63)

Head **8fea1e63c** (rebased on 31e99126; 57cdbd112 = a91da9ffa rebased; 8fea1e63 changes only interp_specs.py, the cm-conics vectors/groups/evidence and the two domain texts). Worktree /home/claude/apollo-pr664c, `source deps/env.sh`, rope assembled with tools/fixed_data_generate.py (MAIN.agc.bin identical to apollo-a2b's).

- **cm-time-free-fall (Comanche055/TIME_OF_FREE_FALL.agc): ACCEPT** (unchanged)
- **cm-conics (Comanche055/CONIC_SUBROUTINES.agc): REJECT.** Blocker: the C2 pin tools/interp_specs.py:1936-1937 (CONIC_SUBROUTINES.agc:613/615/619) is narrower than flight and its stated cause is wrong; docs/port-coverage.json:677 and docs/CM-CONICS.md:469 repeat both errors and misstate two other pins.

## Round-2 blockers

| # | status |
|---|---|
| 1 R1 pin | **Fixed.** interp_specs.py:1928 pins the high word vac+040 (R1 = 32D) to 0310 (LEO, 2^29 m), 0340 (LLO, 2^27 m), 02000, 017571 (RSPHERE, ORBITAL_INTEGRATION.agc:928, at 2^27) and 026706 (the Moon's distance, 2^29), for 12,2042 and 12,2045-2077. These are flight radii and the comment is now right. |
| 2 domain text | **Mostly fixed.** TAU., R1, PUSHLOC, EDOP, VTARGTAG and 12,3016 are now listed, and "Other conic erasables take any value" is gone. The new round-3 pins are misstated; see Blocking 1-2. |
| PR body | **Fixed.** It now says "Refs #659", and no commit says it closes #659 (REST API). |

## (a) What the runaways were (verified)

I reproduced both from the PR's own src, which is unchanged since a91da9ff (src/ diff a91da9ffa..8fea1e63 touches only lm_* files outside this build).

- **specs1017**, spec = head minus the C2/ALPHA/XMAX pins and the 1REV-8 joint (R1 pin only): FAIL 1/287,360, case 27577 at **12,2074**, oracle kind 9 (Z 06475) vs native kind 2 (Z 06037). No word differs.
  - Native dump: KEPC2 = **012562** 76773. That is C2 = **+21.4**, hyperbolic, **not near -1**. R1 = 0340 (LLO).
  - (1/64-C2/64)/2 / R1 overflows the DDV. ALPHA = 40000 40000, XMAX = 00310 00001, period 0D = **+0 exactly**, and TAU. = 77777 00040 is never reduced.
  - The native state is identical at 2,000 and 65,000 steps. The oracle is still in PERIODCH at 4M and at **400M** cycles, and its trace shows MPAC = TAU. = 77777 00040, the same value.
- **lb664r2**: with the R1 pin only, it PASSes 287,360. With R1+C2 (no ALPHA/XMAX), it FAILs 1 at **12,2110**, oracle 9 vs native 2.
  - The entry has ALPHA = 13755 01322, positive. That is flight-impossible on the hyperbolic arm: BPL 1REV at 12,2102 sends only negative ALPHA there.
  - XMAX = +0 and period +0. The oracle loops at 400M cycles with MPAC = TAU. = 77777 00040, and the native state is the same at 2k and 65k steps.
- **Verdict on (a):** in both cases native and yaAGC run away the same way, an endless PERIODCH with period +0. The only "mismatch" is the classification: oracle cycle cap (kind 9) vs native step cap (kind 2). The interp_routine_differential.py:1403-1411 "both sides loop" filter only joins 9/9.
  - There is no native divergence, so no native fix is needed.
  - The real flight bound is "ALPHA does not overflow": |a| > 2^22 m = 4194 km (Earth) or 2^20 m = 1049 km (Moon). Every flight conic satisfies it.
  - Lane B's description ("C2 near -1 ... a period of a few centiseconds ... thousands of passes") does not match the reproduced case: C2 = +21.4, ALPHA overflow, period exactly 0, endless loop.

## (b) Flight plausibility of the pins

- **R1 (:1928): OK** (see above).
- **C2 (:1936-1937): too narrow.** The values are 2074/2075 {-1/2, -1/4, 0, 1/2, 1, 3} and 2077 {2, 1.5, 1, 1/2, 0, -1/2}. C2 = R·V²/MU - 1 is about e at periapsis and grows with r on a hyperbola: C2 = 1 + r/|a|.
  - At the AGC's RSPHERE (64,374 km, moon frame), a lunar-approach or post-TEI hyperbola with v∞ of 0.8-1.2 km/s has C2 of about **15-20**. At 20,000 km it is about 5.
  - So every Moon-centred hyperbolic conic with C2 > 3 (r ≳ 8,000 km) is excluded, and that is flight-typical for the whole lunar-approach and TEI coast. TLI cutoff (C2 ≈ 0.97) and TEI cutoff (≈ 1.5) are inside the range.
  - Excluding them is not needed. I replaced the pin by joint (R1 high, MPAC high) pairs at KEPLERN+26/+27/+29 (12,2074/2075/2077), with ALPHA kept in range:
    - LEO C2 -0.03..0.97;
    - LLO -0.05..1.54;
    - 02000 0.87, 2.7;
    - **RSPHERE 3, 7, 15, 20**;
    - Moon distance -0.97..-0.3.
  - With those pairs, `IRC_SALT=specs1017` and a fresh salt `rv664-r3a`, at 640 cases, both **PASS 287,360**.
- **ALPHA negative at 12,2105/2110 (:1944): OK.**
  - ALPHA = 1/a, with a < 0 on a hyperbola (CONIC_SUBROUTINES.agc:619, BPL 1REV :621-622).
  - The samples are -0.25, -0.0625 and -0.0038 (2^-20/2^-22). They cover both BOV outcomes of DDV ALPHA (|ALPHA| < 50·2^-12 overflows).
  - Lunar hyperbola ALPHA is about -0.1..-0.31. That is inside "negative" but only sparsely sampled; not blocking.
- **XMAX positive at 2110/2114/2116/2117/2122 (:1945): OK.**
  - XMAX (10D) is not set by callers. KEPLERN computes it itself: sqrt(-50SC/ALPHA) or 2PISC/sqrt(ALPHA), both positive, with -50SC/ALPHA ≥ 50·2^-12 in flight.
  - The MPAC values at those points are all positive in flight.
- **1REV-8 joint (:1989-1991): OK and flight-exact** (MPAC = ALPHA after STORE ALPHA). It is applied to half the cases.

## (c) Domain text vs pins (docs/port-coverage.json:677 = docs/CM-CONICS.md:469; the two texts are identical)

- "C2 ... is a flight value from -1/2 to 3": the range is not the flight range (see (b)).
- The rationale "a random tiny R1 or C2 near or past -1 makes ALPHA an orbit far below R1/2, and PERIODCH runs thousands of passes" is wrong. The observed case is a C2 of +21 at LLO radius: ALPHA overflows, the period is 0 and the loop never ends. The comment at interp_specs.py:1929-1935 has the same error.
- "at BPL 1REV (12,2102) MPAC and ALPHA are the same value": true in only **half** the cases (`+ [{}] * 4`, :1991), so it should say so, as the VTARGTAG sentence does.
- "from the SQRT to the period's DMP (12,2110-12,2122) XMAX in MPAC is positive" does not match the pinned points:
  - the pins are 2110, 2114, 2116, 2117 and 2122. **12,2112** (1REV's SQRT, MPAC = ALPHA) is in the stated range but not pinned;
  - at 2110, MPAC is the SQRT argument -50SC/ALPHA;
  - at 2122, MPAC is XMAX·1/ROOTMU.
  - These should be stated per point.
- R1, ALPHA, TAU., PUSHLOC, EDOP and VTARGTAG match their pins exactly.

## Reproduced (head 8fea1e63, verified)

- **`make cm-conics-differential`:** PASS 114,944, rc 0. `git diff --exit-code` is clean, so vectors and groups regenerate identically. Case-set sha **9c9f7d35…** equals the proof.
- **Salts at 640 cases, head spec:** `IRC_SALT=specs1017` PASS 287,360; fresh salt `rv664-r3b` PASS 287,360.
- **Host and sanitizer:** `make test-cm-conics sanitize-cm-conics` gives 3592 checks, 0 failures.
- **`cert_selfcheck.py cm-conics`:** OK, no gaps. All 4215 rows re-run on yaAGC equal their digests; c64_gcov_branches is host 2164 / C64 2164, missing 0.
- **`make cm-conics-vice`:** 29 PRGs, **623 checks, 0 failures**. All 29 PRG SHA-256s and every PASS line equal cm-conics-vice.log.
- **Certificate checks:**
  - `cert_precheck` on both .agc files: PASS (1 WARN: no vice manifest cached).
  - `ledgers.py --validate`: PASS.
  - All port-coverage.json evidence hashes for both milestones are OK.
  - `port_coverage.py` FAILs only on other milestones' unrestored evidence.
- **cm-time-free-fall:** no tff file differs from a91da9ffa, and its SPECS entry is identical. `make cm-time-free-fall-differential`: PASS 31,488 with the tree clean, plus 984/984 host and sanitizer checks.
- **My gcov** (gcc -O0 --coverage, per object, gcov -b from c64-port; build/gcov-r3/):
  - head: **2164 branch outcomes taken**, matching the lane;
  - cm_conics.c `#####`: 1148-1149 (MODDONE BZE, 12,2147, open under #659); 1906, 4510, 4635, 5056 and 5076 (the audit's OFFNOM arms); the unused helpers 89-140; and `default: EXEC_OUT_OF_SCOPE` guards;
  - so uncov = 12,2147 only, as claimed.
- **Arc diffs** (frozen case sets regenerated: a91 sha 4d0d098d…, base sha 5ebd639d…, both equal their proofs; same native sources):
  - a91da9ff → head: 2163 → 2164, **lost 2**, gained 3 (cm_conics.c:206, lm_interp_orders.c:731/732).
    - Lost lm_interp_orders.c:703/704 (via cm_interp_orders.c). DDV BUFNEG takes CSF HALF when the divisor major is exactly -1 with a positive minor (|divisor| < 2^-14, mixed sign). a91 took it in 3 random cases (12,2400 and 12,3620 DDVs).
    - Lost lm_interp_orders.c:1070. BOV with OVFIND = -0. a91 took it in 1 random case (12,3304); the interpreter never leaves OVFIND -0.
    - Both are shared-interpreter edges hit by chance and lost when the case stream reshuffled. Neither is a cm_conics.c arm or a flight-typical state (inferred).
  - base 31e99126 → head: 2160 → 2164, lost 6. These are agc_transfer.c:9, 14 (2 arcs), 95 and 123 (all with the round-2 reasons) plus the 703 arm above; gained includes cm_conics.c:5575-5577 (VTARGTAG).

## Blocking (cm-conics)

1. **The C2 pin is narrower than flight.** tools/interp_specs.py:1936-1937 (AGC CONIC_SUBROUTINES.agc:613 STORE KEPC2, :615 BDSU SR1R, :617 DDV R1). It excludes every Moon-centred hyperbolic conic with C2 > 3, which covers the lunar approach and the post-TEI coast inside RSPHERE (C2 up to about 20).
   - **Lane must:** replace it with joint (R1, C2) states that keep ALPHA in range (|a| > 2^22 m Earth, 2^20 m Moon). Include Moon-frame RSPHERE/02000 radii with C2 up to about 20, and Earth radii with -1 < C2 ≤ 1.
   - My KEPLERN+26/+27/+29 pairs PASS specs1017 and a fresh salt at 287,360 cases each.
   - Then fix the comment at :1929-1935, refreeze, and re-run the salts.
2. **The domain text is wrong in places.** docs/port-coverage.json:677 and docs/CM-CONICS.md:469.
   - **Lane must:**
     - restate the C2/R1 bound as the coupled flight bound;
     - correct the cause to "ALPHA's DDV overflows (C2 large or -1 against a small R1), the period is +0 and PERIODCH never ends";
     - say "in half the cases" for MPAC = ALPHA at 12,2102;
     - state the MPAC sign per pinned point (2110, 2114, 2116, 2117, 2122), and not claim 12,2112.

## Not blocking

- The lost shared arcs lm_interp_orders.c:703/704 (DDV divisor major -1 with a positive minor) and :1070 (BOV with OVFIND -0) are inferred non-flight. A GETX+13-style DDV edge joint would keep 704 deterministic.
- The PR body's "How" list does not mention the C2, ALPHA and XMAX pins. The commit message does.
- 12,2105's MPAC (flight: -50SC from the DLOAD) is left random. This is wider than flight, and harmless.

## Verified vs inferred

- **Verified by running:** everything under (a) and Reproduced, the flight-coupled C2 variant runs, the gcov and arc diffs, and the case origins of the lost arcs (an instrumented native copy under build/gcov-r3/instr).
- **Inferred (orbital mechanics and AGC source):**
  - the flight C2 and ALPHA ranges;
  - that OVFIND is never -0 in flight;
  - that a DDV divisor with major -1 is not a conics flight state.

---

# Round 5 (c1759c2c)

Head **c1759c2c** (refreeze on top of b16e817e, which merges ef6cfa02 with base f7d5802). Compared with f7d5802, it changes no src/ file. It touches only interp_specs.py (#664's states plus #659's TARGETV-5, and the new 12,3605 joint state), the `outside_off_keys` tool lines, cert_arms_allow.py (case sha), the two milestones' vectors, groups and evidence, and the docs. The base tip has since moved to 31dcd226. Between f7d5802 and 31dcd226, none of this PR's files or linked sources change, and the PR is mergeable. Work was done in a fresh worktree /home/claude/apollo-r5c (removed at the end), with `source deps/env.sh`. The ropes were built with fixed_data_generate.py, and both MAIN.agc.bin hashes equal apollo-a2b's.

**Note on rounds:** this file has no "Round 4" section; I could not check the r4 text itself. The r1-r3 blockers are checked below against the head.

- **cm-time-free-fall (Comanche055/TIME_OF_FREE_FALL.agc): ACCEPT** (unchanged)
- **cm-conics (Comanche055/CONIC_SUBROUTINES.agc): REJECT.** There are two cheap blockers. The new 12,3605 joint state does not write the words that the domain text says it does (it takes the arc in 3 cases, by chance), and the CM-CONICS.md evidence counts are stale. Everything else passes.

## Earlier blockers after the merge (verified)

| | status at c1759c2c |
|---|---|
| R1 pin (r1) | **Fixed and kept.** The high word vac+040 is pinned to 0310/0340/02000/017571/026706 for 12,2042 and 12,2045-2077 (interp_specs.py:2005). |
| C2 pin too narrow (r3) | **Fixed and kept.** The pin is now joint (R1, C2) flight pairs at KEPLERN+26/+27/+29 (R1C2/R1C2A, interp_specs.py:425-430 and the joint states). These include 017571 with C2 3-20. |
| Domain text (r1-r3) | **Fixed and kept** for R1/C2, ALPHA, XMAX per point, "half the cases" at 12,2102, TAU., PUSHLOC, EDOP, VTARGTAG and 12,3016. The "Other conic erasables take any value" sentence is gone. |
| #659 arms | **Kept.** cm_conics.c:5575 and 5579, both outcomes, are taken in the head, in ef6cfa02 and in base f7d5802. The 12,3544 VTARGTAG `[0,'any']` pin and #659's `TARGETV-5` joint state are both present. |
| PR body | It says "Closes #553" and "Refs #659" (REST API). |

## Reproduced (verified)

- **`make cm-conics-differential`:** PASS 114,944 (449 routines), rc 0.
  - `git diff --exit-code` on the vectors and groups is clean.
  - The regenerated proof is byte-identical to evidence/cm-conics/cm-conics-proof.json (cases_sha256 3c546535…, the same as cert_arms_allow.py).
- **Host and sanitizer:** `make test-cm-conics sanitize-cm-conics` gives 3592 checks, 0 failures, which matches host.log.
- **`make cm-conics-vice`:** 30 PRGs, **624 checks, 0 failures**.
  - Every PASS line equals cm-conics-vice.log.
  - All 30 PRG SHA-256s equal the log.
  - The vice_targets 322/172/130 add up to 624.
- **`cert_selfcheck.py cm-conics --skip-differential`:** OK, no gaps; host 2164 / C64 2164, missing 0.
- **cm-time-free-fall:**
  - `make cm-time-free-fall-differential`: PASS 31,488, tree clean.
  - test and sanitize: rc 0.
  - No tff evidence, vector or groups file differs from ef6cfa02.
  - The cert_arms record (sha 39090fe8…) has 0 arms.
- **Certificate checks:**
  - `cert_precheck` on both files: PASS (0 FAIL; 1 WARN: no vice manifest cached).
  - `ledgers.py --validate`: PASS.
  - `evidence.py verify` for both milestones: PASS.
  - Every port-coverage.json evidence hash for both files: OK.
  - `cert_arms.py check`: 0 failures. cm-conics has the same 6 untaken arms as the base (5 OFFNOM, plus the UNSURE arm 1147).

### Per-object gcov arc diff (my own)

**Method.** I used a wrapper around `--cover-check` (gcc -O0 --coverage over all 12 linked sources, `gcov -b` per object) that dumps the arc sets. All 12 linked sources are identical at ef6cfa02, f7d5802 and the head. Each revision's own interp_specs.py regenerated its frozen cases, and all three case sets hash to their proofs' cases_sha256: ef6 b2120fe5…, base 28dc0164…, head 3c546535….

| from → head | host arcs | lost | gained |
|---|---|---|---|
| ef6cfa02 → c1759c2c | 2163 → 2164 | **0** | lm_interp_orders.c@cm_interp_orders:611 b2 |
| f7d5802 → c1759c2c | 2161 → 2164 | **7** | 10 (346, 354, 383, 401, 611, 616, 654, 655, 740, 741, all shared DDV/SQRT edges) |

- **Against ef6cfa02,** the lane's claim is exact: 0 lost, 1 new.
- **Against the base, 7 arcs are lost.** None is a cm_conics.c arm.
  - agc_transfer.c:9 b0, :14 b0 and b3, :95 b2 and :123 b1. These are the same losses accepted in rounds 2-3: outside CADRs at DANZIG and random EDOP. They are gone by design (`outside_off_keys` and the CHECKCTR EDOP +0 pin).
  - lm_interp_orders.c:703 b2. This is DDV BUFNEG with a divisor major of -1 and a positive minor; round 3 already listed it as not blocking.
  - lm_interp_orders.c:**658 b1**. This is new. It is DDV's BUFZERO path after the swap with both majors 0 (a dividend major of ±0 under a divisor major of +0), the same kind of edge as round 1's trace-cmp feature at :651. The base hit it in a random case.
  - (Inferred) none of these is a flight-typical conics state.
- So the request "0 lost against base" does not hold literally. I do not block on it, because the cert covers the file's own arms. A DDV edge joint state (like GETX+13) would make :658 and :703 deterministic.
- In the head's cm_conics.c, the only `#####` lines are 1148-1149 (12,2147 UNSURE), 1906, 4510, 4635, 5056 and 5076 (the OFFNOM arms), the unused helpers 89-140, and the `EXEC_OUT_OF_SCOPE` guards. That is the same as round 3.

### The 12,3605 joint state (verified by running each case)

- **Where it is.** 12,3605 is the `VXSC VSU` in **TIMERAD** (CONIC_SUBROUTINES.agc:1642-1643, TIMERAD at 12,3556). It is not in LAMBERT, as the comment at interp_specs.py:2095 says. Its operands are:
  - MPAC = D1/64 - R1A, which is (1 - R1A) at B+6;
  - UR1 is the VXSC operand: word 12,3606 = 02724, which is E5,1723 in the listing;
  - the subtrahend is the vector that PDDL pushed at 12,3602 ("PL AT 6").
- **What the state writes.** interp_specs.py:2106 writes `'E:%o' % (0o1724+i)`. That means E<case EB>,1724-1731, under the case's EBANK.
  - cm-conics has no `'ebank'` key, so cases draw EB 0-7.
  - The interpreter takes UR1's EBANK from the operand itself (lmi_direct GEADDR, `TS EBANK`), so it always reads **E5,1723-1730**.
  - So in 7 of 8 cases, UR1 is not written at all. With EB 5, the writes are one word off: UR1's first component high word stays random.
- **PUSHLOC is not pinned** at 12,3605 (cm-conics has no `resume_flow` or `flow_states`), so `vac+0..5` is the popped vector only when the drawn PUSHLOC happens to be vac+6.
- **Measured.**
  - Of the 256 frozen cases at 12,3605, 61 have the joint state.
  - The rare arm of OVERFLOW (lm_interp_orders.c:298, the return without OVFIND) runs 6 times in all 114,944 cases. All 6 are in **3 joint cases**, each with EB 5 and PUSHLOC = vac+6. No other case takes it.
  - With the state rewritten as `'UR1+%d'` keys plus `'12,3605': {'0166': ['vac+6']}` (my variant, not frozen), **59** joint cases take it.
- **So the arc is taken now only by chance.** The next reshuffle can lose it again, which is how it was lost this time. The domain text describes UR1 and "PL at 6" values that the cases do not have.
- **Flight plausibility (inferred).** A scalar of about 1 means R1A ≈ -63, that is r/|a| ≈ 63 on a hyperbola. UR1 components of 1/2 each (B+1) give |UR1| = √3, not a unit vector. So this is not a flight state. It is an edge state for a shared-interpreter arc. As a widening it is harmless, and the domain text does not call it flight. It only has to say what the cases actually contain.

## Blocking (cm-conics)

1. **The 12,3605 joint state writes the wrong words, and the domain text describes words the cases do not have.**
   - Spec: tools/interp_specs.py:2095-2107.
   - Text: docs/port-coverage.json:677 and docs/CM-CONICS.md:469 ("A quarter of those at 12,3605 (VXSC VSU, PL at 6) have … UR1's components 020000 020000 and the push-down vector's 057777 020000").
   - AGC: CONIC_SUBROUTINES.agc:1640-1643. Native: src/cm_conics.c:5750-5760.
   - **Lane must:**
     - write UR1 by symbol (`'UR1+%d'`, E5,1723-1730) instead of `E:1724+`;
     - pin PUSHLOC to vac+6 at 12,3605 (`states`);
     - fix the comment (TIMERAD, not LAMBERT);
     - refreeze, and re-run the per-object arc diff (0 lost against ef6cfa02), the VICE run and selfcheck;
     - keep the text in step with the state, and optionally say it is an interpreter edge state, not a flight one.
2. **The CM-CONICS.md evidence counts are stale.** docs/CM-CONICS.md:478 still says "29 diagnostic programs with 623 cases … 322, 172 and 129 … 4641 of 4641 … 2163 of the 2163". The evidence says 30 PRGs, 624 cases, 322/172/130, 4643 of 4643 edges (proof c64_coverage) and 2164 of 2164 (c64_gcov_branches). **Lane must:** update the line from the refrozen proof.

## Not blocking

- **The UNSURE arm 1147 (12,2147 MODDONE `SIGN BZE BADX`).**
  - It is unchanged from the base, which is already `complete` with it, and the PR says "Refs #659".
  - XKEPNEW is written only by KEPPREP (ORBITAL_INTEGRATION.agc:97): XKEP + S·(…). So it is ±0 when the step's TAU. is 0 after RECTIFY (XKEP = 0), which (inferred) is a zero-length integration step.
  - A MODDONE joint state with XKEPNEW ±0 and TAU. 0 would settle it cheaply. I recommend it, but it belongs to #659.
- **The other 5 untaken arms** (1905, 4509, 4631, 5055, 5075) are OFFNOM, as read in the audit: TIMEOVFL, a colinear transfer, the thin COGAMAX band, non-convergence and a stall. I see no FLIGHT arm untaken.
- **The domain's "under EB 6 (the file's EBANK=)" is not what the cases do.** Cases draw EB 0-7, because the spec has no `'ebank'`; the file's own `EBANK=` is UR1's, E5. This is a wider domain than stated, and it was already in the base text. It should say "any EB (cases draw 0-7)", as the tff text does.
- **Arcs lost against base:** lm_interp_orders.c:658 b1 and :703 b2 (see the arc diff above).
- **Other spec text:** 12,3605's comment says "#659's TARGETV-5 state reshuffled the random case". That is accurate (verified: ef6cfa02 took :298 by chance).

## Flip or re-cite, and independent_review

- **It is a re-cite, not a flip.** Both files are already `"status": "complete"` on base f7d5802 and on 31dcd226. The PR refreezes the evidence and updates the domain text and hashes. Lane B merges it; no separate pending → complete flip PR is needed.
- **independent_review:**
  - cm-conics still cites docs/audits/rev2-cm-arms-679.md, the #659 arm audit. It does not review this refreeze or the #553/#664 domain. Once the blockers are fixed, it should cite this review (cm-conics-664.md through the accepting round) committed under docs/audits/.
  - cm-time-free-fall cites rev2-recite-b506.md. Its proof and VICE evidence were re-frozen in #664 (accepted round 1), so it should cite the same #664 review.

## Verified vs inferred

- **Verified by running:** everything under Reproduced, the three-way arc diff, the per-case 12,3605 measurement, the corrected-state variant, the operand EB decoding (lmi_direct, listing 12,3606 = 02724 → E5,1723), the PR body and the mergeability.
- **Inferred:** the flight plausibility of the 12,3605 values and of the lost DDV edges, and when XKEPNEW is ±0.
