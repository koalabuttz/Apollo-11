# SYSTEM_TEST_STANDARD_LEAD_INS against yaAGC (both missions)

Lane A, milestone `standard-lead-oracle`. Re-certification evidence for the
STANDARD_LEAD_INS items of #129: every registered entry of the file in both
missions, and its interpretive CALL and EXIT words, run through
`agc_transfer_run` and compared with the original program in yaAGC.

## Cases

`tests/standard_lead_oracle_cases.c` builds 1,408 cases: 64 for each of 11
kinds in each mission (case `n`: mission `n % 2`, kind `(n / 2) % 11`).

| kind | entry | reached (LM / CM) |
|---|---|---|
| E/BKCALL | basic entry | 64 / 64 |
| RETURN0, RETURN1, RETURN2 | basic entries | 64 / 64 each |
| E/CALL | basic entry, into the interpreter | 62 / 62 |
| E/CALL resume | the word after EXIT (LXCH EBUF2 +1) | 64 / 64 |
| E/JOBWAK | basic entry, through JOBWAKE | 64 / 33 |
| INTPRET, INTPRET +2 | the interpreter entries E/CALL reaches | 64 / 64 each |
| CALL word | interpretive, LOC at the CALL under BANKSET | 64 / 64 |
| EXIT word | interpretive, LOC at the EXIT under BANKSET | 64 / 64 |

1,373 cases reach a continuation outside the file; the other 35 are native
refusals (out of the supported domain, left unchanged), printed as such by
both sides. A case reaches when the run ends with `EXEC_OK` or with
`EXEC_NOT_FOUND` at an address no routine owns (E/JOBWAK continuing into a
job the image's NEWJOB names). Images come from a 16-bit xorshift; bank
registers, BANKSET, LOC and the callee CADRs are set per kind so the
interpreter is entered with BBANK equal to BANKSET (the fetch point reads LOC
under the current FB).

Native changes for this milestone: `standard_lead_interp_call_order()`
(`src/standard_lead_interp.c`) runs the CALL word at the fetch point (like
`interp_call`, but taking the bank from BANKSET at 0165 and leaving Q and the
interrupt state alone); the CALL and EXIT words are registered as
interpretive keys at 04,2624/04,2626 (LM) and 13,2064/13,2066 (CM) in
`src/standard_lead_routines.c`; CALL and EXIT require FB/EB (word 6) equal to
BANKSET's bank bits.

## Oracle and checks

`tools/standard_lead_oracle_reference.c` loads each case into yaAGC with the
original rope and runs until Z and FB first equal the native continuation.
`tools/standard_lead_oracle_freeze.py` requires the two outputs byte-identical
(the reach flag, the interrupt inhibit, SUPERBNK and all 2048 erasable words)
and freezes one Fletcher digest per case in
`tests/standard_lead_oracle_vectors.h`. `make test-standard-lead-oracle` and
`make sanitize-standard-lead-oracle` check every digest (1,414 checks).

The C64 diagnostic runs every third case plus the 4 cover rows that
`tools/lane_a_c64_cover.py` adds so that the C64 rows take all 664 branch
arcs the host corpus takes (`evidence/standard-lead-oracle/standard-lead-oracle-c64-cover.json`);
`make vice-standard-lead-oracle` passes 477 checks in VICE.

## Source map

`tools/lane_a_source_map.py standard-lead-oracle --check` maps every
assembled word of both missions' file to the registered native routine and
records whether the oracle executed it (the oracle writes the fixed words it
runs and the interpretive words its fetch point reads, through
`tools/lane_a_executed.h`). Every LM instruction ran. In Comanche055, COMPVER,
GTSCPSS1 and REDO (33,2000-2002) run in milestone `cm-lead-tests`, and
REDO's `MM 07` is the operand NEWMODEX reads.
