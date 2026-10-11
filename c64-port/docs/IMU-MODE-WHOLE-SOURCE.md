# Luminary and Comanche IMU mode-switching local-source closure

All 660 emitted words in original Luminary099 and all 656 emitted words in
original Comanche055 `IMU_MODE_SWITCHING_ROUTINES.agc` now have explicit native
owners. The additive ledgers verify each original instruction/data word against
the unchanged declaration-qualified rope. No local word remains uncovered.

| Native packet | LM words | CM words |
| --- | ---: | ---: |
| Mode control and initialization | 142 | 141 |
| IMUPULSE lead-in and reservation | 51 | 51 |
| Coarse alignment and command segmentation | 120 | 120 |
| Stall and pulse activation controls | 37 | 36 |
| IMU completion and failure | 40 | 38 |
| Gyro wake lead-in | 9 | 9 |
| Deferred gyro task and registration | 153 | 153 |
| Attitude admission and R02 | 36 | 36 |
| Standby and local scaler/time recovery | 79 | 79 |

The table includes seven shared helper words per mission (LGWAKE, NORESET and
five CAGETEST words), independently qualified in overlapping packets. The
distinct physical-word union remains 660 LM and 656 CM; table rows are not
additive.

The qualified native bodies preserve the complete canonical source image, raw
A/L, original bank controls, IRQ/SUPER and source hardware latches in each
packet's declared domain. They compile local call, branch, exchange, arithmetic,
flag setup, pulse/coarse command segmentation, time recovery and data operations
from source operands. Original fixed address/CADR/passive data words have owners.
No expected flight answer or opcode decoder replaces these computations.

The frozen packets retain independent unchanged-original/full strict/full
ASAN/UBSAN complete-state streams, actual stock PAL C64 PRGs/maps/mailboxes,
source-word maps, measured stack bounds and independent reviews. Their source
footprints, compiler qualifications and input contracts remain immutable. The
final standby packet adds 2,752,512 complete original states and 42 actual C64
diagnostics with 1344 complete source-frozen states, 504 rejected-input calls
and 420 original passive data checks. The independent containing-file review
reads both originals and checks all 1316 distinct local words against the ropes.

The contracts require ordinary canonical data, coherent source banks and the
specified compiled caller/helper-return frames. Command and I/O producers remain
excluded during each operation. Pulse reservation/task/device and coarse stages
retain their documented idle/activity/counter domains; this closure does not
broaden them to arbitrary asynchronous hardware contexts. Standby consumes the
actual FINETIME/SHORTMP/TPAGREE return ABI at explicit compiled return entries.
It does not claim physical scaler sampling or the hardware standby transition.
Channel13 trap-enable latches are excluded from the final readable-I/O contract.

The real external ALARM/POODOO, SETISSW, MODEEXIT, WAITLIST/scheduler/task return,
flag, bank, phase, display, FINETIME and arithmetic helper bodies remain typed
continuations in their containing modules. Every local instruction leading to
those entries is compiled and qualified. External source bodies are not silently
simulated or credited to these files. Native internal returns retain canonical
ownership and introduce no source Executive checks.

This recommends certification of both complete containing files' local source
coverage after root review and integration. It does not claim autonomous execution
of either complete Apollo application, arbitrary caller-opcode dispatch or
unbounded physical-device operation. The complete-port acceptance gate remains
unchanged and requires every other source and mission integration obligation.
