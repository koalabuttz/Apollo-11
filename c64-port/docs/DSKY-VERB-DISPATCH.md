# Native verb dispatch

`dsky_verb_dispatch` and `cm_verb_dispatch` execute actual VERBFAN41,2133.
Verbs0..39 and negative zero select each mission's unchanged40 passive CADR
words, then execute source BANKJUMP to the actual selected verb entry.
Verbs40..99 store the source verb-minus40 index in MPAC and stop at actual
RELDSP: Luminary04457 or Comanche04473, with actual returnQ02143.

The native table contains passive CADR data extracted from separately rebuilt
unchanged original assemblies. Its audit records both original source hashes,
all40 source labels and words. Native code uses that data in explicit source
bank/address operations; it does not fetch or decode source instructions or
invoke an emulator. Each mission is qualified independently. Original CM
REQEXLQC and LM REQEXLOC spellings are retained in the table audit.

Inputs are actual bank41, all8 EB banks, SUPER100/120/140/160, IRQ0/1,
ordinary erasable words and VERBREG0..99 or negative zero. The canonical image
is sole authority; caller owns restoring producer exclusion. Unsupported
image, metadata and route remain unchanged. The service preserves raw CPU
register semantics, actual A/L/Q/BB/FB debris and source-selected target.

8192 complete2051-field original/native/ASan/UBSan states cover all40 table
entries, all extended verb indices, signed zero, every EB/SUPER, IRQ states
and raw L correction edges. SHA256:
`b9e2a15a34556d4a99b7001209fe57ec7c5af5052c480a4ca2a2954e5900e5ef`.
The oracle links only unchanged original source execution and source-agnostic
inputs, never the native service. Rebuilt full ropes must match pinned
mission fixtures; original sources and indentation patches are retained.

Sixty-four frozen complete states pass strict host, sanitizers and actual
native C64 diagnostics. The explicit C64 target checks stock-ROM VICE mailbox,
RAM bounds, zero-page allocation and both stack sentinels. Portable host
all/test/sanitize aliases are in tools/dsky_verb_dispatch.mk. Selected verb
bodies and internal RELDSP return remain explicit next source continuations;
this dispatcher is not a whole Pinball completion claim.
