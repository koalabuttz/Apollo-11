# Canonical Comanche CLOCKJOB control

`cm_clockjob_enter`, `cm_clockjob_return`, and `cm_clockjob_select` translate
actual CLOCKJOB24,03332, its BANKCALL/QUICTRIG/SWRETURN bridge, and the
CKNVWRD1 branch. They operate on the sole 2,048-word image and explicit CM io
metadata. The caller holds producer exclusion; each call commits source
register, bank, and scratch effects before returning. Unsupported inputs leave
both owned objects unchanged.

The selected source job starts in bank24, EB6, with SUPER0..060. The lead-in
copies CDU X/Y/Z into CDUSPOT X/Y/Z and enters actual QUICTRIG23,03555 with
Q04675 and BUF2{03342,050000}. Separately qualified `cm_quictrig` returns
before TC A23,03605. The return helper executes TC A, SWRETURN, and TC BUF2 to
CKNVWRD1, preserving the ordinary sign-extended L word left by QUICTRIG.
CKNVWRD1 disables interrupts and branches on raw NVWORD1: positive values go
to REGODSP10,02707, positive zero to ENDOFJOB05217, negative values to
CLOCPLAY10,02715 for V99, and negative zero to CLOCPLAY for V97. The two flash
routes both load the actual V06N40 source literal01450 and retain their
different saved caller continuations. All other erasable inputs are ordinary
15-bit caller words, except full register A/L/Q patterns at the qualified
entries. Other external io fields remain unchanged.

The independent original-source corpus compares 24,576 complete 2,050-word
states, covering both zero signs, NVWORD sign classes, raw register patterns,
all four supported SUPER values and both IRQ states. Its SHA256 is:
`b2975daba408e3481a9a03529f4fa6583cc5ea1065bd44eef9f46739fbf7f206`.
Strict checks freeze 64 complete states and reject 31 invalid boundaries
without mutation. The whole corpus also agrees under ASAN/UBSAN. A continuous
512-case original CLOCKJOB run adds four checkpoints per case, executing the
native QUICTRIG arithmetic between both bridge calls: all 2,048 complete
states agree under strict C and ASAN/UBSAN, SHA256:
`0d006971cdcca9ab596842abc5ed096bc35cbc22fe9af2fa204ba535a49a9b7d`.

The C64 diagnostic checks all 64 frozen complete bridge states in PAL VICE;
its map and mailbox are recorded in `evidence/cm-clockjob-proof.json`.
The source slices and assembled rope are pinned independently in that
allowlist. Native code uses no AGC instruction decoder, emulator, floating
point, or heap. Recipes are in `tools/cm_clockjob.mk`.

This milestone stops before the selected display or END body. It does not
claim execution of DYNDISP/P40CNV85, the subsequent V97/V99 response body,
REGODSP/CLOCPLAY, or the shared Pinball consumer. Those are distinct source
continuations.
