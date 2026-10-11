# Comanche public superbank dispatch

`cm_super_dispatch` translates original SUPDXCHZ05227/+1(05230) and the actual
P20 termination caller GOTOV5604622. It receives the canonical2048-word image,
explicit external latches and output transfer. The actual selected compiled
callee remains a dispatcher continuation; no original word is decoded or
executed and no native callback occurs inside this group.

SUPDXCHZ swaps normal A/L source2CADR words; its +1 entry already has BBCON
in A and location in L. Both perform the source channel7 write, BBANK store
and `TC L`, producing Q2, the actual source PC/EB/FB/BB/SUPER pair and unchanged
IRQ. Legal source BBCON bits are076167; locations0100..07777 designate ordinary
compiled storage. CPU/editing/counter registers as code storage and raw
overflow descriptors are outside that compiled-caller contract. Malformed
inputs preserve image, latches and transfer.

The real GOTOV56 source caller performs EXTEND/DCA of original VB56CADR
{02653,066107}, then TCF SUPDXCHZ and transfers to actual TRACKTRM. The tuple is
authenticated against the original6e3150 rope and original P20 source. This
caller qualification does not certify upstream P20 or the TRACKTRM body.
Other actual public callers are NVSUBEND4216, BLANKSUB40,3432 and the Executive
idle paths. Their source-produced normal pairs lie in the proven public
entry contract; whole containing-caller bodies require their own composition
proofs, not inference from a matching tuple.

All32,768 original states match native complete2048 words and13 external
latches, SHA
`215e251abd3256d33cd40b51c680dbea40a0481d3db1ff762ba323bc1d913ab9`.
Direct-entry cohorts include every4032 ordinary location in both entry forms,
all2048 legal BBCONs, incoming hardware banks, virtual IRQ and SUPER; 4096
states execute the actual GOTOV56 caller. Both source ISR contexts are
asserted unchanged by the original oracle; the helper does not own that
caller metadata. Full ASAN/UBSAN repeats the same source stream.

The stock C64 diagnostic loads native module244, input254 and checker255
on its separate test disk. IDs are diagnostic-local and allocate no final
mission registry entries. Its native transfer body is924 loaded bytes.
Source-derived64 target cases preserve incoming I/ROM around every returned
disk overlay call, retain two directly compared2061-field source samples and
compare the final2048-word captured image directly. Published target verdict,
software stack measurement and exact payload identities are in the report.

Reproduce with `tools/cm_super_dispatch_differential.py`,
`tools/cm_super_dispatch_sanitize.py`, `tools/cm_freeze_super_dispatch.py`,
`tools/cm_super_dispatch_build.py` and `tools/cm_super_dispatch_vice_test.py`.
The VirtualAGC source is host-oracle only; native code is MIT and links no
AGC interpreter.
