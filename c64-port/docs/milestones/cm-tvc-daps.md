---
id: cm-tvc-daps
title: Translate Comanche TVCDAPS (CM pitch and yaw TVC autopilot: rate estimate, cascade filters, actuator commands)
date: 2026-10-12T18:30Z
lane: G
---
Native C for all of Comanche055 TVCDAPS (bank 20 `2530`-`3500`, 489 words): the PITCHDAP and YAWDAP T5 tasks (CDU rate limit, body-rate error, error limit, the three-cascade forward filter with VARK gain, offsets, actuator limit, TVC commands on TVCPITCH/TVCYAW with the channel 14 drive bits, precompensation and the DELBAR filter), the DAPINIT T5 task and the PCOPY/YCOPY copy cycles, from every entry to HACK, RESUME, NOQRSM or the caller's return. The final image and channel 14 equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN; all 489 words, 32 fixed constants, the TC Q dispatch and HACK returns are checked against the listing and rope; a C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-TVC-DAPS.md`.
