# Native square root and normalized root

`p66_square_root` translates SQRT with triple input in MPAC 0..2 and DP
MODE. `p66_normalized_square_root` translates SQRTSUB for subsequent UNIT,
accepting DP or vector MODE and retaining normalization shift count. Original
Newton approximation, signed zeros, CYL/SR editing and raw register/scratch
stores are preserved. The new normalization scratch object owns 22 bytes.
Caller exclusively owns distinct ordinary-word contexts and excludes producers.
Unsupported MODE rejects unchanged.

Negative inputs stop before SQRTABRT and return an observation of alarm 1302.
This numerical milestone does not execute the Executive alarm/recovery path.
That error route must be connected by the native application continuation.

All 132,096 original-assembly comparisons match 20 raw modeled words:
66,406 root results and 65,690 pre-1302 stops. Cases cover every raw major
word, 512 triple corners, signed zeros, tiny normalization/long right shifts
and 33,024 normalized vector-MODE cases. Unrelated native fields are checked
separately for preservation. The reference uses unchanged original assembly
and the pinned hardware DV model, as qualified in DIVIDE.md; native numerical
code is excluded from its GPL executable. Snapshot SHA256:
`cb141db0803cd747bc022b65f779e71df0590eace82af5fe037ffb0fa0b311d2`.
The complete sanitizer snapshot stream also matches the reference.

637 frozen complete states pass host, ASan/UBSan and PAL C64 VICE.
Mailbox `a500007d02` records zero failures. PRG 35,536 bytes; image/state end
at $93bc below reserved $c000. No extra compiler ZP; measured hardware/software
stack extents 30/158 bytes. No flight timing or full-application budget is claimed.

Use `make test-norm`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-norm`,
`python3 tools/norm_differential.py`, `make c64-norm`, and
`tools/norm_vice_test.py` with standard VICE/ROM arguments.
Deterministic gzip complete snapshots/sanitizer stream and target evidence
are retained under `evidence/norm-*`. Earlier evidence hashes and historical
sources remain unchanged. Native code has no decoder, heap or floating point.
ABVAL/UNIT and composed position normalization follow as separate milestones.
