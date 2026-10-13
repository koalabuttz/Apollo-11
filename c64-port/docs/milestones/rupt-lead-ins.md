---
id: rupt-lead-ins
title: Run the interrupt lead-ins of both missions as native routines
date: 2026-10-12T21:00Z
lane: A
---
Every word of INTERRUPT_LEAD_INS.agc in Luminary099 and Comanche055 is native. The GOPROG restart lead-in, the T4, KEYRUPT1, KEYRUPT2, UPRUPT, DOWNRUPT, RADAR, RUPT10 and HANDRUPT vectors, the T5RUPT lead-ins and the Comanche T6RUPT vector are routines of the control-transfer ABI (`lm_lead_in_routines`, `cm_lead_in_routines`); the LM T6RUPT and both T3RUPT vectors were already native. Comanche's RESUME, NOQRSM and NOQBRSM interrupt exits join them. All 3,072 cases equal the original programs in yaAGC in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit and InIsr, and the original executed every instruction of both files in that comparison. The 8,992 strict and sanitizer checks pass, and every sixteenth case matches in VICE. Complete certificates are drafted for both files. See `docs/RUPT-LEAD-INS.md`.
