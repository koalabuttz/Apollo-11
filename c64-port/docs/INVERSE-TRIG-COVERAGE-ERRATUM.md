# Inverse-trig input coverage correction

The original `e764b97` qualification accurately compares316,640 complete source/native/sanitizer states. Its statement that every ordinary high-word input was covered is incorrect: the corner factory replaces the first4,096 ordinary cases, leaving30,723 distinct high words per function and omitting2,045. Ordinary MODE coverage is unaffected.

The original packet and all its identities remain unchanged. This correction takes precedence over the exhaustive-input wording in `INVERSE-TRIG.md`. Independent review found no concrete numerical or control discrepancy. A separate exhaustive supplement is being qualified; this document does not claim it complete. See `evidence/inverse-trig-input-coverage-erratum.json` and `evidence/inverse-trig-independent-review.txt`.
