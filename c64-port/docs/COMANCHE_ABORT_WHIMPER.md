# Comanche WHIMPER source recovery transfer

`cm_abort_whimper` executes the actual INHINT/WHIMPER (`07766` or `07767`),
RESUME and source-selected `TC 07773`, followed by the actual POSTJUMP to
ENEMA (`05,2651`). It writes BRUPT=`07773` as source data, commits the genuine
raw CPU/bank image, and clears the external ISR-active metadata at RESUME.
The known selected source continuation is compiled directly; no BRUPT word
is decoded or used to execute arbitrary original instructions.

The `07766` entry inhibits interrupts. A direct `07767` WHIMPER caller retains
its incoming source IRQ state, exactly as the original. The final transfer
preserves SUPERBNK, selects physical FBANK=`012000`, retains source EBANK,
raw A=`07773`, and reaches actual ENEMA with Q=`00651`. Producer exclusion
is caller-owned. ENEMA cleanup and selected application restart remain
subsequent explicit native groups.

The original-layout source rope is authenticated as
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
All 8,192 complete original/native physical-image states match every word,
raw register and SUPERBNK/IRQ, with lossless stream SHA256
`176abac930129c46ef6b99863482a79fafe58ba2fabe0906bd1aa1464779fe27`.
Cases vary the full ordinary image, all physical banks, raw A/L, both entries,
ISR-active states, saved ZRUPT and incoming IRQ. Both the source oracle and
native snapshot assert that ISR-active clears. Full strict and ASAN/UBSAN pass.

Stock PAL C64 loads actual native control 217 and proof-only input/checker
254/255 through the checked D64 loader. All 64 representative source contexts
pass with mailbox `a500004000`, two direct complete image/I/O samples and the
final full image. The checker additionally asserts cleared ISR metadata.
Measured software stack extent is 712 bytes; the control payload is 893 bytes.

Run `make -f Makefile -f tools/cm_abort_whimper.mk test-cm-abort-whimper`
and `sanitize-cm-abort-whimper`. The bridge qualifies this actual WHIMPER
transfer through ENEMA entry; the complete abort/restart bodies and complete
mission caller closure remain separate source qualifications.
