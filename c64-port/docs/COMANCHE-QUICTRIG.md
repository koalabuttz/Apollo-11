# Comanche basic sine, cosine and QUICTRIG

The native `cm_single_trig` and `cm_quictrig` services reproduce the original Comanche basic arithmetic and its raw register/scratch effects. All164,096 complete2048-word-plus-IRQ source states match the strict native build and the entire ASAN/UBSAN stream:65,536 raw sixteen-bit accumulator patterns each for SPSIN and SPCOS, plus33,024 QUICTRIG inputs including every15-bit angle on each CDU axis. The constructor varies every actual input and rotates the nonzero unowned image context in256-case blocks. Every source image word is compared; MODE is retained for all ordinary word values.

A further512 original CLOCKJOB calls execute the actual source CA/TS CDU copies and BANKCALL before entering QUICTRIG. On each independently computed source entry, the native numerical service matches the complete2048-word-plus-IRQ output, including its separate sanitizer run. Four actual C64 images check32 complete original entry/exit states each (128 total): SPSIN, SPCOS, direct QUICTRIG, and QUICTRIG with an actual CLOCKJOB call context.

The original code uses a single-precision polynomial and source-specific quadrant/overflow paths. It is distinct from the interpreter's double-precision SINE/COSINE approximation. No host floating-point calculation or instruction decoder participates in the native implementation. Raw signed zeros and accumulator overflow patterns retain source behavior; an untouched raw overflowed L becomes sign-corrected by the source hardware instruction cycle.

`cm_single_trig` accepts basic PC05075 for SPSIN or05074 for SPCOS and stops before TC Q at05130 (polynomial) or05133 (quadrant extreme). It preserves source Q and IRQ. The caller owns the basic return continuation. `cm_quictrig` accepts source23,3555, bank23 with a valid EB/BB pair and actual source Q. It stops before TC A at23,3605, retaining the source A/L/Q and all bank controls. The source INHINT/RELINT sequence leaves IRQ enabled. There is no Executive handoff inside either basic routine; caller ownership and scratch-producer exclusion span the service.

| Source word | Physical address / behavior |
|---|---|
| CDUSPOT Y/Z/X | 0766/0767,0770/0771,0772/0773; retained |
| SINCDU Y/Z/X | 0736/0737,0740/0741,0742/0743 |
| COSCDU Y/Z/X | 0744/0745,0746/0747,0750/0751 |
| ITEMP1 | 0061, actual incoming Q retained |
| ITEMP2 | 0062, final positive zero |
| TEMK / SQ | 01075/01076, source polynomial/overflow debris |
| QUICTRIG end | PC03605,Q3,A=extended ITEMP1,L=old COSCDUY low word,IRQ1 |

The actual CLOCKJOB source boundary is24,3332, with CADR QUICTRIG at24,3341. Its BANKCALL supplies **Q04675 (SWRETURN)** to QUICTRIG, while **BUF2(0133/0134)=03342/050000** retains the caller's return and bank. At QUICTRIG entry, BB046006 and EB03000 are source-derived. QUICTRIG preserves BUF2. The owner of CLOCKJOB control must execute the TC A/SWRETURN/TC BUF2 bridge before CKNVWRD1 at24,3342; passing3342 directly as QUICTRIG's Q loses the real source return state.

The separate CM fixture overlays original POWERED_FLIGHT_SUBROUTINES, SINGLE_PRECISION_SUBROUTINES and P40-P47 onto the pinned supporting source domain. Only1/0/7 pseudo-label indentation repairs are recorded. The complete rope is identical to pinned2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79. None of these routines uses ALTVAR/R53FLBIT; this qualification does not change the separate globally-original ERASABLE_ASSIGNMENTS domain. The historical original source tree remains untouched. The reference runner uses the pinned host-only GPL Virtual AGC engine, with its documented hardware-DV branch selection; native code contains no copied instruction engine.

The one deterministic compressed main corpus serves the reference/native/sanitizer logical stream names, whose exact SHA identities and successful whole-stream comparisons are recorded in `evidence/cm-trig-proof.json`. Source/native/sanitizer CLOCKJOB qualification and actual target PRGs, maps, mailboxes and stack measurements are retained separately. `evidence/cm-trig-evidence.sha256` covers the package.

Reproduce with `python tools/cm_trig_prepare_reference.py`, `python tools/prepare_hardware_dv.py`, `make -f tools/cm_trig.mk test clock-test check-host sanitize clock-sanitize`, `python tools/freeze_cm_trig.py`, and the `c64`/matching VICE targets for selectors0..3 (default0). This qualifies the complete numerical contents of SINGLE_PRECISION_SUBROUTINES, whose only public entries are SPSIN and SPCOS; its final TC Q return still belongs to a caller control bridge. QUICTRIG is one service in POWERED_FLIGHT_SUBROUTINES, not whole-file coverage for that larger source file. Standalone diagnostics do not certify complete resident Comanche or Luminary applications.
