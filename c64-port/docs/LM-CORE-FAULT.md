# LM Executive core-priority fault aliases

`lm_core_fault` executes the literal NOVAC, FINDVAC, SPVACIN, SPVAC and JOBWAKE preparations through a +0 core-priority fault. The source admission scan then executes the immutable NO.CORES word00007 as TC7; the wake scan executes COREINC word00014 as TC014. The returned canonical PC is respectively7 or014, with Q02635 or03034. EXEC_UNMAPPED marks the actual reserved-code frontier; it does not invent an alarm, decode the register word at that address, or claim recovery.

Admission retains the actual staged job pair, packed priority, selected/reserved VAC, caller-bank scratch, scan displacement and diminished down-counter. Wake retains its staged CADR and scan state. Every preceding admission core is occupied; every preceding sleeping wake candidate is a non-match. A prior free admission core or actual wake match is rejected without mutation because that source route reaches another qualified body first. Superbank remains unchanged, source interrupt enable is zero, and A/L/Q, EB/FB/BB and every other canonical word match the original source at the literal transfer.

The original assembly proof compares16,384 full2048-word states plus SUPER/IRQ across all five public entries and all eight fault slots, positive/negative preceding priorities, all five legal VACs, raw low-priority bits, erasable inline pairs and EB/SUPER forms. Strict C and full ASAN/UBSAN match the complete stream. All16,384 prior frozen admission/wake guards remain unchanged;23 mutation guards and two null interfaces preserve all supplied state.

Stock PAL C64 ROM/D81 execution checks64 frozen original complete-state fingerprints through32 returned fixture modules. Native I/$01 is restored, the sole source image is at$7000, overlay code returns before replacement, and extra zero page is zero. Target resource measurements are retained with the proof.

These corrupt +0 priorities are not valid mission-produced Executive states. The bounded prefixes qualify their real local instruction aliases; arbitrary execution of reserved CPU/register words after the literal PC7/014 transfer remains outside a compiled native program. This does not certify complete mission application acceptance. Ordinary source selection, resource alarms and CCSHOLE transfers remain in their unchanged groups.

Fresh-checkout targets are `lm-core-fault-port-test` and `lm-core-fault-port-sanitize` from `tools/lm_core_fault_port_integration.mk`, using private builds, retained original snapshots and32 checked fixture descriptors.
