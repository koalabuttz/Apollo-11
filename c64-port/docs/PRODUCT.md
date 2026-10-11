# Native DP multiplication and vector scaling

`p66_vector_multiply` translates arbitrary-factor VXSC and per-component
VROUND. `p66_scalar_product` translates unrounded DMP1, retaining the third
product word in MPAC+2 and DP MODE. Both preserve original one's-complement
partial products, mixed signs, signed zeros, MPTEMP/OVFIND and A/L effects.
The vector path rounds as the original; the scalar path does not round.
Caller supplies an ordinary DP factor and exclusively owns distinct state
and scratch, with producers excluded. Unsupported MODE rejects unchanged.

All 8,192 complete50-word checkpoints match unchanged VXSC7401/DMP17572
assembly through DANZIG6061. The reference executable contains no native
numerical code and checks source identity, control boundaries and surrounding
guards. Snapshot SHA256:
`c361603c0682599e61e606db217305f9ae3b8b44d09432554cf1f343456ab35d`.
There are7,238 VROUND returns without skip,5,050 rounding-bit skips and 10
low-word carry skips; high-word rounding overflow was not observed.
An independent arithmetic/oracle code review found no blocking defect.

128 frozen complete states pass host, ASan/UBSan and PAL C64 VICE;
all 32,767 unsupported MODE words reject unchanged for both APIs.
Mailbox `a50000800003643224` records zero failures. PRG 22,356 bytes:
text 9,397/rodata 12,945/BSS 100/noinit 1, ending at $5fb7; no extra compiler ZP.
Measured hardware/software stack extents 29/229 bytes. Five finite call
samples 3,385/10,919/5,249/21,733/8,830 PAL cycles include marker/VIC/IRQ
effects and do not establish a worst-case or application deadline.

Use `make test-product`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-product`,
`python3 tools/product_differential.py deps/fixture`, `make c64-product`,
and `tools/product_vice_test.py` with standard ROM/VICE arguments.
`evidence/product-*` retains deterministic gzip full snapshots, logs,
PRG/map and target dumps. Earlier evidence and historical sources are unchanged.
Native code contains no instruction decoder or linked GPL engine.
