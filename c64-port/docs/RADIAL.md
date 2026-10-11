# P66 radial velocity and predicted altitude

`p66_radial_position` translates P66's UNIT R through the final
STODL HCALC1,HDOTDISP before BDSU VDGVERT. It stores the half-unit radius
at VAC 14..19, computes radial velocity from the updated velocity at 24..29,
reloads elapsed time at 30/31, and computes predicted altitude using the retained
length at 36/37 and the erasable landing radius. `/LAND/` is an input owned by
the original GUIDINIT path; the test lunar-radius seed is not a fixed program
constant. The stores preserve original destination exchange and A/L debris.

The operation composes verified normalization, dot product, shifts, multiplication
and scalar sums. UNIT overflow continues through the original interpreter route
and is reported after the final store. Unsupported ownership/MODE rejects before
mutation; a negative SQRT reports its source alarm boundary. The caller owns
scratch/VAC and excludes producers. Eight new bytes hold HDOTDISP and HCALC1;
all modeled state is 134 raw words (268 bytes).

The unchanged original-assembly oracle with the pinned hardware-DV qualification
checks 2,467 inputs and 4,952 complete checkpoints. These cover all 128 integrated
skip inputs, three four-step histories, raw corners and 2,048 random calls.
Host comparisons run the complete live arithmetic prefix through radial output.
Snapshot SHA256:
`7267aa699556e96da64ac25fb24149b08e15a7bd801232f686cecac87484acdd`.
Strict host and complete ASan/UBSan state comparisons pass.

PAL C64 verification executes this radial stage from exact pre-UNIT words
recorded by the original oracle. Six images compare 203 complete output states,
including the pre-states produced by every integrated skip input and retained
signed-zero regressions. They contain 35/35/35/35/44/19 states respectively;
none calculates its expected outputs from the native implementation. Maximum
RAM/state ends at $af6e, with zero extra compiler zero page. Measured hardware
and software stack extents are 31/421 bytes. Finite target samples took
54,844..158,996 PAL cycles, approximately 55.7..161.4 ms.

This stage target proof does not establish a resident complete P66 executable.
The full-prefix target link exceeded RAM: text alone was 41,767 bytes and the
fixture/state layout ended at $e574. The application overlay architecture must
resolve cumulative code and state pressure before full flight-program acceptance.
The earlier velocity target independently verifies its live prefix.

`tools/radial.mk` supplies the host, sanitizer and six C64 builds.
`tools/radial_differential.py deps/fixture` reproduces assembly comparisons;
`tools/radial_vice_test.py --half 0..5` selects each target image. Evidence
includes the lossless full snapshots, frozen states, maps and target mailboxes.
