# P66 updated velocity through position load

`p66_update_velocity` composes the original source group after PDVL GDT/2:
VSU VBIAS; implicit quotient pop/VXSC; VSL2; VAD V; implicit scaled-vector
pop/VAD; STADR/STOVL24D and load R. It ends at DANZIG before UNIT.
Input vectors are distinct stable ordinary-word VBIAS, V and R. Caller
exclusively owns state/scratch/VAC and excludes producers. MODE must be vector
and PUSHLOC exactly FIXLOC+8 in a historical VAC. Unsupported mode/base/pointer
rejects without changing any object.

The quotient pop consumes two words; the scaled-vector pop consumes six,
leaving PUSHLOC=FIXLOC and popped payload intact. Updated velocity occupies
VAC24..29; saved elapsed time30/31 is retained. R loads into MPAC's three
DP pairs. A=-1; L receives updated-velocity Z low. Original intermediate
arithmetic, MPAC+2, MODE, BUF/MPTEMP/OVFIND and division scratch are preserved.
Context plus three stable input vectors is117 words/234 bytes.

All 4,648 complete117-word checkpoints over3,475 inputs match unchanged
original assembly. Direct group and full RODCOMP prefixes include all 128
preparation skip inputs, three four-step histories,256 corners,2,048 direct
and 1,024 integrated random inputs. Full-prefix oracle uses the pinned hardware
DV model from DIVIDE.md; no native numerical code links into the oracle.
Snapshot SHA256:
`1beae8e1b464988bea4315835d90ca18e0281f4274f19335adeafdc5b406be39`.
Original source/bank/pointer/control boundaries and neighboring guards are checked.

156 frozen complete states pass host and ASan/UBSan, including all unsupported
ordinary modes/bases/pointers without mutation. Target expectations are split
losslessly across two RAM-safe C64 PRGs:80 complete states in half A,76 in B.
Together they cover every selected state, skip input and retained history.
PAL VICE mailboxes `a50000500003ea7524` / `a500004c0003ea7524` record zero
failures. PRGs 38,340/37,395 bytes; image/state highest addresses$9ead/$9afc,
below reserved$c000. Extra compiler ZP is zero; measured hardware/software
stack extents 50/412 bytes. Group-only samples 27,606..48,741 PAL cycles; full
prefix samples 75,952/73,151 cycles (77.09/74.25 ms). Finite samples include
markers/VIC/IRQ effects; no worst-case or flight deadline is established.

Use `make test-velocity`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-velocity`,
`python3 tools/velocity_differential.py deps/fixture`, `make c64-velocity`,
and `tools/velocity_vice_test.py --half 0` / `--half 1` with standard ROM/VICE
arguments. Complete snapshots are deterministic gzip under evidence/velocity-*;
only the final half-A/B binaries/maps are retained. Earlier evidence hashes and
historical sources remain unchanged. Next is position normalization, radial
velocity and altitude; the full P66/LM/CM applications remain in development.
