# Native abort flags and restart-group cleanup

`recovery_abort` translates FLAGS, MR.KLEAN, P00KLEAN, V37KLEAN and the
POODOO phase-four write boundary. FLAGS clears STATEFLG/REINTFLG/NODOFLAG.
MR.KLEAN deactivates all six groups; P00KLEAN selects groups1/3/4/5/6,
V37KLEAN selects1/3/5/6. POODOO writes phase-four035 through GOPOODOO entry.
Address/priority/base payloads remain intact. A/L/Q/BBANK and interrupt
permission retain original stores and return debris. Caller owns ordinary
word state and excludes scheduler/restart readers and producers.

All 49,152 original-assembly checkpoints compare all47 modeled words plus
14 surrounding guards. Every raw32768-bit-pattern flag word is tested,
with4,096 cases per phase route, signed zeros, eight E/S bank values and
256 return-Q addresses. Snapshot SHA256:
`21de976a77f90373f7a4b2e9044034bfa126859742fb8cb67c2bf5ce0f5a8c6b`.
The host reference links original assembly/engine and constructors only.
Original source remains unchanged. This is bounded abort cleanup, not full
ENEMA, SERVICER, display or restart execution.

128 frozen complete states pass host, ASan/UBSan and PAL C64 VICE.
Mailbox `a50000802f` records zero failures. PRG19,299 bytes; text7,142,
rodata12,143/BSS100/static-stack78. Measured dynamic hardware/software
stack extents10/6 bytes exclude static storage; no extra compiler ZP.
No application timing or full-image memory budget is claimed.

Use `make test-recovery`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-recovery`,
`python3 tools/recovery_differential.py deps/fixture`, `make c64-recovery`
and `tools/recovery_vice_test.py` with standard ROM/VICE arguments.
Complete snapshots are deterministic gzip under `evidence/recovery-*`;
logs, binary/map and target dumps are retained. All earlier hashes and
historical sources are unchanged. Native code has no decoder or GPL linkage.
