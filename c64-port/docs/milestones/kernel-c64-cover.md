---
id: kernel-c64-cover
title: Kernel C64 rows by branch cover, flight edges, and the #143/#144 fixes
date: 2026-10-13T04:31Z
lane: A
---
The C64 programs of the nine kernel milestones (`lm-t6rupt`, `rupt-lead-ins`, `keyrupt-uprupt` and the six Executive milestones) now run, beyond every third case, the fewest extra cases that take every branch arc the host corpus takes in every linked source (`tools/lane_a_c64_cover.py`); all nine pass in VICE. The Executive, Waitlist and T6 generators now produce flight edges on purpose (TIME3 wrap, tasks due together or 1 cs apart, equal priorities, NEWJOB -0, Q and QRUPT holding data, TIME6 1-3). QXCH now moves any Q (#143), and RESUME re-executes BRUPT only when it is the word at ZRUPT-1 (#144). The certificates of EXECUTIVE, WAITLIST, INTERRUPT_LEAD_INS, KEYRUPT_UPRUPT (both missions) and T6-RUPT_PROGRAMS cite the cover records and their VICE targets. See `docs/KERNEL-C64-COVER.md`.
