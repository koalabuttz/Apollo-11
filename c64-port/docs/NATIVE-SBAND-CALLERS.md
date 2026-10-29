# Antenna control and canonical trigonometry

This packet compiles the original Luminary099 and Comanche055 antenna basic
entry/display-loop controls and canonical DP SINE/COSINE bodies into native C.
The antenna files remain **partial**: their interpreted calculation sequence
and the external orbital/display/Executive services need further composition.
No interpreter dispatch or instruction decoder is present.

`sband_control()` implements ten explicit source entry stages, with the
mission-specific priority/restart path. The source BANKCALLs enter R02BOTH,
GOMARKFR or CM DELAYJOB at their genuine addresses. B5OFF executes the unchanged
generic service routine through the ENDOFJOB entry. PRIOCHNG, BLANKET, ENDEXT,
ENDOFJOB and the other called bodies are explicit external continuations;
entering one is not evidence that its body has completed. The INTPRET front
enables interrupts and stops before NEWOPS fetches its first interpreted pair.

The caller supplies one canonical 2,048-word erasable image and owns SUPER and
IRQ externally. The original antenna bank is physical 42, selected by FB064000
and SUPER bit0100 under all four 0100/0120/0140/0160 latch forms. Entry PC,
coherent EB/BB, ordinary RAM and raw16 A/L/Q are checked before mutation.
Unsupported contexts preserve the image and latches. Caller exclusion of
interrupt/counter/Executive producers lasts through each native call; the
packet does not establish live application scheduling.

`sband_trig()` accepts the explicit original SINE or COSINE entry in fixed bank
0 with DP MODE0, ordinary RAM/Q and raw16 A/L. It gathers transient numerical
objects from the canonical image, calls the unchanged native polynomial and
product arithmetic, and commits every original scratch/register effect.
It returns at pending DANZIG, with actual Q3. LM TEM1/POLISH are03573/03560;
CM03572/03557. Source coefficients and degree are independently compared
against both original assembled ropes. External SUPER/IRQ and unowned RAM are
preserved. This bridge does not claim arbitrary vector-MODE or caller control.

The fresh host-only source oracle uses the original fixed-data layouts and
byte-original antenna files. LM INTERPRETER is byte-original; CM uses the
previously audited indentation-only repairs. Native code is MIT; GPL engine
objects appear only in the independent host oracle, with input-only constructors
and no linked native numerical or control implementation.

Control strict and sanitizer runs each compare77,824 complete source states.
Trig strict and sanitizer runs each compare262,144 complete source states.
Every state compares all2,048 words and both external latches individually;
lossless source archives retain exact records. The trig factory includes each
ordinary MPAC high/minor and each raw16 A/L marginal per entry, with sampled
other contexts. These are not exhaustive Cartesian claims. PC traces record
executed source controls separately from literal data.

Stock-ROM PAL C64 diagnostics compare304 control and128 trig complete frozen
states with the same exact word decoder. Their PRGs are14,476 and14,671 bytes,
with BSS ending608b/614e. Each standalone diagnostic uses two test images;
these sizes do not describe the integrated application resident budget.
The support proof and SHA ledger retain actual PRGs, maps, mailboxes and logs.

Reproduce using `tools/sband_control.mk` and `tools/sband_trig.mk`: targets
`source-audit`, `test`, `sanitize`, `freeze`, `check-host`, `vice-test`.
Target tools accept `MOS_CC`, `VICE`, `ROM_DIR` and `VICE_DATA_DIR` overrides.
