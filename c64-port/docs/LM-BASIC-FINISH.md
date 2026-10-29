# Canonical LM ENDOFJOB

`lm_basic_finish` translates actual ENDOFJOB05155 through ENDJOB1/EJSCAN
and the selected basic DTCB, interpreter INTRSM (before NEWOPS fetch), or
idle ADVAN03214 after the original channel11 activity-light output. It
releases the source core/VAC, preserves full source scan and CPU register
debris, restores selected banks/SUPER, and returns an explicit `exec_transfer`.
The idle address is {0,0}; an interpreter address is the positive actual next
compiled program entry while canonical Z remains source NEWOPS06070.

The caller owns producer exclusion and distinct image/io/channel/next objects.
The header defines the admitted current VAC and selected context domains.
Unsupported inputs preserve every owned object. The helper reuses the shared
native context exchange: a selected negative interpreted LOC is converted
only in the transient typed view, then the original INTRSM canonical effects
are committed. Raw priority1 cannot win the empty source CCS scan and remains
unchanged in unselected contexts. Native code contains no AGC instruction
decoder or emulated CPU.

`tools/lm_finish_differential.py` compares16,384 complete original Luminary099
2,051-word states. Cases include all EB/FB/SUPER values, caller Q raw16-bit
patterns, seven selected slots, ties, raw priorities1/2, both push/overflow
signs, all five admitted VAC bases, and all output routes:3,206 interpreted
NEWOPS,4,160 idle ADVAN, and9,018 basic DTCB. SHA256:
`e4c3eebee9c76eef5c54c5badad7efec475ba0f7ccaa2e55546e60d6336b8500`.
Strict checks cover64 frozen complete images, returned transfer metadata and
21 unchanged invalid boundaries. All source states agree under ASAN/UBSAN.
Actual C64 PAL VICE passes all64 states: mailbox`a500004000`, PRG15,996
bytes, no extra zero page, measured hardware stack17 bytes plus14 ROM bytes,
software stack694 bytes. Recipes are in `tools/lm_finish.mk`.

This body finishes at real dispatch boundaries. It does not execute the next
selected job, fetch interpretive opcodes or run idle SELFCHK. The source/artifact
allowlist is `evidence/lm-finish-proof.json`; full original output streams and
target artifacts remain available under the paths recorded there.
