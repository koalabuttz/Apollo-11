# Native scalar and vector shifts

`guidance_shift.h` exposes direct scalar/vector left/right operations, with
explicit count and general/short form. General counts0..14, short scalar1..4
and short vector1..8 preserve the original distinct control effects. Scalar
right shifts preserve TP low scratch without rounding. Vector right shifts
round; vector left shifts do not. Left shifts retain original overflow
realignment. Unsupported MODE/count/form rejects without mutation.
Caller exclusively owns ordinary-word state and scratch and excludes producers.

All26,880 complete89-word checkpoints match unchanged GENSHIFT and genuine
SHORTT/SHORTV dispatch. Cases include64 triple raw corners and256 random
inputs per operation/count family. Snapshot SHA256:
`7061c42a1e7fbe1837fd084bb58e1fa19416f82bcab4b6513b5e3b1b622c04a3`.
General scalar SL leaves A7; short SL leaves A027777-count. General SR14
retains prologue L03400 and MPTEMP=-0. Vector SR14 retains its final rounded
low scratch in MPAC+2. These stores are preserved, not normalized away.

Host and ASan/UBSan pass, including the complete differential corpus.
168 frozen complete states pass PAL C64 VICE; mailbox `a50000a80003b55924`.
Image/state end at $9df7, no extra compiler ZP; measured hardware/software
stack extents27/274 bytes. The native context adds no storage; three bytes
of count/form metadata belong only to the diagnostic. Finite PAL samples:
SL1=1,431; SR14=442; VSL2=8,697; VSR14=3,528; short VSR8=10,782 cycles.
These include marker/VIC/IRQ effects and are not worst-case or flight budgets.

Use `make test-shift`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-shift`,
`python3 tools/shift_differential.py deps/fixture`, `make c64-shift` and
`tools/shift_vice_test.py` with standard VICE/ROM arguments.
Full snapshots are deterministic gzip under `evidence/shift-*`; logs,
PRG/map, timing and mailbox/stack dumps are retained. Earlier evidence hashes
and historical sources are unchanged. Native code has no instruction decoder.
