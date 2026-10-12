---
id: cm-entry-dap
title: Translate Comanche CM_ENTRY_DIGITAL_AUTOPILOT (CM entry DAP: body rates, attitude, dead zones, roll DAP)
date: 2026-10-12T23:00Z
lane: G
---
Native C for all of Comanche055 CM_ENTRY_DIGITAL_AUTOPILOT (bank 15 `2434`-`3754` and bank 20 `3612`-`3737`, 807 words). This covers the READGYMB/BODYRATE CDU and body-rate task and the ATTRATES T5 attitude update, with the pitch/yaw dead zones and the exo-atmospheric DAP. It also covers the CM/RCS roll DAP and its JETCALL jet timing, the FDAI and telemetry dump, and the CM/DAPON/CM/DAPIC start-up. Every entry runs to its waitlist, executive, single-precision or restart frontier. The final image and channels 5/6 equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN. All 807 words, 108 fixed constants, the 2CADRs, the INDEX tables, the TC Q dispatch and the frontier returns are checked against the listing and the rope. A C64 diagnostic passes 136 frozen oracle rows in VICE. See `docs/CM-ENTRY-DAP.md`.
