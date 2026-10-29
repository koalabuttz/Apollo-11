# Original load caller table setup and final stores

This additive native service translates actual PUTXYZ/PUTXY and the single
load resume paths, their literal PUTCOM caller stores, and the noun7 flag
controls for both original programs. It composes unchanged ALLDC/OC and
LODNNTAB bodies with immutable native original noun tables. It executes
compiled source control and word exchanges, not instruction decoding.

All entries are source bank41 in both missions:

| Entry/stage | PC |
|---|---:|
| PUTXYZ | 02632 |
| PUTXY | 02714 |
| A/B/C load resume | 02733 / 02750 / 02765 |
| XYZ X/Y/Z PUTCOM return | 02641 / 02645 / 02651 |
| XY X/Y PUTCOM return | 02723 / 02727 |
| A/B/C single PUTCOM return | 02740 / 02755 / 02772 |
| Noun7 flag setup | 02660 |
| BITSOFF / BITSOFF1 | 02674 / 02701 |

`dsky_load_store` accepts the canonical image and `dsky_lamp_io`;
`cm_load_store` accepts the canonical image, `cm_native_io`, retained channel
write events and restart light. Callers retain restoring producer exclusion.
Entries require FB062000, SUPER100/120/140/160, aligned retained EBANK0..7
with coherent BBANK, ZERO+0, IRQ0/1 and restart0/1. Incoming Q is opaque.
Unused source fields impose no added guard. Unsupported inputs preserve the
whole image, IO, events, restart and route atomically.

Table-setup entries require noun0..99, including either zero; this caller
contract also applies if ALLDC/OC detects a mixture before table reading.
Final indexed component stores require a source logical address plus component
index<=01777 selecting ordinary physical RAM1100..3777, excluding register,
editor and shared scratch aliases. Their incoming A must have no overflow;
the real PUTCOM producer returns a sign-extended ordinary MPAC word via XCH,
so this is its genuine caller return contract. Arbitrary raw-overflow standalone
caller frames and unsupported destination aliases are outside this contract.
Noun7 setup accepts a selected flag ECADR masked to ordinary physical0100..3777,
including source flag RAM below1100. Direct BITSOFF accepts an ordinary
selected NOUNADD address0100..3777. BITSOFF1 does not inspect an address and
retains raw A, including overflow encodings.

PUTXYZ/PUTXY execute actual ALLDC/OC with -6/-5 and preserve genuine ALMCYCLE
failures. All five setup entries execute the real DCA/DXCH Z caller exchange,
including temporary EB1/FB064000, actual LM42,2103/CM42,2062 LODNNTAB,
original normal/mixed table scratch and source restored banks/A/L/Q. They end
at actual PUTCOM41,3075 with literal component0/1/2 and genuine caller Q.
PUTCOM arithmetic/store dispatch is separately implemented by the unchanged
input services; this wrapper does not fabricate its result.

Each return stage writes the actual indexed destination with source TS word
correction, then selects the next PUTCOM component or actual LOADLV41,2775.
The XYZ final store executes the real CS7/AD NOUNREG/BZF comparison, preserving
both zero branches and raw A. Noun7 flag setup executes SETNCADR+1 without
changing NOUNCADR, derives actual EBANK/NOUNADD, inhibits interrupts and follows
the real ZREG selector. Both source ZREG zeros select BITSOFF. Set-bits uses
complement/mask/ADS with original one's-complement addition; clear-bits uses
complement/mask/TS. Mask results are sign-extended before the source arithmetic
and store, including negative flag words. Both routes execute real RELINT and
TC LOADLV with original Q02703. The COMPLETE route ends at actual LOADLV;
its complete release/recall/END lifecycle has a separate verified service.
No channels, SUPER, write events or restart light are written by these stages.

The independent fresh-assembly oracle checks both ropes against pinned
fixtures and links no tested native body. All8192 complete original LM+CM
states match strict native C and full ASan/UBSan: all2048 image words plus
SUPER, IRQ, channels11/12/13, events, restart and route.
Snapshot SHA256:
`47595ff196d5c54878d3444b5206ad7afc586262ea0777d25233a1d6f4013546`.
The corpus covers all16 entries, every noun-table index/both noun zeros,
normal/mixed table selection, decimal/octal mixture failure, final caller
stores, all retained EBANKs/selecting SUPER forms, flag address banks/upper
sign bits, independent flag-mask words and both ZREG zeros. It samples
combinations rather than asserting their Cartesian product. Frozen64 full
source states and26 atomic guards pass strict host and sanitizers, including
raw TS overflow, unsupported destination aliases and selected flag addresses.

Actual C64 PAL VICE diagnostics pass32 complete source states per mission.
LM mailboxa500002000, PRG11338 bytes/loaded11336; CM mailboxa500002001,
PRG10838 bytes/loaded10836. Hardware/software stack extents are10/49 for both,
with no added zero-page storage and below-C000 diagnostic RAM budget. Mission
fixed-data compile guards retain only that original program's constant tables.
These linked component checks do not establish application disk-overlay
integration. Exact sources/dependencies, fixture audits/listings, full compressed
streams, logs, PRG/maps and raw mailbox/stack evidence are frozen.
`tools/dsky_load_store.mk` supplies host-only all/test/sanitize aliases and
explicit C64/differential/freeze commands. Genuine channel loading and
uninterrupted source load/input/application composition remain tracked work.
