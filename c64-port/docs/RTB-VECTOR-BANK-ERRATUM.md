# RTB PULSEIMU frontier bank metadata correction

The PULSEIMU frontier selects bank **07**, with FB **016000**, for both missions. The original containing inline CADRs are LM 17323 and CM 17206, reaching PCs 03323 and 03206.

This supersedes only the textual `bank06` statement in `docs/RTB-VECTOR.md` and the `FB014000` text in both `actual_boundaries.PULSEIMU` fields of `evidence/rtb-vector-source-map.json`. The frozen native implementation, source states, sanitizer states, C64 targets and proof identities already use the correct bank07/FB016000 and remain unchanged. The original metadata error did not affect arithmetic or execution.

The additive audit independently checks all 131,072 original complete PULSEIMU frontier states, including coherent BB and retained EB, and records unchanged byte identities. A separate independent reviewer confirmed both literal original words and every retained frontier state, with no numerical discrepancy. IMUPULSE body/hardware output remains outside the frontier packet.
