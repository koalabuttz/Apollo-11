# Returned-conic antenna caller

`sband_orbit()` compiles the original LM and CM SLOAD/BHIZ reference-frame
selection and its following load/store/call controls into native C. It accepts
the genuine conic-return NEWOPS entry: LM LOC03616 or CM03662 in bank042
(octal), with ordinary source image, raw16 A/L, valid VAC and NEWJOB+0.
It does not implement or pretend to run either conic routine.

The source X2 at FIXLOC+047 selects Earth for both +0 and -0. In the LM,
the Earth path loads RATT, sets PUSHLOC and reaches the original UNIT basic
entry03023 with vector MODE, source LOC03641 and FB0. In the CM, it loads and
complements RATT, sets PUSHLOC to VAC+2, selects the actual REFSMMAT erasable
bank, and reaches MXV07303 with ADDRWD01735. Neither helper body executes in
this stage. These frontiers can be composed with the frozen canonical UNIT
and matrix APIs under the same producer exclusion.

For the Moon path, LM stores the complete vector at physical02204..02211
(original RLM E4,1604), while CM stores it at VAC+2..7. CM's destination overlaps
the source RATT at VAC+0..5: all six vector words are first loaded into MPAC,
then the original sequential stores use those preserved values. Both missions
load TAT from VAC+014/+015 and enter the real LUNPOS interpreted entry, with
saved QPRET065627 LM or065673 CM. New basic PC remains the original NEWOPS;
LOC, FB, BANKSET, INTBIT15, scratch and register debris match the source.
LUNPOS and the following vector/angle stages remain external continuations.

The single canonical image is authoritative; IRQ and SUPER remain externally
owned and unchanged. Entry PC/LOC/banks, ZERO, pending EDOP, ordinary words,
VAC base and NEWJOB are checked before mutation. Unsupported context preserves
all image words and latches. The caller excludes interrupt/counter/Executive
producers throughout each call. No AGC instruction engine or interpreted
dispatcher is present in native code.

Fresh original-source and full ASAN/UBSAN runs each compare131,072 complete
source states: all2,048 words and SUPER/IRQ individually. A separate compiled
factory inspection confirms all32,768 X2 words and all65,536 raw A/L values
per mission, with259 Earth and65,277 Moon cases each. All five VAC bases,
eight EB banks, four bank-selecting SUPER forms and both IRQ values appear.
These are marginal coverage counts, not an exhaustive Cartesian guarantee.
The input-only GPL oracle contains no native numerical/control implementation;
the new native module is MIT and uses public-domain source control semantics.

The C64 diagnostic compares64 exact frozen states using lossless word data.
Its PRG is8,621 bytes, load ends29ac and BSS49ac (hex), with mailboxa5400000.
Two diagnostic images are test storage, not duplicated application RAM.
Actual PRG/map, mailbox, target logs, canonical source archive, factory counts
and source-line word maps are retained in the packet.

This maps13 LM and14 CM original antenna words. Both containing antenna files
remain partial until the later numerical paths and real returns are composed.
