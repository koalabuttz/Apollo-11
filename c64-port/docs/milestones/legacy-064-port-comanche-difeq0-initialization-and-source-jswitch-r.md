---
id: legacy-064-port-comanche-difeq0-initialization-and-source-jswitch-r
title: Port Comanche DIFEQ0 initialization and source JSWITCH routing
seq: 64
---

Native orbital initialization and JSWITCH BON/GOTO routing reproduce all 5,120 complete 2,048-word original images plus IRQ from 1,024 calls, SHA 5a0688f511a9c6097a5aeafe9b8ac628c6ff9598345b029c40e0471e0d1cb37c. Strict and full-source sanitizer checks plus four actual C64 images containing 128 original phase entry/exit states pass. Phases0..2 reach genuine DANZIG; phase3 restores IRQ and returns to NEWOPS at original DOW/ACCOMP without inserting a job check. Maximum target image ends0409B, hardware20/software44 stack bytes, zero additional zero page. Actual PRGs and all original-source proof artifacts are retained. See `docs/COMANCHE-ORBITAL-INITIAL.md`.

