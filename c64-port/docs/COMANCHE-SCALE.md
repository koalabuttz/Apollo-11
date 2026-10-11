# Comanche fresh decimal output scaling source qualification

`cm_scale_output` executes actual Comanche055 DP1OUTSF40,2614, DP2OUTSF40,2621, or DP3OUTSF40,2623 through fresh READLO, TPAGREE, multiplication and rescaling to DSPDCEND41,2566. The fixed native arithmetic/store body is shared after independent actual CM source execution verifies all physical words. Literal CM caller stores remain2615/2622/2624 in MPAC+6 when TPLEFTN has not replaced that word,2632 in TEM1 for READLO, and2626 in SFTEMP2 for DP3's TPLEFTN call. No original address word is decoded as a native opcode or callback.

The qualified source domain is bank40/SUPER100 with any retained EB0..7 and the fresh-input domains documented by `dsky_scale_output`. MIXBR1=1 selects a normal NOUNADD logical DP pair; MIXBR1=2 selects an IDAD1TEM physical ECADR pair1000..3776 using DECOUNT0..2. Pairs must remain inside their erasable bank. Scale-factor words are ordinary source data. Unsupported inputs leave source state and I/O unchanged; the caller owns exclusion.

The independent original-assembly oracle covers4096 complete checkpoints, including all three components, all retained erasable banks, physical and logical input selection, signed zeros, sign disagreement, fractional shifts, scale-factor and multiplication boundaries, and overflow-dependent MPAC+6 stores. Every2048 physical word plus SUPER/IRQ matches:2050 fields per checkpoint. Original/native snapshot SHA256:
`10bd36aa4fee086c81a3227964b3d38c276c90658cc2e36408298c043d21f3c5`.
The complete ASan/UBSan stream matches byte for byte. The oracle links only original engine code and independent input construction. Pinball, Inter-bank Communication and Interpreter original source bytes and indentation-only assembler changes are audited in `build/cm-scale-source-audit.json`; the assembled rope must equal the separately pinned CM rope.

Frozen64 complete original source images pass strict host, sanitizer and native C64. VICE PAL mailbox `a500004002`; PRG14581 bytes (loaded14579); hardware/software stack19/151 bytes. Target section/PRG bounds remain belowC000 with no extra compiler zero-page storage.

Evidence uses `build/cm-scale-*` and `build/cm_scale-*`, including compressed full streams and the original source assembly listing. `tools/cm_scale.mk` exposes portable host-only all/test/sanitize and explicit c64-cm-scale/differential-cm-scale targets. This body stops at the real decimal caller boundary; noun selection, application control and queued CM display composition remain separate required work.
