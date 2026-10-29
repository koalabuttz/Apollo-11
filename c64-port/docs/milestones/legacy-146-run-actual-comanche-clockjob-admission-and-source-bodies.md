---
id: legacy-146-run-actual-comanche-clockjob-admission-and-source-bodies
title: Run actual Comanche CLOCKJOB admission and source bodies on C64 disk
seq: 146
---

Nonzero-NV source timer cohorts actually admit and execute CLOCKJOB through QUICTRIG, SWRETURN and display control to MAKEPLAY. Separate zero-NV CLOCKJOB entries finish and restore the suspended interpreter. All 448 source sequences and 3,392 complete checkpoints match host and full sanitizers; real C64 D64 replay matches 106 checkpoints across fourteen paths plus the complete final image. Downstream MAKEPLAY/rendering and other flight programs remain pending. See `docs/COMANCHE_CLOCK_MISSION.md`.

