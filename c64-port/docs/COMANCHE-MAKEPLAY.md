# Canonical Comanche MAKEPLAY control

`cm_makeplay_priority`, `cm_makeplay_select`, and `cm_makeplay_wake_return`
translate the original MAKEPLAY prefix, normal/clock display selection, and
XCHTOEND continuation. The bodies use the sole canonical image and explicit
CM io metadata. Producer exclusion is caller-owned; no scheduler exchange or
pending-job poll is invented inside these basic routines.

The first call saves USERPRIO and enters actual PRIOCHNG05210 with caller
return02775. The selection body accepts the normal, CLOCPLAY, and GOFLASH
flags, checks duplicate LINUS requests and flash legality, copies the original
normal display state, and ends at actual JOBWAKE05201, GOSLEEPS10,03176,
ENDOFJOB05217, or POODOO05721 with alarm01502. The wake-return body replaces
a found sleeping continuation with the actual END instruction and proceeds
to GOPLAY10,03312. All source A/L/Q, IRQ, channel11 and overlapping MPAC/core
scratch effects are retained. The public header defines supported raw flag,
priority, DSPLOCK, bank and wake-displacement domains. Unsupported inputs
preserve the image and complete io record.

The unchanged original assembly corpus compares 24,576 complete 2,051-word
states. It covers raw priority sign/zero corners, USERPRIO0, both channel11
signs, full caller L patterns, all EB/SUPER values, duplicate and rejected
flash requests, both PMMASK sign branches, no sleeping match, every selected
wake slot and LOCCTR0. Strict checks freeze 64 full original states and reject
34 invalid boundaries without mutation. All source states agree under
ASAN/UBSAN. Uncompressed stream SHA256:
`d01d3913843c726f378dba8ff82a33dbce76a1967dd0cf0ed6c9ceb3bf2f4857`.

The differential runner compares every complete output line while writing
native and original streams directly to deterministic XZ files, avoiding
large raw duplicates. The freezer likewise reads the compressed source and
input stream with bounded working memory. The C64 PAL VICE diagnostic checks
all 64 frozen complete states; exact artifacts and resource measurements are
in `evidence/cm-makeplay-proof.json`. Recipes are in `tools/cm_makeplay.mk`.
Native code contains no AGC decoder, emulator, floating point or heap.

Canonical priority, wake and sleep service bodies execute separately at the
actual source entrances. This milestone does not execute the GOPLAY/Pinball
consumer, higher priority or mark display branches, astronaut response bodies,
or POODOO recovery.
