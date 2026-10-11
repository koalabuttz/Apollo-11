# Original input component preparation and octal DP stores

The additive `dsky_input_store`/`cm_input_store` service implements actual
PUTCOM41,3075, PUTNORM41,3137, PUTDPCOM41,3130 and PUTCOM241,3155 in both
missions, composing the separately verified original noun checks and scale
selectors. It works directly on the canonical2048-word image. The caller
retains producer exclusion. No AGC instruction engine is linked into native
host or C64 target code.

PUTCOM accepts literal component A0..2, source MIXBR1/2, and ordinary source
caller Q0100..7777. Other entries use saved DECRET0115 as the selected caller.
PUTCOM2 does not constrain unused MIXBR/DECOUNT. PUTNORM's component is unused;
PUTDPCOM requires DECOUNT0..2 and a logical erasable destination0100..1774
whose physical selected major falls1100..3775. Entries require physical bank41,
FB062000, SUPER100/120/140/160, any retained aligned EBANK, coherent BBANK,
ZERO+0, IRQ0/1, ordinary image words and raw A/L registers. Selected double
precision destination is ordinary writable RAM1100..3775 with a bank-local
pair; register/editor/scratch aliases are outside this component contract.
For mixed DP preparation the selected ECADR low offset must accommodate the
component subtraction. The packed source scale index is0..13 when the actual
octal branch reaches DPTEST; decimal and negative-zero DECBRNCH do not need
that table-index condition. Decimal-only early alarm paths likewise retain out-of-table routine indices as unused source data. Unsupported input preserves image, IO, event,
restart and route atomically.

PUTCOM saves DECOUNT/DECRET, swaps the selected actual XREG/XREGLP, loads
MPAC/+1, clears MPAC+6 and derives the actual normal/mixed target address and
EBANK. It preserves the original CCS DECBRNCH four-way behavior. Positive
DECBRNCH reaches literal PUTDECSF03174 with the exact source CPU and stores,
reported as DECIMAL. +0 executes real DCTSTCYC: decimal-only noun reaches
actual ALMCYCLE (LM04145/CM04161), reported RECYCLE; otherwise the source
continues into SFRUT and DPTEST. Negative nonzero uses the scale selector;
-0 skips it and enters DPTEST with zero, retaining otherwise unused scratch.

The actual DP routines4/5/7/10 zero the major and adjust NOUNADD/DECOUNT for
an octal minor load. **CM routine13 is additionally DPFRACOT, while LM13 is
360-CDU with no DP**. The native composition preserves that original mission
difference. Normal non-DP channel-noun selection reaches literal CHANLOAD03161,
reported CHANNEL, retaining its original Q03146. Other completed paths execute
actual XCH MPAC and stored TC DECRET, returning with Q0116 and exact A/L,
scratch, address and data-word effects. PUTCOM2 supports raw overflow A and
both zeros through its actual register/memory exchange. Direct PUTDPCOM
performs original INCR/ADS corrections and the indexed zero-major store.

This service stops before decimal scaling or actual channel output. These
are genuine source continuations and remain separately tracked follow-on work.
It does not claim PUTXYZ/PUTXY/final single-component caller stores, complete
LOADLV/keyboard request lifecycle, arbitrary-memory aliases, or whole Pinball
completion.

Independent original assembly proof covers8192 full LM+CM states: every2048
image word and SUPER, IRQ, channels11/12/13, event mask, restart and route.
All source/native/full ASan/UBSan streams match byte for byte, SHA256:
`06575fd723c33b85490f2e7634a532ee7475a1f6e266144b2a63cdcc95cc1c99`.
Coverage includes both missions' true DP tables, all four DECBRNCH classes,
normal/mixed components, decimal-only alarms, channel handoffs, positive and
negative zero and extrema of entered words, raw register edges, retained
banks/latches and opaque unused fields. Combinations are sampled, not an
exhaustive Cartesian product. Fresh assembly equals each pinned original rope;
only indentation normalization needed by the reference assembler is recorded.
Frozen64 whole states plus16 atomic guards pass strict host and sanitizers.
Separate LM/CM native C64 diagnostics execute32 full frozen states each,
checking PAL VICE mailbox, no added zero-page storage, below-C000 memory and
hardware/software stack sentinels. They are component qualification builds,
not a production disk-overlay integration claim.

Reproduce with `tools/dsky_input_store.mk` host-only all/test/sanitize aliases,
explicit c64 and fresh differential/freeze targets. The immutable packet keeps
source/dependency hashes, fixture/source listings and audit, full compressed
source/native/sanitizer streams, host logs, target PRG/maps and raw mailbox and
stack evidence.
