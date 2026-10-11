# Comanche normal V06N85 display caller

`cm_nv85_step` translates the actual normal CKNVWRD1/REGODSP admission,
MAKEPLAY priority and wake services, GOPLAY/NVDSP, NVMONOPT/NVSUB,
verb6/noun85 setup, all three mixed-noun component displays, normal GOPLAY
completion, and actual source ENDOFJOB through idle ADVAN03231.
The source verb/noun word is octal01525. This packet closes this concrete
normal display path; it does not certify every Pinball or display-interface
entry or the still-separate flashing/busy continuations.

The sole persistent erasable image contains source registers, seven cores,
VACs and display scratch. External SUPER, IRQ and channel11 are explicit
`cm_native_io` fields. Native routes0..25 follow the compiled source; they
are not instruction words. Producer exclusion and overlay loading belong
to the caller. A linked route must return before its code overlay is
replaced. An unexpected scheduling/wake/END continuation is a native
fault boundary rather than an additional invented source poll.

The new leaf groups begin at source PCs03312,04171,02337,03602,03604,
02537,02566,02570 and03344. The source NVSUB bridge selects bank41/E1,
then the verb and noun formatter calls use bank40. The noun85 mixed
addresses are physical03664,03666,03670 (VGBODY); each DP3 formatter
freshly reads the selected pair. Original noun literals are
NNADTEM024207, NNTYPTEM024512 and RUTMXTEM016347. All three components
use the velocity3 factor{01031,021032}; DP3 applies the original product
and seven left shifts before the rounded decimal formatter. This preserves
source low-word signs, rounding/clamp behavior, CYR/CYL, register values,
return banks, source scratch and relay words. NOUT and eleven DSPTAB words
are updated by the actual formatter. Device relay draining is a separate
qualified service and is not called by this packet.

The bounded admission domain uses a normal unblocked display, no external
monitor, no pending NEWJOB, NVWORD1=01525, active basic core zero, and
six unused negative-zero core priorities. Tests initialize only relevant
Pinball fields from actual STARTSUB defaults: eleven dirty blank DSPTAB
words073777, NOUT11, no display lock or monitor, and CADRSTOR pair zero.
Other erasable words remain caller supplied. SUPER0/20/40/60 and IRQ0/1
are retained/restored according to source. The public leaf contract is
this normal domain; the derived busy early-return branch in `cm_nv85_enter`
is not a qualified busy-display lifecycle.

The host oracle executes the original pinned Comanche rope, not native
helpers. All512 continuous paths and26 boundaries (13,312 complete images,
2,051 fields each) match without normalization. ASAN/UBSAN matches the
same complete stream. Raw source SHA256 is
`0a203ba3c2affb31004cd0341777ff6cea5d513489afd31bb3585dd84ac21b6c`.
Sixty original before/after caller states also pass the strict frozen
checker and81 unchanged invalid-domain checks. Actual PAL C64 execution
passes all60 states with mailbox `a500003c00`; PRG31656 bytes, measured
hardware stack11 bytes plus14 ROM bytes, software stack102 bytes and
zero additional zero-page bytes. The target imports original inputs at
caller boundaries; the uninterrupted whole chain is independently proved
on the host, while callees have their own prior source/C64 qualifications.

The reference rope is the immutable existing powered-source fixture
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The runner checks normalized source identity for Executive and noun tables.
It allows only the fixture's pre-existing four DSPOCTWO→DSPOCTWD spelling
repairs (outside this decimal path), one CAF EBANKSAV→CA EBANKSAV mnemonic
alias repair, and omission of unused PRIOSAVE EQUALS R1SAVE. Both CAF and CA
assemble to the same basic CA instruction; no arithmetic body is changed.
Original and fixture identities are recorded in the evidence manifest.

Reproduce using `tools/cm_nv85_differential.py`,
`tools/freeze_cm_nv85.py`, `tools/cm_nv85.mk` host/sanitize/C64 targets and
`tools/cm_nv85_vice_test.py`. The compressed original stream is retained;
no duplicated raw stream is required.
