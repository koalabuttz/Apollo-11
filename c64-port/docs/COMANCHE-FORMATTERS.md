# Comanche decimal formatter source qualification

`cm_two_digit` implements actual Comanche055 DSPDECVN40,3213 through its caller.
`cm_decimal_display` implements actual DSPDECWD40,3133 (rounded) and
DSPDECNR40,3167 (unrounded), including original sign-relay writes, rounding and
five decimal digits. Both use one canonical image and transient SUPER/IRQ only.
The fixed arithmetic/store kernels are shared with existing native formatters
after independent CM source execution verifies every physical word. Literal CM
source return stores are retained: TRACE1S's TC DSPIN3157 stores3160 in DSEXIT114,
and DSPSIGN's rounded/unrounded caller stores3136/3172 in TEM1144. No opcode
or native callback is fetched from those ordinary original address words.

Domains match original qualified formatter paths: bank40/SUPER100/retained
EB0..7, ordinary caller Q and nonzero relay words. Two-digit input A0..99 uses
odd DSPCOUNT1..21 and preserves original entry MPAC+2. Decimal input uses a
sign-agreed DP MPAC0/1, original TP MPAC+2 and DSPCOUNT14/9/4 selecting R1/R2/R3.
Unsupported inputs leave source/I/O unchanged. Formatting creates actual DSPTAB
and NOUT source output; queued display jobs/device draining are separate callers.

Three4096-case corpora execute actual original CM source independently, covering
two-digit conversion and both signed five-digit modes, all retained banks,
positive/negative values and signed zeros, maximum/rounding boundary inputs,
TP debris and NOUT overflow corners. Every one of12288 checkpoints compares
all2048 physical words plus SUPER/IRQ and matches the original. The oracle links
no native bodies. The private Pinball source is byte-identical to the original;
T4 indentation normalization and original SHA hashes are recorded in
`build/cm-formatter-source-audit.json`, with the complete rope required to equal
the separately assembled pinned CM rope. Matching snapshot SHA256:
`af601b37ca463041eb2bad3686d56aadc834e78a5049bde2117012bdf3dd467c`.
Full ASan/UBSan output equals the complete original stream byte for byte.

Frozen64 complete2050-field source images pass strict host, sanitizer and native
C64, including all eight signed/max/corner DP values in each of R1/R2/R3 for
both decimal modes. VICE PAL mailbox `a500004002`; PRG17833 bytes (loaded17831);
hardware/software stack13/35 bytes. PRG and section bounds below C000, plus zero
additional compiler zero-page storage, are checked. Evidence uses
`build/cm-formatter-*` and `build/cm_formatter-*`; full streams are retained
compressed. `tools/cm_formatter.mk` exposes portable host-only all/test/sanitize
and explicit c64-cm-formatter/differential-cm-formatter targets.

These services qualify the actual CM formatter bodies. Noun selection/scaling,
NVSUB application control, actual queued display consumption and remaining CM
keyboard functions remain separate required translation/composition work.
