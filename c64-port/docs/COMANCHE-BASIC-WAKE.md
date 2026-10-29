# Canonical Comanche JOBWAKE

`cm_basic_wake` translates actual JOBWAKE05201 through the ordinary basic
caller return. It reuses the qualified seven-core context service through the
shared canonical gather/commit API, then retains all source CPU and scan
scratch effects. The first matching dormant CADR is awakened; a missing
match returns EXEC_NOT_FOUND with the original LOCCTR-1 representation.
It preserves full caller L exchange debris and leaves IRQ inhibited.

The caller holds producer exclusion and owns the canonical image and CM io.
A is an ordinary sign-extended sleeping CADR, and Q is a basic return within
0100..07777. Core0 is active positive; all candidate priorities are
positive/dormant/-0, with +0 excluded. NEWJOB is -0 or a valid displacement
0..72 by12. Matching dormant contexts have source saved bank bits<=0177.
A pending native Executive alarm is outside scope. Unsupported inputs leave
both objects unchanged. All other non-register erasable words are ordinary
15-bit inputs.

The independent unchanged Comanche source compares 8,192 complete
2,050-word states: all EB/FB/SUPER values, both IRQ states, full caller L
patterns, four caller boundaries, all six matching slots, missing matches,
duplicate CADRs and both zero signs. Strict checks freeze 64 complete states
and reject 19 invalid boundaries without mutation. The entire corpus also
agrees under ASAN/UBSAN, uncompressed SHA256:
`6416525e5b41aafb661b3f4d70d716cebd350d54cb5de419c3abd5fbcc8cf73c`.
Original, native and sanitizer streams are retained as deterministic XZ.

All 64 original frozen states pass on an actual C64 program in PAL VICE,
mailbox`a500004000`, PRG11,931 bytes. Measured hardware stack is18 bytes
plus14 ROM bytes; software stack736 bytes; no extra zero page is allocated.
Exact source and artifact checksums are in `evidence/cm-wake-proof.json`;
recipes are in `tools/cm_wake.mk`. No native AGC decoder, emulator, floating
point or heap is used. The helper stops at the caller entrance; it does not
execute the next consumer group or a job exchange.
