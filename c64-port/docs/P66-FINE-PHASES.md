# P66 feedback pairs and horizontal overlay phases

The new native feedback module exposes every completed source pair from LOC03433 through LOC03507. All 296,644 complete 137-word numerical/control states from 16,384 source inputs match the unchanged Luminary assembly, including IRQ enabled and disabled, both clamp branches, division overflow/zero returns, and branch register debris. The whole sanitizer corpus also matches. Twenty C64 images each check 32 original pair entry/exit states (640 total).

The source SETPD operation returns at **NOIBNKSW06063, LOC03506, EDOP0**, directly admitting the Executive check. It bypasses DANZIG06061. The preceding branch targets go through NEWOPS and continue to their next actual check; their jump alone does not admit a job. ABVAL's intermediate DANZIG03437 retains pending PDDL (EDOP0052), so ownership spans all internal length overlays and the timestamp suffix until LOC03440.

`p66_feedback_pair` uses an explicit native pair number, actual entry control words, the same source numerical scratch and VAC, and explicit stable operands. The returned `next_pair` is compiled continuation metadata, outside historical memory. Source POLISH0117 is included because the clamp branch dispatch writes065504 even when the branch is not taken. Producers must remain excluded while a numerical callback owns these objects.

| Pair | Operations | Completed LOC | Next pair |
|---|---|---|---|
| 0 | VXSC KPIP1; VAD VBIAS | 03436 | 1 |
| 1 | ABVAL; PDDL THISTPIP | 03440 | 2 |
| 2 | DSU LASTTPIP; PDDL THISTPIP | 03443 | 3 |
| 3 | STODL LASTTPIP; pop elapsed interval | 03444 | 4 |
| 4 | DDV SHFTFACT; pop/BDDV measured magnitude | 03446 | 5 |
| 5 | PDDL FWEIGHT; DMP BIT1H | 03451 | 6 |
| 6 | DDV MASS; DDV SCALEFAC | 03454 | 7 |
| 7 | DAD saved desired acceleration; PDDL saved gravity | 03457 | 8 |
| 8 | DDV saved projection; pop/DSU | 03461 | 9 |
| 9 | DMP LAG/TAU; DAD AFC | 03464 | 10 |
| 10 | PDDL MAXFORCE; DDV MASS | 03467 | 11 |
| 11 | PDDL MINFORCE; DDV MASS | 03472 | 12 |
| 12 | PUSH; BDSU requested acceleration | 03474 | 13 |
| 13 | BMN; pop/load minimum, or branch/load minimum | 03476 or 03504 | 14 or 18 |
| 14 | Pop/load maximum; PUSH | 03477 | 15 |
| 15 | BDSU requested acceleration; BPL, with branch/load maximum | 03502 or 03504 | 16 or 18 |
| 16 | Pop/load maximum | 03503 | 17 |
| 17 | Pop/load requested acceleration | 03504 | 18 |
| 18 | SETPD VAC2 | 03506 | 19 |
| 19 | STODL AFC; pop/load measured acceleration | 03507 | done |

Pair1 is retained for bounded numerical compatibility. The live overlay caller instead invokes `p66_length_square_phase`, `p66_length_root_phase`, `p66_length_finish_phase`, and `p66_feedback_timestamp_after_length`. This avoids coalescing the real LOC03440 handoff and does not introduce checks inside basic square-root code. All 81,920 complete 137-word states (16,384 inputs, five checkpoints) match original assembly and the whole sanitizer stream; 128 additional C64 original entry/exit states pass.

The horizontal extension resumes the pending VXSC via `p66_horizontal_radial_product` without repeating UNIT. The existing generic UNIT square/root/finish APIs already retain an arbitrary source PUSHLOC, including actual horizontal FIXLOC+6. The new generic length phases likewise retain PUSHLOC. All 113,993 complete 132-word states from 8,192 inputs match source and the whole sanitizer stream, including 695 UNIT early overflow paths that bypass the normalized-root return and 72 overflow paths after that return. Thirteen C64 images each check32 complete original phase entry/exit states (416 total).

| Horizontal return | PC | Q | FB / EB / BB | LOC | CYR | EDOP |
|---|---|---|---|---|---|---|
| Load velocity | 06061 | retained | 064000 / 03400 / 064007 | 03516 | 020000 | 0 |
| Surface velocity | 06061 | retained unless VSL/VAD overflow, then3 | same | 03520 | 020024 | 0 |
| Push velocity/load position | 06061 | 3 | same | 03522 | 020014 | 0 |
| UNIT square | 03031 | 0140 | 0 / 03400 / 7 | 03523 | 5 | 0016 |
| UNIT normalized root | 03043 | 3 | same | 03523 | 5 | 0016 |
| UNIT finish | 06061 | 3 or02413 on overflow | same | 03523 | 5 | 0016 |
| Pending VXSC | 06061 | 3 | 064000 / 03400 / 064007 | 03524 | 020003 | 0 |
| Tangential velocity | 06061 | 3 | same | 03525 | 020026 | 0 |
| ABVAL square | 03202 | 0140 | 0 / 03400 / 7 | 03526 | 040005 | 0 |
| ABVAL normalized root | 03210 | 3 | same | 03526 | 040005 | 0 |
| ABVAL finish | 06061 | 3 | same | 03526 | 040005 | 0 |
| Store VHORIZ | 06061 | 3 | 064000 / 02000 / 064004 | 03527 | 040005 | 0 |
| Numeric EXIT | 03531 | 06747 | 064000 / 03400 / 064007 | 03530 | 0 | 0 |

All horizontal returns retain BANKSET064007, INTBIT15040000, POLISH and IRQ. The UNIT internal returns retain pending VXSC and never admit an Executive check. Feedback ABVAL internal controls are the same basic PC/Q/bank/CYR values as horizontal ABVAL, with source LOC03437 and pending EDOP0052. Its timestamp suffix returns PC06061/Q3/FB064000/EB03400/BB064007/LOC03440/CYR020012/EDOP0/ADDRWD01761.

Original-reference runners use the pinned hardware-DV engine variant documented by `tools/prepare_hardware_dv.py`; they never invoke native arithmetic. Original source and rope identities, complete compressed source/native/sanitizer streams, actual37 target PRGs, maps, mailboxes and measured stacks are retained in `evidence/p66-fine-phases-proof.json` and the SHA256 manifest. The source numerical records compare every owned word; separate canonical-driver proofs compare the entire source image including every unowned word and scheduling continuation.

Reproduce with `python tools/prepare_hardware_dv.py`, `make -f tools/throttle_feedback_pairs.mk test check-host length-test length-check-host sanitize length-sanitize`, and `make -f tools/horizontal_phased.mk test check-host sanitize`. Regenerate frozen fixtures using the three `tools/freeze_*` scripts, then build target selectors with the standalone makefiles and run their matching VICE runners. All tooling defaults to selector0 when omitted. These arithmetic extensions support live scheduling integration; their standalone diagnostics do not certify a complete resident flight application or complete Luminary/Comanche source coverage.
