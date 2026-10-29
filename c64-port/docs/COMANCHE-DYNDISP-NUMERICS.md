# CM dynamic display numerical caller

The native CM dynamic display path now translates actual DYNDISP24,3325
through P40CNV85 and S41.1 to CKNVWRD1 at24,3342. It enters the original
interpreter, saves both source return words, pushes VGPREV, computes live
CDU sine/cosine, transforms through REFSMMAT, rotates through *SMNB*,
transforms through the original fixed QUADROT, multiplies by10.B-4,
shifts left5, stores VGBODY and returns through the actual basic EXIT/TCF.
QTEMP is physical03657; QTEMP1 is03730; VGPREV is03720..03725;
VGBODY is03664..03671; REFSMMAT is01735..01756. The source fixed
QUADROT operand starts at physical fixed18946; TENBNK14 is{024000,0}.

`cm_dyndisp_stage` provides seventeen explicit compiled caller groups.
`cm_dyndisp_step` provides27 chronological native routes, including the
four stages of each MXV and the separately qualified CDUTRIG/*SMNB*
callees. Routes are native continuation identifiers, not AGC opcodes.
The canonical image is the sole persistent RAM. Caller producer exclusion
must cover every pending operation; linked route frames must return before
a target overlay replacement. Internal source-calculated bank, register,
CYR, EDOP, ADDRWD, POLISH, QPRET, MPAC and return debris remain exact.

| Caller stage | Source operation or bridge | End PC / LOC |
| --- | --- | --- |
| 0 | Actual basic INTPRET entry | 06037 /03326 |
| 1 | CALL P40CNV85 | 06037 /02370 |
| 2 | STQ QTEMP1 | 06030 /02371, pending SETPD |
| 3 | SETPD0 and source no-request continuation | 06037 /02373 |
| 4 | VLOAD VGPREV | 06030 /02374, pending PUSH |
| 5 | PUSH vector | 06030 /02374 |
| 6 | CALL S41.1 | 06037 /03431 |
| 7 | STQ QTEMP and CALL CDUTRIG | 06037 /03477 |
| 8 | Implicit VLOAD from pushdown | 06030 /03434 |
| 9 | REFSMMAT operand admission | 07303 /03436 |
| 10 | Pending CALL *SMNB* | 06037 /03644 |
| 11 | QUADROT operand admission | 07303 /03441 |
| 12 | Pending VXSC TENBNK14 | 06030 /03442 |
| 13 | VSL5 | 06030 /03443, pending GOTO |
| 14 | GOTO QTEMP | 06037 /02377 |
| 15 | STCALL VGBODY, QTEMP1 | 06037 /03330 |
| 16 | Actual EXIT and TCF CKNVWRD1 | 03342 /03330 |

The bounded domain is an admitted bank24/E6 source job, source vector
MODE077776, one of five valid VAC bases0401..0661, NEWJOB+0 and legal
SUPER0/20/40/60 plus IRQ0/1. Source entry PC/LOC, FB/EB/BB, BANKSET and
pending EDOP are checked for each caller group. Ordinary non-register RAM
and raw16-bit A/L/Q are caller inputs. Unsupported caller stages preserve
image and external metadata. Linked callee failures are native faults,
not rollback boundaries. Scheduling at genuine EDOP0/DANZIG checks and
pending NEWJOB composition remain separate runtime integration work.

The independent byte-original assembly oracle executes512 uninterrupted
paths and compares all2048 words plus SUPER/IRQ at27 checkpoints:
13,824 complete states. It covers five VAC contexts, both IRQ states, all
four source SUPER forms, arbitrary DP word signs/zeros and raw matrices,
CDU angles and VGPREV inputs. Strict and full ASAN/UBSAN output match
exactly, raw SHA256:
`b83a36e103510d48f0c61af4daa6c1121b82e6a325e5828fe81835affe7c4667`.
Normalized source identity is checked for the actual P40CNV85, DYNDISP and
S41.1 caller bodies. The original-declaration CM rope is
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.

68 frozen complete source caller transitions and204 unchanged invalid
contexts pass strict and sanitized host tests. The actual PAL C64
caller diagnostic passes all68, mailbox `a500004400`, PRG25,632 bytes,
hardware stack23 bytes excluding14 ROM bytes, software stack149 bytes,
and no additional zero-page allocation. The target diagnostic imports
original before-states and tests caller groups; the uninterrupted full
numerical path is qualified on the host. The matrix, CDU and rotation
callee target proofs remain independent. This packet ends before noun85
display selection/formatting and before actual Executive end processing.

Recipes are `tools/cm_dyndisp_differential.py`, `tools/freeze_cm_dyndisp.py`,
`tools/cm_dyndisp.mk` and `tools/cm_dyndisp_vice_test.py`. Exact source,
dependency and artifact identities are in `evidence/cm-dyndisp-proof.json`.
The GPL original engine is confined to the host oracle; native modules
are MIT integer C without AGC instruction decoding or emulated execution.
