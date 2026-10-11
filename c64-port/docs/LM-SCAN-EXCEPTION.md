# LM Executive scan exception handoffs

`lm_scan_exception` executes the PRIOCHNG, JOBSLEEP or ENDOFJOB preparations and EJSCAN through a genuine CCSHOLE transfer. A literal EJSCAN entry also qualifies source post-scan exception boundaries. Ordinary selection, interpreter restore and idle remain in their unchanged groups.

The source scan retains the real diminished candidate, BUF+1 comparison and BUF/BUF+2 debris from every earlier candidate. A +0 priority in slots 1, 2, 4, 5, 6 or 7 executes the actual TC CCSHOLE with its exact Q. Slot 3 is different: its +0 branch executes the `-CCSPR` data word as AD07610, then falls through the next candidate check. That temporary A value is overwritten by the following CCS; it changes no retained BUF/Q or other canonical word. The native body preserves this source behavior rather than inventing an alarm at the third slot. Literal positive/+0 BUF+1 post-scan cases execute the genuine calls at 03155/03156; those manufactured boundary states do not establish a valid flight producer.

The entry-specific state is retained: PRIOCHNG stores the actual caller/packed priority and switches BBANK, JOBSLEEP complements priority and saves EB/SUPER, and ENDOFJOB releases core zero and its lawful owned VAC before scanning. All routes stop at CCSHOLE05705 with source interrupt enable zero, exact A/L/Q, complete canonical scratch and unchanged SUPER. EXEC_BAILOUT_PENDING marks that source handoff; it does not simulate an alarm or recovery.

The unchanged original assembly proof compares 16,384 full 2048-word images plus SUPER/IRQ, spanning all three public entries, direct EJSCAN, every genuine zero-candidate call, third-slot data fallthrough and both post-scan calls. Native strict/full ASAN/UBSAN match every source byte. All 12,288 corresponding public-entry cases preserve the prior frozen lifecycle/END guards unchanged. Twenty-one mutation guards and two null interfaces preserve complete image/I/O when rejecting another domain.

Stock PAL C64 ROM/D81 execution checks 64 frozen original-state fingerprints through 32 returned input/checker modules. Mailbox `a50000400002`, software stack 61 bytes, hardware stack 30 bytes, additional zero page zero; native I/$01 is restored and source memory stays at $7000.

The body requires coherent source banks, ordinary non-overflowing public entry arguments and lawful owned VAC pointers. Direct positive/+0 BUF+1 qualification uses only non-active remaining candidates. Unrelated ordinary dispatch, raw TS/ADS overflow and malformed code-storage execution are outside this packet. CCSHOLE recovery belongs to ALARM_AND_ABORT and remains external.

Fresh-checkout targets are `lm-scan-exception-port-test` and `lm-scan-exception-port-sanitize` in `tools/lm_scan_exception_port_integration.mk`, with private host builds, streamed retained originals and checked fixture descriptors.
