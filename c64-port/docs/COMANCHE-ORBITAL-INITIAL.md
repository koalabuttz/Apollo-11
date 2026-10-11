# Comanche DIFEQ0 initialization native milestone

`cm_orbital_initial_phase(image, interrupt_enabled, phase)` translates the
original DIFEQ0 initialization and its JSWITCH BON/GOTO route into four native
source phases over the canonical 2,048-word Comanche image. A separate
caller-owned 0/1 interrupt latch models source IRQ state. It loads the trial
position from YV, sets DIFEQCNT to positive zero, copies the vector into
ALPHAV, clears H and selects ACCOMP or DOW from JSWITCH. The selected routines
and the complete orbital integrator remain separate milestones.

| Phase | Source group | Returned LOC | Source return |
| --- | --- | --- | --- |
| 0 | VLOAD YV; SSP DIFEQCNT, 0 | 03310 | DANZIG 06030 |
| 1 | STODL ALPHAV, DPZERO | 03312 | DANZIG 06030 |
| 2 | STORE H | 03313 | DANZIG 06030 |
| 3 | BON JSWITCH, DOW; GOTO ACCOMP | 03665 or 02512 | NEWOPS 06037 |

Phases 0–2 end at genuine EDOP +0 job-check boundaries. Phase 0 retains the
incoming Q: its load/SSP sequence does not call a numerical basic subroutine.
Phase 3 returns directly to NEWOPS, introduces no Executive check and enables
interrupts through the original SWITCHES RELINT. Source SWBIT 0131 becomes
020000 and SWWORD/BUF/INDEXLOC 0130 becomes +0. Q receives STATE 0074; the
state word itself is retained. A/L, bank registers, saved BANKSET and MODE
retain the source exchanges and control effects.

Valid entries have a legal Comanche VAC (0401, 0455, 0531, 0605, 0661), fixed
bank 11 in BANKSET, EDOP +0 and the phase's source MODE (vector for phase 1,
DP for phases 2/3; phase 0 loads independently of previous MODE). Invalid
scope rejects without changing either image or IRQ latch. All scratch and
sensor producers must be excluded during the callback. Nothing persists in
private native state, and no pointer is stored in the flight image.

The independent oracle executes the unchanged assembly, recording all
2,048 words and the IRQ latch at each phase. The pinned original whole rope
SHA256 is `2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79`;
the orbital source fixture audit records original hashes and indentation-only
assembly repairs. There are no native numerical callbacks in that oracle.

Strict host C and the full ASAN/UBSAN corpus match 5,120 complete source
input/return states from 1,024 calls, covering all five VACs, all eight initial
E-banks, both initial IRQ states, both JSWITCH routes, raw signed-zero/limit
corners and deterministic random words. Complete-state SHA256:
`5a0688f511a9c6097a5aeafe9b8ac628c6ff9598345b029c40e0471e0d1cb37c`.
Four C64 phase diagnostic images compare all 4,096 physical bytes plus IRQ
for 32 independent original entry/exit pairs each (128 pairs total). Fixture
compression is lossless input-seed plus exact before/after word deltas.
Measured maps, stack extents, zero-page use and exact target PRGs are retained
in the evidence package; these are phase diagnostics, not a resident complete
application proof.

Reproduce with `tools/cm_orbital_initial_differential.py` and
`tools/freeze_cm_orbital_initial.py`, then include
`tools/cm_orbital_initial.mk` for host, sanitizer and C64 diagnostic targets.
Build phases 0–3, run `tools/cm_orbital_initial_vice_test.py`, then
`tools/cm_orbital_initial_report.py`. Compressed streams contain little-endian
uint16 case, phase, every physical word and IRQ; phase 255 denotes input.
