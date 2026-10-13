# SERVICE_ROUTINES in the transfer ABI (lane A)

Every entry of `Luminary099/SERVICE_ROUTINES.agc` and
`Comanche055/SERVICE_ROUTINES.agc` is a control-transfer ABI routine at its
original address (issue #129). Each one runs the certified
`service_routines_step` (GENTRAN: one iteration of `service_routines_block`)
and leaves Z at the continuation, where the trampoline enters that point's own
routine.

| entry | LM | CM | reached by |
|---|---|---|---|
| UPENT2, DOWNENT2 | 10,3544 and 10,3554 | 07722, 07740 | `TC` with Q a return address |
| UPFLAG, DOWNFLAG | 05504, 05516 | 05546, 05560 | `TC UPFLAG` / `TC DOWNFLAG` then `ADRES flag` (73 and 96 LM sites) |
| DELAYJOB | 00,3735 | 00,3732 | `TC BANKCALL` `CADR DELAYJOB` |
| TCGETCAD | 00,3761 | 00,3755 | a woken job (TCSLEEP) |
| WAKER, WAKER +4 | 00,3765, 00,3771 | 00,3761, 00,3765 | the WAITLIST task (WAKECAD), JOBWAKE's return |
| GENTRAN, GENTRAN +1 | 05544, 05545 | 05606, 05607 | `TC GENTRAN` then two `ADRES` words; the back edge |
| B5OFF | 05563 | 05625 | `TC B5OFF` from extended verbs |
| TRFAILOF, TRFAILON | | 05631, 05643 | `TC` with Q a return address |

`src/service_transfer.c` holds `lm_service_routines` (11 entries) and
`cm_service_routines` (13). The `ADRES` words after every genuine fixed
`TC UPFLAG`, `TC DOWNFLAG` and `TC GENTRAN` are compiled constants
(`src/lm_service_sites.inc`, 171 words; `src/cm_service_sites.inc`, 152;
`tools/agc_sites_generate.py lm-service|cm-service`). GENTRAN's fixed source
words come from the program's passive constant reader. A fixed Q that is no
genuine site is refused.

TCGETCAD, WAKER, its finish after JOBWAKE and B5OFF never read Q (their first
`TC` overwrites it, or they end in ENDOFJOB or TASKOVER), so
`service_routines_step` now admits any Q for them. A waitlist task or a woken
job reaches them with Q left by the executive (yaAGC runs WAKER and B5OFF at
Q = 3). The other entries keep the compiled-caller range 0100..07775.

## Proof

`tools/service_transfer_freeze.py` runs the 1,642 cases of
`tests/service_transfer_cases.c` through `agc_transfer_run` and in yaAGC from
the original ropes. There are two cases per genuine UPFLAG, DOWNFLAG and
GENTRAN site of each mission, entered at the callee with Q at the operand word
and random erasable, banks and interrupt inhibit. There are also 256 entry
cases per mission over the other entries, half of the Q-free ones with Q below
0100 and with free and busy delay slots. yaAGC stops where Z and FB first equal
the native continuation. 1,640 cases reach it; the other two are GENTRAN
blocks whose operands the step refuses. The inhibit flag and all 2048 erasable
words are identical. The proof
(`evidence/service-transfer/service-transfer-proof.json`) lists the hashes of
every native source it depends on.

`tests/test_service_transfer.c` checks the frozen digests, the refusals (UPFLAG
and DOWNFLAG at Q 077 and at a fixed Q that is no site) and the table order:
1,654 checks. The C64 diagnostic runs every fifth case and the refusals.
