# Live CDU projection

The native projection composes CDUTRIGS, AX*SR*T, DOT14, STORE22 and saved acceleration BDDV into AFC. Three explicit APIs return at the original pair-consumed scheduling points LOC03427, LOC03430 and LOC03433. Pending-job source tests prove the basic CDU trig section completes before the first CHANG2 handoff; no artificial yield is inserted within it.

All 8,192 direct cases match three complete 134-word checkpoints (24,576 comparisons, SHA256 `9fa9c041a266df1f9ea1e7951c8494a08bb27bd2c38c43121753c5490f976cbc`). The full native prefix matches 14,856 complete 166-word checkpoints from 2,467 inputs (SHA256 `58839623985578e104ae82e8ad7bdaaa00fbf7fa156459e6916ed264f1be3669`). Both entire corpora match under ASan/UBSan.

All 144 frozen C64 checkpoints pass across four images, 36 each, mailbox `a500002400030c8623`. Target inputs are independently verified original pre-LOC03422 states; the target computes live trig/rotation. Maximum image/state end is $925c, text 23,209 bytes, additional zero page zero, hardware/software stack extents 38/293 bytes. Sampled complete timings span 321,988–440,901 PAL cycles, about 327–448ms; these are finite measurements rather than a complete application deadline proof.
