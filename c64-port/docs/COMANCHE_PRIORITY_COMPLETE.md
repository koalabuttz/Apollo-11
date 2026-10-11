# Comanche priority change with basic and interpreter selection

`cm_priority_complete` translates actual PRIOCHNG (`05210`) through the
source scan and current or selected basic DTCB transfer or interpreted INTRSM
to NEWOPS (`06037`). It returns the genuine selected continuation and kind;
application bodies execute in a later native frame. The only persistent
flight state is the canonical 2,048-word image. Its temporary seven-core view
commits before return, and producer exclusion is caller-owned.

The source scan recomputes NEWJOB and restores the selected physical BBCON,
SUPERBNK, priority and registers. Selected negative interpreter LOC is decoded
as complement(saved LOC)+1. PUSH and OVFIND preserve source signed-zero and
overflow conventions, and INTRSM commits the actual LOC, BANKSET, INTBIT15 and
EDOP effects. Ordinary raw A/L/Q are corrected or retained only where the
source instructions do so; returning the current basic context leaves the
source BANKSET debris derived from the old L.

The public domain requires a positive upper priority in A, an ordinary compiled
caller Q, consistent physical banks and source-valid active/saved core states.
Priority 1, dormant and -0 nonwinning candidates are represented. CCSHOLE/+0
fault candidates and selected alarm/application body execution remain outside
this packet. Unsupported inputs preserve image, I/O and output transfer.
No instruction decoder or saved-word execution is present.

The independently assembled original-layout rope is authenticated as
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
All 14,336 complete source states match every physical word, raw A/L/Q and
SUPERBNK/IRQ, with lossless stream SHA256
`46d674a39e37a28a73763529014a18edba7fd0f2d66f88b4da3270df050eafb3`.
The corpus spans current context or all six selected slots, basic/interpreter
selection and 1,024 priority/register/bank/PUSH/OVFIND combinations. Returned
native tuples are independently compared with the actual source CPU image.
Full strict and ASAN/UBSAN validation pass.

The stock PAL C64 packet loads native control `212` and proof-only input/checker
`248/249` through the checked disk loader and replays 112 representative
contexts. Incoming C64 I alternates. Two complete image/I/O samples and the
final image are compared directly, with complete-state fingerprints and
actual tuples for every context. The retained report records its outcome,
mailbox, exact payload bytes and software stack extent.

Run `make -f Makefile -f tools/cm_priority_complete.mk test-cm-priority-complete`
and `sanitize-cm-priority-complete`. The original-rope preparation prerequisite
is explicit. This packet qualifies its stated source domains; it does not
certify the entire Executive file or every mission caller.
