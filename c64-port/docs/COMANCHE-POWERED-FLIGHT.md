# Comanche POWERED_FLIGHT_SUBROUTINES source review

All original entries, helpers, fixed constants and documented return paths in `Comanche055/POWERED_FLIGHT_SUBROUTINES.agc` now have native implementations and independent source qualification. This certifies the numerical source file within its documented entry contracts. It does not certify the complete resident Comanche application.

| Original source group | Native binding and qualification |
| --- | --- |
| CDUTRIG / CD*TR*G | `cm_powered_interface` plus source trig/reentry; all-bank `cm_powered_fixed_return` closes RVQ |
| CDUTRIGS / CD*TR*GS | `cm_powered_trig` canonical source phases |
| QUICTRIG | Immutable `cm_quictrig` requalified on original declarations; `cm_powered_quick_return` performs actual TC A |
| Legacy NBSM | `cm_nbsm` stages plus permanent TRG*NBSM composition and actual STCALL, including lower-bank P51 return |
| Six permanent transforms | `cm_powered_interface`, CDU trig/basic rotation and fixed RVQ return |
| AX*SR*T | `cm_powered_axis`, stored RTNSAVER return and immutable qualified raw rotation kernel |
| SMCDURES | `cm_smcdure` eighteen compiled source operations including zero/overflow DDV and RVQ |
| SINESLOC / INDEXI | `cm_powered_constant_read`, all five byte-original raw fixed words |

The independent source map assigns every one of187 emitted original words to a native group, including five data words. Original source line312 (the positive-zero TC RTNSAVER route) is unreachable from the documented +3/-3 direction: source DIM preserves sign and finishes with negative zero, taking source line314. The alternate word remains disclosed in the map. The original AX physical guarantee for vectors of magnitude belowunity is retained; additional raw overflow comparisons do not enlarge that guarantee.

The seven constituent packets match 2,187,264 complete 2048-word-plus-IRQ checkpoints against unchanged original assembly in both strict native and whole ASAN/UBSAN runs. 50 actual C64 diagnostic PRGs pass 1,600 complete frozen states. The five mixed-file constants are checked separately against emitted source words and on both QUICTRIG target images. Each proof preserves exact decoded state identities and actual target PRGs, maps, mailboxes and stack evidence. The byte-original declaration rope is `6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`; original ALTVAR01357/R53FLBIT=BIT8 is used. Previously frozen support-rope packets remain immutable.

RVQ enters GOTO+4 and supports all fixed FCADR banks, deriving INTBIT15 and A from the final bank. Legacy NBSM final STCALL uses indirect GOTO classification, so its qualified fixed-return domain starts004000. Basic TC A/RTNSAVER calls accept source fixed code addresses02000..07777. Other entry contracts require ordinary raw words, coherent source bank/control state and an owned source VAC. Stable S1 arrays and DTHETASM/trig producers remain excluded during their original operations. Five source VAC bases and all eight EB settings occur in the evidence.

The native phase boundaries retain original pending EDOP and exact PC/Q/bank controls. EDOP0 DANZIG points permit the external Executive check; pending operations and internal arithmetic/basic returns retain canonical ownership. The numerical oracles use original NEWJOB+0 to finish each call. Live scheduling, overlay admission, hardware producers and caller continuation code remain application responsibilities. No AGC opcode decoder, host floating point or private persistent continuation implements these routines.

Run the preparation and constituent make targets documented in their seven milestone files. `python tools/cm_powered_file_review.py` checks every emitted original word and five constants. `python tools/package_cm_powered_completion.py` assembles the four final immutable evidence packets after verification; it deduplicates each identical source/native/sanitizer stream into one lossless XZ object.
