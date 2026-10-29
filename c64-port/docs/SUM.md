# Native scalar sums and backward vector subtraction

`guidance_sum.h` exposes DAD, DSU, BDSU and BVSU with stable ordinary-word
DP or six-word operands. The scalar backward form computes operand minus
MPAC, preserving the actual exchange/complement stores. BVSU processes Y,
Z, then X. Original DAS/OVERFLOW realignment, signed zeros, mixed signs and
A/L/BUF/OVFIND effects are preserved. MPAC+2, MODE, PIPTIME, MPTEMP and
operands remain unchanged. The APIs add no numerical context storage.
Caller owns distinct contexts/inputs, excludes producers and supplies valid
scalar/vector MODE. Unsupported MODE rejects without mutation.

All 27,200 complete 50-word checkpoints match unchanged Luminary assembly:
6,800 inputs with each of four operations. Original path observations
0/1/5/7 are covered in every active component and both sign families.
The independent oracle includes no native arithmetic and checks source
identity, control boundaries and neighboring guards. Snapshot SHA256:
`0e192daed2bc0a4e61994a757f2e29220190e7fd2c87722df7c4125bde466fe3`.
Host and complete-corpus ASan/UBSan pass. Comanche numerical equivalence
has not yet been independently asserted.

256 frozen complete states pass PAL C64 VICE; mailbox `a50000000103643224`.
Diagnostic context 100 bytes / 50 words; image/state end at $843e, no extra
compiler ZP. Measured hardware/software stack extents 27/222 bytes.
Finite PAL samples: DAD resolved overflow 1,476; DSU 1,480; BDSU 1,319;
BVSU overflow 3,807; BVSU random 2,816 cycles. These include markers/VIC/IRQ
effects and establish no worst-case or flight budget.

Use `make test-sum`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-sum`,
`python3 tools/sum_differential.py deps/fixture`, `make c64-sum` and
`tools/sum_vice_test.py` with standard VICE/ROM arguments.
Full snapshots are deterministic gzip under `evidence/sum-*`; logs,
PRG/map, timings and mailbox/stack dumps are retained. Earlier evidence hashes
and historical sources remain unchanged. Native code has no AGC decoder.
