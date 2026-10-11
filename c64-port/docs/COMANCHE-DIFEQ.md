# Comanche Nystrom contributions native milestone

`cm_difeq_phase(image, stage, phase)` translates the original Comanche055
ORBITAL_INTEGRATION Nystrom integration contributions into native C. Stages
0 and 1 run DIFEQ+0/+1 and their common DIFEQCOM continuation through STORE
ALPHAV; stage 2 runs DIFEQ+2 through STORE ZV. The phase counts are 9, 12 and
9. Control following those stores, force generation and the complete orbital
integrator remain separate milestones.

The routine computes the first and midpoint acceleration contributions,
advances H and DIFEQCNT, forms the next trial position ALPHAV, then applies the
final Nystrom position and velocity updates to YV/ZV. Source DP2/3 is the
original fixed-word pair 025252/025253. Arithmetic retains original rounded
multiplication, vector shift rounding, raw signed zero, mixed word signs,
overflow behavior and all scratch/register exchanges. Scalar-times-vector
DVXSC follows its own source exchanges and operand order; it does not replace
that path with a value-only commutative vector multiply.

The API modifies the caller's canonical 2,048-word erasable image. It stores
no pointer or private continuation in flight memory and retains no state
between callbacks. The caller excludes scratch and input producers during a
callback and regathers canonical state after a real source scheduling return.
Required entries have one of the five Comanche VAC bases, BANKSET fixed bank
11, EDOP +0 and the source phase's MODE/push pointer. Ordinary memory words
and the A/L raw register bits are preserved. Unsupported scope rejects
without mutation. Device, superbank and interrupt latches remain external
caller-owned state.

Stages 0/1 have an internal STCALL into DIFEQCOM at NEWOPS 06037. Those
returns do **not** permit an Executive job check. Every other exported phase
ends at the original DANZIG 06030 with EDOP +0 and permits the source job
check. Returned LOC and physical bank/register state are committed before
return. A callback return is not permission to introduce a new scheduling
point inside STCALL.

| Stage | Phase | Returned LOC | Last source group |
| --- | --- | --- | --- |
| 0 | 0 | 03447 | VLOAD FV; VSR3 |
| 0 | 1 | 03626 | STCALL PHIV, DIFEQCOM; NEWOPS |
| 1 | 0 | 03453 | VLOAD FV; VSR1 |
| 1 | 1 | 03455 | PUSH; VAD PHIV |
| 1 | 2 | 03456 | STOVL PSIV; implicit saved-vector load |
| 1 | 3 | 03460 | VSR1; VAD PHIV |
| 1 | 4 | 03626 | STCALL PHIV, DIFEQCOM; NEWOPS |
| 0/1 | 2/5 | 03630 | DLOAD DT/2; DAD H |
| 0/1 | 3/6 | 03633 | INCR1 -12; SXA1 DIFEQCNT |
| 0/1 | 4/7 | 03634 | STORE H |
| 0/1 | 5/8 | 03636 | Scalar VXSC FV; VSR1 |
| 0/1 | 6/9 | 03641 | VAD ZV; VXSC H |
| 0/1 | 7/10 | 03643 | VAD YV |
| 0/1 | 8/11 | 03644 | STORE ALPHAV |
| 2 | 0 | 03465 | DLOAD H; DMPR DP2/3 |
| 2 | 1 | 03467 | PUSH; scalar VXSC PHIV |
| 2 | 2 | 03471 | VSL1; VAD ZV |
| 2 | 3 | 03474 | VXSC H; VAD YV |
| 2 | 4 | 03476 | STOVL YV, FV |
| 2 | 5 | 03500 | VSR3; VAD PSIV |
| 2 | 6 | 03501 | VXSC saved scalar; VSL1 |
| 2 | 7 | 03503 | VAD ZV |
| 2 | 8 | 03504 | STORE ZV |

Physical operand addresses are DIFEQCNT 01500, ALPHAV 02151–02156,
PHIV 02165–02172, PSIV 02173–02200, FV 02201–02206, H 02211/02212,
DT/2 02313/02314, YV 01111–01116 and ZV 01117–01124.
MPAC/MODE, A/L, MPTEMP, OVFIND and BUF/INDEXLOC retain their original
physical aliases. ADDRWD holds source bank-relative addresses and opcode
bits where the original leaves them; it is not a native pointer.

The independent oracle executes the unchanged original assembly through
yaAGC, stopping at every listed source boundary and recording all 2,048
physical words. The original orbital fixture's complete rope hash is
`2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79`;
its audit records original files and indentation-only assembly repairs.

Strict host C and ASAN/UBSAN match all 19,840 complete images from 1,792
source calls. The primary 1,536 calls cover the three source stage indices,
all five VAC bases, all eight initial E-bank choices, signed-zero/limit/mixed
corners and deterministic random numerical state. An additional 256 direct
stage-0/1 entry calls exercise raw index and INCR overflow corners; those
are independent routine-entry tests, not dispatcher reachability evidence.
Complete-state SHA256:
`9b8b544ee7eac53ad9ac77fa6f27e4d980a23b0e3dcf22d7843e9ff4607a3ecf`.

Thirty separate native C64 diagnostic images check 32 exact original
source entry/exit pairs each, including all unchanged words (960 pairs total).
Fixtures encode complete states losslessly as input seeds and address/value
deltas. Maximum image end is 05F36; measured hardware/software stack extents
are 20/177 bytes, with no added zero page. Diagnostic maps and full extents
are retained in the proof report; these phase diagnostics do not establish a
resident complete application or the unimplemented adjacent source control.

Reproduce with `tools/cm_difeq_differential.py`, `tools/freeze_cm_difeq.py`,
then include `tools/cm_difeq.mk` for `cm-difeq-host` and
`cm-difeq-sanitize`. Build `cm-difeq-c64 SELECTOR=stage*16+phase` for each
valid phase, run `tools/cm_difeq_vice_test.py`, then
`tools/cm_difeq_report.py`. Compressed full-state streams contain little-endian
uint16 case, phase and every physical word; phase 255 denotes input.
