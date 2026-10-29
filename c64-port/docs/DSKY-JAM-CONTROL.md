# Native programmatic proceed and terminate

`dsky_jam_control` and `cm_jam_control` execute actual fixed-fixed JAMPROC
and JAMTERM through source SUPERBNK write, request setup, POSTJUMP and the
complete source VBPROC/VBTERM→RECALTST→ENDOFJOB entry. The code composes the
unchanged qualified load-control body and its real internal RELDSP/JOBWAKE
services. JAMPROC sets REQRET33; JAMTERM sets34; both set DSPCOUNT-19 and
SUPER100, exactly as original. Actual channel7 write and channel11 events are
reported separately, including caller SUPER transitions from all eight legal
latch forms. Actual canonical FB/BB/CPU/scratch and sleeping-job state remain
source authority, with restoring producer exclusion held by the caller.

Inputs accept all aligned caller FB/EB, eight SUPER forms and IRQ0/1. Only
selected DSPLIST/CADRSTOR wakes require the independently documented wake
domains and CM absence of a preexisting Executive alarm. Unused Executive
fields impose no guard; signed-zero request/list cases and original no-match
LOCCTR-1 response data exchanges remain supported. Invalid inputs preserve
complete image, metadata and both optional event outputs; the CM event
pointers and source SUPER-write pointer must be supplied.

8192 complete2053-field unchanged original/native/fullASan/UBSan checkpoints
match all2048 words plus SUPER, IRQ, channel11, its event and channel7 event.
Full SHA256:
`6461c3cd5dfe36f307d89bd18848598a7b7330425574b37e9617dd515c4a8317`.
Each original mission is rebuilt independently with seven-file original
source audits and complete pinned-rope equality. The oracle excludes native
bodies and executes actual source transfers through final ENDOFJOB entry.

Sixty-four frozen complete source states pass strict host, sanitizers and
native stock-ROM C64 VICE. Mailbox a500004002; PRG25204 loaded25202 bytes;
hardware/software stack extents40/787. The diagnostic includes two complete
canonical images, both mission Executive bodies and frozen checkpoints;
production retains one owned canonical image. tools/dsky_jam_control.mk
provides portable host aliases and an explicit C64 target. Actual Executive
job completion remains its separate source continuation, not a fake return.
