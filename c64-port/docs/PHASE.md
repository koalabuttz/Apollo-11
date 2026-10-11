# Native phase-table maintenance

`phase_change` translates PHASCHNG and 2PHSCHNG through their final
RELINT/DTCB return. It updates all Type A/B/C phase pairs, priorities,
addresses and counter-relative bases for groups 1..6. The first double-change
word must select Type A. Caller owns stable ordinary-word tables, counters
and parameter stream and excludes concurrent readers/producers. Invalid
words, groups, lengths and register inputs leave all state unchanged.
The API does not execute a restart or display a mode.

All 49,152 original-assembly checkpoints compare all 56 modeled words and
nine surrounding guards. Every raw phase word selecting a legal group is
checked at both entries; signed zeros, both IRQ states and all BBANK fixed
banks/EBANK/SUPERBNK values are covered. Original source is
PHASE_TABLE_MAINTENANCE:202..409; entries PHASCHNG5353/2PHSCHNG5327.
The GPL host oracle links input construction and original engine only.
Snapshot SHA256:
`db5766d2d5738ac4c0110f9baa0c00283fee31e844c8ac7876da278e17467dab`.

128 frozen complete states pass host, ASan/UBSan and PAL C64 VICE. All8,192
invalid group words reject unchanged; malformed stream/register cases pass.
Mailbox `a500008038` records zero failures. PRG23,066 bytes; text8,614,
rodata14,438, BSS129, noinit78 including compiler static stack. Measured
dynamic hardware/software stack extents10/13 bytes exclude static storage;
no extra compiler ZP. This is isolated diagnostic capacity, not a full
application timing or memory result.

Use `make test-phase`, `ASAN_OPTIONS=detect_leaks=0 make sanitize-phase`,
`python3 tools/phase_differential.py deps/fixture`, `make c64-phase` and
`tools/phase_vice_test.py` with standard VICE/ROM arguments. Complete raw
snapshots and the long reference trace are retained as deterministic gzip
under `evidence/phase-*`; other logs, PRG/map and target dumps are retained.
All earlier evidence identities and historical sources remain unchanged.
Native phase code has no AGC decoder and never links the GPL oracle.
