---
id: lanec-ev-track
title: Translate the basic code of the tracking and state-vector verbs
date: 2026-10-12T16:20Z
lane: C
---
Native C for the basic code around the interpretive EXTENDED_VERBS bodies: V69, LM TRMTRAK1, DSPRRLOS/RRLOSDSP and ATTACHIT, CM TRACKTRM, CLEANOUT, SETVHFLG, RESETVHF, ATTACHIT, TACHEXIT, LMTOCMSV and LMTOCM, translated block by block up to each TC INTPRET and from each interpretive EXIT. 2,304 complete states are identical to the unchanged original program in Virtual AGC, in strict and ASan/UBSan builds, and 210 of them (every entry) match the oracle on a VICE PAL C64. See `docs/LANEC-EV-TRACK.md`.
