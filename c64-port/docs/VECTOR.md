# Native vector addition and subtraction

`p66_vector_add` and `p66_vector_subtract` translate original VAD/VSU through
DANZIG. They process Y, Z, then X; retain MPAC+2 and vector MODE; preserve
ordinary-word signed zeros, mixed signs, low carry and original OVERFLOW
realignment. BUF and OVFIND change only through their historical paths.
A/L retain the final component effects. Caller owns distinct preparation,
scale/elapsed scratch and stable six-word operand, and excludes producers.
Unsupported MODE rejects unchanged. Other raw-word/ownership requirements
are caller obligations. Result bits observe original DAS and TS skips.

All 13,600 checkpoints (6,800 inputs with both operations) match all 50
modeled words and nine overflow observations against unchanged original
assembly. The oracle checks eight surrounding guards, source identity and
VSU6747/VAD6751/OVERFLOW7017/DANZIG6061 boundaries; native arithmetic is
excluded from the reference executable. Snapshot SHA256:
`406e7bd75f6baaf3c6ddfdc3218d1aae32099249820be3073be53b9bb5d321a9`.
Independent code review found no blocking defect.

128 frozen complete states pass host, ASan/UBSan and PAL C64 VICE.
All 32,767 unsupported modes preserve every owned word for both APIs.
C64 mailbox `a50000800003643224` records zero failures. PRG20,579 bytes,
text7,381/rodata13,184/BSS100/noinit1, ending at $58c6; no extra compiler ZP.
Measured hardware/software stack extents31/231 bytes; finite call samples
2,585/2,284/2,332/2,923/2,890 PAL cycles include marker and VIC/IRQ effects.
These samples establish neither worst-case execution time nor a flight budget.

Use `make test-vector`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-vector`,
`python3 tools/vector_differential.py deps/fixture`, and `make c64-vector`.
Run `tools/vector_vice_test.py` with the usual VICE/ROM arguments.
Retained `evidence/vector-*` includes deterministic gzip full snapshots,
logs, PRG/map, timings and target dumps. All239 earlier hashes are preserved;
manifest256 entries. Historical source remains unchanged.

The composed P66 updated-velocity sequence and complete flight applications
remain separate milestones. These are reusable direct C operations; native
code contains no AGC decoder or linked GPL oracle engine.
