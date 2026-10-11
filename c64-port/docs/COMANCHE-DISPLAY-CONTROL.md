# Canonical Comanche display entries

`cm_display_control_enter` executes actual REGODSP10,02707,
CLOCPLAY10,02715, and GOFLASH10,02720 through GOFLASH2 and SAVELOCS,
ending at MAKEPLAY10,02770. It saves the verb/noun argument and actual
caller CADR, combines the selected source flags with the current EB, and
retains the original RUPTREG3 addition and full register debris. The source
TC L leaves Q2; caller L, Q and accumulator inputs are handled through the
original stores and exchanges. IRQ is enabled on return.

The caller owns producer exclusion, a canonical 2,048-word image, and distinct
CM io metadata. The source fixed bank is10; all eight EB and SUPER values
are retained. A must be an ordinary sign-extended word. BUF2 contains a saved
basic location0..07777 and aligned fixed bank0..076000; all other non-register
erasable inputs are ordinary 15-bit words. Unsupported inputs leave both
objects unchanged. Native code uses no AGC decoder, emulator, floating point,
or heap.

The independent unchanged original source compares 24,576 complete
2,050-word states, including both zero signs, sign classes, full caller
register patterns, all banks in the saved return, and both signs of the
final addition overflow. Strict checks freeze 64 complete states and reject
42 invalid boundaries without mutation. The entire corpus agrees under
ASAN/UBSAN, SHA256:
`c7571e875fe36442ae7e0bb5b1229089ce3f32b77154766b668a5859d7451673`. A continuous CLOCKJOB test adds the real QUICTRIG and both
BANKCALL bridges, then these entries: 512 cases, 2,544 original full-state
checkpoints, including zero-NV termination. Its SHA256 is:
`275c1bccb176f151873feb44b92a976be40170463589309e6df2b8b16ca37a71`.
The continuous corpus also agrees under ASAN/UBSAN.

The C64 diagnostic executes all 64 frozen complete states in PAL VICE.
Source and target artifacts, exact checksums, and target resource measurements
are recorded in `evidence/cm-display-control-proof.json`. The source identity
check covers every executed instruction in the three entry routines,
SAVELOCS, and MAKECADR. Recipes are in `tools/cm_display_control.mk`.

This milestone stops before MAKEPLAY. Subsequent priority exchange,
sleep/wake control, Pinball consumer, and astronaut V97/V99 responses remain
separate source continuations.
