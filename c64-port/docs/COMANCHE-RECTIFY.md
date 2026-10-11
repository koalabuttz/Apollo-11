# Comanche RECTIFY and MINIRECT native milestone

`cm_rectify_phase()` translates the RECTIFY/MINIRECT groups in the unchanged
Comanche055 ORBITAL_INTEGRATION source into native C over the application's
canonical 2,048-word erasable image. It establishes the next conic from the
conic and deviation position/velocity vectors, clears both deviations and
resets TC and XKEP. This is a verified numerical and control milestone, not
the complete orbital integrator or either complete flight application.

The API executes twelve explicit source groups in order. RECTIFY begins at
phase 0; direct MINIRECT begins at phase 5 with vector MODE. Caller-owned
scratch and sensor producers must be excluded throughout each callback;
no private state survives a return. Phases 0–10 return at original DANZIG
(06030, EDOP +0), where the Executive may check for a higher-priority job.
Phase 11 returns directly to NEWOPS (06037) after RVQ: it does **not** introduce
an Executive job check. Re-entering a phase regathers source words after any
real scheduling boundary.

Supported entries have a legal Comanche VAC base (0401, 0455, 0531, 0605,
0661), BANKSET's fixed bank 11, and ordinary erasable words. PBODY is 0 for
Earth or 2 for Moon. The shift phases use saved X2 in the VAC rather than
rereading PBODY after a possible handoff. QPRET must hold an ordinary fixed
memory interpretive return address. Invalid documented preconditions reject
without mutation. Device and interrupt latches live outside the erasable
image and are retained by this numerical module.

The source scales the position deviation right by 7 + PBODY and the velocity
deviation right by 4 + PBODY; vector right shifts retain the original rounding.
The routine preserves raw signed zero, overflow handling, scratch aliases,
A/L exchanges, source bank registers and the physical contents of every
unowned erasable word.

| Phase | Source operation group | Returned LOC | Return |
| --- | --- | --- | --- |
| 0 | LXA2 PBODY; VLOAD TDELTAV | 03423 | DANZIG |
| 1 | Shift position deviation; VAD RCV | 03426 | DANZIG |
| 2 | STORE RRECT | 03427 | DANZIG |
| 3 | STOVL RCV, TNUV | 03431 | DANZIG |
| 4 | Shift velocity deviation; VAD VCV | 03434 | DANZIG |
| 5 | STORE VRECT | 03435 | DANZIG |
| 6 | STOVL VCV, ZEROVEC | 03437 | DANZIG |
| 7 | STORE TDELTAV | 03440 | DANZIG |
| 8 | STODL TNUV, ZEROVEC | 03442 | DANZIG |
| 9 | STORE TC | 03443 | DANZIG |
| 10 | STORE XKEP | 03444 | DANZIG |
| 11 | RVQ | QPRET's source subaddress | NEWOPS |

Physical operands are PBODY 02150, RRECT 01502–01507, VRECT 01510–01515,
TDELTAV 01520–01525, TNUV 01526–01533, RCV 01534–01541,
VCV 01542–01547, TC 01550/01551 and XKEP 01552/01553.
X2 is VAC +047 and QPRET is VAC +052. The source banked-memory addresses
are resolved as physical erasable offsets; no native pointer is stored in
flight memory.

The independent oracle executes the original assembly through yaAGC;
there are no native numerical callbacks in that oracle. The source fixture
normalizes historical pseudo-label indentation only and reproduces the pinned
complete Comanche rope SHA256
`2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79`.
The audit records each original source hash and the exact indentation patches.

Verification includes 2,048 source calls (1,536 RECTIFY and 512 MINIRECT),
24,064 complete 2,048-word input/return states, all five VACs, Earth/Moon,
raw signed-zero/limit/mixed-sign corners, deterministic random inputs, all
eight initial E-bank choices, fourteen fixed return-bank choices and 32
return subaddresses. Strict host C and the entire ASAN/UBSAN corpus match
original full images. Complete-state SHA256:
`7b93fc96fd7e845b9fec26c64cbfd48e96ba80dc472e1990cd21590381faee23`.

All twelve native C64 diagnostic images pass 32 original-source entry/exit
pairs each (384 pairs total), comparing all 4,096 bytes including unchanged
words. Source fixtures use lossless seed plus address/value deltas. Maximum
image end is 0557C; measured hardware/software stack extents are 20/170 bytes,
and no additional zero page is allocated. These are phase diagnostics;
a resident complete flight executable and application scheduling proof are
separate milestones.

Reproduce with `python3 tools/cm_rectify_differential.py`,
`python3 tools/freeze_cm_rectify.py`, then include `tools/cm_rectify.mk` for
`cm-rectify-host`, `cm-rectify-sanitize` and `cm-rectify-c64 PHASE=0..11`.
Run each image with `tools/cm_rectify_vice_test.py`, and finally run
`python3 tools/cm_rectify_report.py`. The compressed evidence streams contain
little-endian uint16 case, phase, and all 2,048 physical words; phase 12 denotes
input. Their decompressed full-state hashes equal the value above.

The twelve exact C64 binaries are retained as `evidence/cm-rectify-phase0.prg`
through `evidence/cm-rectify-phase11.prg`, with SHA256 identities in the full
evidence manifest. Main Makefile `test` and `sanitize` include this module;
the default standalone C64 phase is zero.
