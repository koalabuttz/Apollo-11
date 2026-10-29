# Comanche Executive zero-priority scan

`cm_scan_zero` extends the immutable canonical END, JOBSLEEP and PRIOCHNG
wrappers with the actual original EJSCAN +0 branches. It takes the sole
2048-word flight image, explicit external latches and an output transfer;
it never executes the selected application or alarm body.

The original scan is not uniform. At source slots 1, 2, 4, 5 and 6, +0
executes `TC CCSHOLE`. The wrapper commits the actual earlier entry effects,
partial highest-priority scan, BUF words, return Q, source banks and IRQ0,
then reports the real CCSHOLE05747 tuple. Its Q is respectively 03144,
03150, 03160, 03164 or 03170. PRIOCHNG has already installed EXECBANK02006
in BBANK, including EBANK03000; END and JOBSLEEP preserve the old EBANK.
The later independently compiled alarm/recovery stages are real continuations.

Slot 3 instead encounters the original `-CCSPR` word67610, then TCF+1.
Its fixed-address AD result is overwritten by the following CCS and has no
retained source effect at the completed group. Under caller producer
exclusion, the wrapper temporarily represents this unavailable slot as -0
while calling the source-qualified ordinary exchange, and restores its actual
+0 before the overlay returns. No source producer can observe that transient
adaptation, no private persistent image exists, and all other source words and
external latches match the original completed group. This transient method
does not permit an overlay swap, callback or asynchronous producer inside it.

The accepted contract is the ordinary canonical hardware frame, positive
current priority and source-allocated fraction (NOVAC0110 or one of the five
VAC bases+1), compiled caller Q0100..07777, normal source sleep CADR or packed
priority, and legal selected source context. Unsupported preflight inputs
leave image, I/O and transfer unchanged. This is an exceptional-state
translation, not a claim that normal admission publishes a zero priority.

The final 12,288 complete source states cover all three public entries, each
zero position, earlier active/dormant/-0 candidates, NOVAC and all five actual
VAC fractions, all31 packed priorities, bank/SUPER/IRQ variations and raw L
patterns. Each record retains all2048 words and all13 external latches.
The source stream SHA is
`90bbea0c97f7ea7a81f19adb70c581804da25b82c36205e9f784d0bb9fdafbb9`.
The original rope is the independently assembled6e3150 baseline. No original
source or existing helper is changed; the GPL engine is host-oracle only.

`tools/cm_scan_zero_differential.py` checks complete original/native streams;
`tools/cm_scan_zero_sanitize.py` checks the full sanitizer stream against that
same proof; `tools/cm_freeze_scan_zero.py` freezes source-selected C64 cases.
The target uses three real disk overlays with returned C frames, one canonical
image, explicit scratch-size bounds, caller exclusion and incoming hardware
I/ROM checks. Its checker compares every word/latch by retained fingerprints,
two complete2061-field samples directly and the final2048-word capture directly.

The earlier successful VAC1-only source/SAN/C6472 run is retained exactly in
`evidence/cm-scan-zero-vac1-first-pass/`, with an audit of its original artifact
identities. It is distinct from the expanded final source/target cohort.
The containing 337-word Executive still requires its remaining public caller
and wake/exception-domain closures before whole-file certification.
