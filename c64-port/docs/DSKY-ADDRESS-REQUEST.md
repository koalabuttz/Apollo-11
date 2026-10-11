# Native machine-address and component request control

`dsky_address_request` and `cm_address_request` execute original bank41
REQADD2064, operator request return2074, USEADD2105, REQDATX/Y/Z2303/2305/
2307 and common REQCOM2310. REQADD stores source pass0 ownership and tests
actual ENTRET: internal calls compose the complete INTMCTBS path; external
calls perform the real request, BANKCALL5BLANK, SWRETURN and FLASHON sequence.
REQDAT saves the complemented actual caller in REQRET and returns through
the source ENTRET producer instruction to actual ENDOFJOB, NVSUBEND or
PASTEVB. No callback, fake input or scheduler poll substitutes for these
source stages. Channel11 writes are explicit events, even when the value
already contains the flash bit.

The operator return performs all four original DECBRNCH CCS branches,
including both signed zeros and negative words. Positive decimal status
selects actual ALMCYCLE(LM04145/CM04161); octal paths execute source
CADRSTOR/FLASHON conditions and exchange ZREG into NOUNCADR, its EB and
NOUNADD. Actual immutable LODNNTAB is then called using original DXCH Z
transfer/return, and control reaches VERBFAN41,2133. USEADD is also exposed
as its actual entry. Neither decimal recycle nor verb dispatch is falsely
reported as completed by this request-control body.

Inputs require canonical ordinary image words, raw16 CPU, physical bank41,
any retained EB, SUPER100/120/140/160 and IRQ0/1. Numeric request callers
require ordinary Q0100..7777; common REQCOM requires ordinary A0..14.
NOUNREG0..99/eitherzero is required only when actual USEADD/table loading
is selected. When ENTRET is used, the accepted literal return instructions
are precisely those produced by both original programs: ENDINST, NVSBENDL
and MONBACK, as audited from every TS ENTRET producer. Unused ENTRET,
NOUNREG, Q and unrelated relay words impose no guard. Selected isolated
positive-zero relay executes real CCSHOLE prelude before any flash/return;
paired zeros execute corrected blanking. Unsupported inputs preserve image,
I/O and route. Caller holds restoring producer exclusion across composition.

8192 complete2053-field unchanged original/native/fullASan+UBSan checkpoints
match all2048 words plus SUPER, IRQ, channel11, its write event and route.
Full SHA256:
`d84afeb9770db1c369a7f30d021005ffb2e3d3484cef5022b9e13ccd01299140`.
Both original missions are independently rebuilt with eight-file original
source audits (including actual interbank communication) and byte-exact
pinned ropes. The oracle links no native request, blanking or reader body.
Its VERBFAN stop checks the actual physical bank, avoiding a same-address
noun-reader instruction. Corpus includes every entry/frontier, both zeros,
all EB/SUPER/IRQ, NOUT and decimal boundaries, raw CPU, real noun tables,
selected positive-zero recovery and deliberately invalid unused fields.

Sixty-four frozen complete checkpoints cover every source entry/route pair,
all retained EB, selecting SUPER and IRQ states, raw L classes and NOUT
edges in each mission. Strict host, sanitizers and native stock-ROM C64 VICE
pass; mailbox a500004002; PRG25897 loaded25895 bytes; hardware/software
stack extents25/95. tools/dsky_address_request.mk provides portable host
aliases and explicit target. The diagnostic includes two canonical images,
immutable noun data and frozen changes; production retains one image.
Actual downstream ALMCYCLE, selected verb, PASTEVB/NVSUBEND, source job
completion and exceptional recovery remain separately compiled services.
