# Native unrounded triple-precision dot product

`p66_dot` translates DOT through DANZIG with vector input and stable
six-word operand. It preserves original unrounded triple accumulation and
sets MODE to DP. The later P66 SL1 shifts that triple low word; DOT itself
does not round. MPAC exchange debris, intermediate/final overflow correction,
BUF pair/third, MPTEMP, OVFIND, DOTINC and DOTRET are retained. DOTINC0136
aliases division DVSIGN; DOTRET0137 aliases DVNORMCT. The supplied BUF-third
pointer allows reuse of normalization scratch; two additional numerical
bytes are required before that shared scratch is integrated.

Caller owns distinct ordinary-word state/scratch/operand and excludes
producers. Unsupported MODE rejects unchanged. No interpreter dispatch is
introduced. The oracle separately asserts source ADDRWD01400->01404 and
unchanged banks; those control words are outside the native data interface.

All 10,272 complete100-word checkpoints match unchanged original assembly.
There are4,096 six-word corners with same/opposite vectors and 2,048 random
input pairs. Intermediate overflow has1,056 positive/1,059 negative cases;
final overflow1,610 positive/1,580 negative. Snapshot SHA256:
`0a4638514865244d92a22331253a946794fdf3461991d59dbb6c46c39597ae15`.
Host and complete-corpus ASan/UBSan pass. The reference has no native numeric
link, and historical source remains unchanged.

96 frozen complete states pass PAL C64 VICE; mailbox `a50000600003c86424`.
Diagnostic state200 bytes includes12 operand bytes; image/state end at$72ab.
No extra compiler ZP; measured hardware/software stack extents 29/294 bytes.
Five finite PAL samples 15,200/20,620/31,575/31,974/28,551 cycles include
markers/VIC/IRQ effects and establish no worst-case or flight budget.

Use `make test-dot`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-dot`,
`python3 tools/dot_differential.py deps/fixture`, `make c64-dot`, and
`tools/dot_vice_test.py` with standard VICE/ROM arguments. Retained
`evidence/dot-*` includes deterministic gzip full snapshots, logs, PRG/map,
timings and mailbox/stack dumps. Earlier evidence hashes are preserved.
