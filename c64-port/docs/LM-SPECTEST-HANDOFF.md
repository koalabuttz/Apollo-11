# Executive SPECTEST exception handoffs

`lm_spectest_handoff` executes the literal source prefixes that the ordinary admission/wake wrappers exclude: admission into free core zero, or waking dormant core zero, when NEWJOB is already +0. It also accepts the literal SPECTEST entry at 02651 for +0 and negative nonzero flags. Every route stops before the actual CCSHOLE body at 05705.

Admission preserves the true descriptor, reserves the real VAC when needed, prepares packed priority/PUSHLOC/FIXLOC/OVFIND, and leaves the old core-zero LOC/BANKSET unchanged. Wake prepares the real wake pair and re-complements priority but also leaves the old sleeping core-zero pair unchanged. The +0 CCS branch arrives at CCSHOLE with Q02654/A0. The direct negative-flag branch arrives with Q02655 and the actual diminished absolute flag. The exceptional flags are retained. EXEC_BAILOUT_PENDING denotes this source transfer, not a made-up alarm or recovery. There is no lifecycle call, additional source poll, decoder or suspended native frame.

The independent unchanged assembly proof contains 16,384 complete 2048-word images plus SUPER/IRQ, spanning four admission entries, JOBWAKE and both direct SPECTEST branches. Strict and full ASAN/UBSAN outputs match every original word. Another 13,654 admission/wake cases confirm that the earlier frozen wrappers still reject these domains without mutation. Twenty-one mutation guards and two null interfaces preserve all supplied canonical state/I/O.

The stock PAL C64 ROM/D81 diagnostic checks 64 frozen original-state fingerprints using 32 input/checker modules, returning every frame before replacement. Its mailbox is `a50000400002`, measured software/hardware stack 56/33 bytes, additional zero page zero, with native I/$01 restored. The sole source image remains at $7000.

The entry contracts require canonical caller banks, a lawful free/owned running slot appropriate to the selected route, non-overflowing admission priorities and valid VAC markers. Other admission and wake routes remain in their separately frozen groups. CCSHOLE/ABORT2 recovery belongs to ALARM_AND_ABORT and remains an explicit continuation outside this packet. This closes these actual Executive exception words without certifying the whole file.

Fresh-checkout targets: `lm-spectest-handoff-port-test` and `lm-spectest-handoff-port-sanitize` in `tools/lm_spectest_handoff_port_integration.mk`; retained originals are streamed in private builds and all fixture descriptors checked.
