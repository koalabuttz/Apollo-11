# Comanche basic and interpreter context change

`cm_context_change` translates actual CHANG1 (`05164`) and CHANG2 (`05170`)
through selected basic DTCB transfer or interpreted INTRSM to NEWOPS (`06037`).
It returns the actual selected native continuation tuple without executing its
application body. It uses the sole 2,048-word image, explicit I/O and a
temporary seven-core view which commits before return.

Basic suspension saves the caller Q and actual physical BBCON plus SUPERBNK
in the selected outgoing core. Its raw incoming L is exchanged into Q exactly;
current BANKSET debris retains its original physical bits. Interpreter
suspension stores complement(completed LOC). Selected interpreted contexts
resume at complement(saved LOC)+1 and restore the source BANKSET, INTBIT15 and
EDOP effects. PUSH/OVFIND encode and decode both signed zeros and nonzero
overflow states as the source requires. Each path reports the genuine selected
basic or interpreter kind and address. Producer exclusion is caller-owned.

The source domain requires an active current core, a positive selected core
displacement and a valid basic/interpreter saved location/BBCON. Basic outgoing
Q and interpreted completed LOC are ordinary compiled addresses. Unsupported
inputs preserve image, I/O and output transfer. Raw A/L and interpreted raw Q
are retained or overwritten according to actual source instructions. This
adds no instruction decoder, saved-word execution or callback inside the
exchange frame.

The host corpus covers all six selected slots, both outgoing and incoming
context types and 512 register/bank/PUSH/OVFIND sets: 12,288 complete source
states. It matches every physical word, raw A/L/Q and SUPERBNK/IRQ with
lossless SHA256
`391ef82792a479b4762f0487c81f3a262240b88ceb0a50f0c73ece0036ad5319`.
The API's returned tuple also matches the source CPU image. Full strict and
ASAN/UBSAN checks pass. The original-layout rope is independently assembled
and authenticated as `6e3150ae...`; original Executive/interpreter instruction
operands remain untouched. The source oracle identifies the final PC together
with BBANK/SUPERBNK so a coincident Executive internal address cannot be mistaken
for a selected application boundary.

The actual C64 control payload `211` and proof-only input/checker `246/247`
replay 96 complete contexts through the checked disk loader, spanning both
context kinds and every selected core. Incoming C64 I alternates, while
producer exclusion covers returned frames and slot loads. Two full image/I/O
samples and the final whole image are compared directly, alongside every
complete-state fingerprint and returned source tuple. The retained target
report supplies the exact mailbox and software stack extent. All payloads fit
the shared `$8000..$bfff` slot; no second persistent Executive image is added.

Run `make -f Makefile -f tools/cm_context_change.mk test-cm-context-change`
and `sanitize-cm-context-change`. `prepare-cm-original-rope` supplies the explicit
fresh-checkout assembly prerequisite. The build and VICE scripts reproduce
actual payloads and compare their stock PAL C64 execution.
