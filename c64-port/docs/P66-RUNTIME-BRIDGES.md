# P66 live source control bridges

`p66_runtime_throttle_enter` starts at the compiled source lead-in
NEWOPS06070/LOC03510 or literal EXIT06743. It translates the fixed EXIT,
DXCH MPAC and BANKCALL through the actual THROTTLE+3 entry02221.
`p66_runtime_throttle_return` translates THDUMP/SWRETURN/INTPRET through
NEWOPS06070/LOC03515. `p66_runtime_display_exit` follows the actual final
basic PC03531 to REGODSPR02470 or suppressed-display ENDOFJOB05155.

These boundaries occur after the preceding legal DANZIG job test. All three
preserve any ordinary 15-bit NEWJOB word without polling it. This includes a
request arriving after that preceding test. A later real interpreted boundary
or Executive admission service decides how to consume the request. Source
REGODSPR admission itself is a separate service and may change NEWJOB.
Canonical raw A/L/Q, bank aliases, source scratch and explicit SUPER/IRQ
are retained exactly; the header states the supported entry domain. Invalid
boundaries leave both input objects unchanged. The caller owns exclusion.

`tools/runtime_bridge_differential.py` independently compares 16,384 complete
2,050-word original Luminary099 states across both throttle entrances,
return and display branches. Pending words include +0, +1, valid core offsets,
maximum positive, negative and -0 forms. Snapshot SHA256:
`e9855784cd6054b430f8745d452ecb6a5a4eef304ea0c7e327ed7288cc094f01`.
Strict tests cover 64 frozen complete images and 56 invalid boundaries; all
source states also agree under ASAN/UBSAN. C64 PAL VICE passes all64 images:
mailbox `a500004000`, PRG 6,770 bytes, no extra zero page, measured hardware
stack20 bytes (plus14 ROM bytes) and software stack24 bytes.

Standalone recipes are in `tools/runtime_bridge.mk`. Earlier bounded bridge
modules and their published evidence remain unchanged. Native code executes
fixed translated control paths and contains no AGC instruction decoder.
