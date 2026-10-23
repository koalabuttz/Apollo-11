# SPDX-License-Identifier: MIT
# c64-resident-cdu-asm: the C64 resident's output drives
# (platform/c64/resident_cdu.S) against yaAGC's IMU CDU vectors and
# src/resident_cdu.c's C drive in llvm-mos's simulator, built once for each
# mission (tools/resident_cdu_asm_check.c).
RESIDENT_CDU_SIM_CC ?= $(subst mos-c64-clang,mos-sim-clang,$(MOS_CC))
RESIDENT_CDU_SIM ?= $(subst mos-c64-clang,mos-sim,$(MOS_CC))
RESIDENT_CDU_ASM_SRC := tools/resident_cdu_asm_check.c tests/resident_cdu_cases.c platform/c64/resident_cdu.S
.PHONY: c64-resident-cdu-asm
build/resident-cdu-asm-lm build/resident-cdu-asm-cm: build/resident-cdu-asm-%: $(RESIDENT_CDU_ASM_SRC) src/resident_cdu.c \
  include/resident_cdu.h tests/resident_cdu_cases.h tests/resident_cdu_vectors.h | build
	$(RESIDENT_CDU_SIM_CC) -Oz -fno-static-stack -Wall -Wextra -Werror -DRK_$(shell echo $* | tr a-z A-Z)=1 \
	  -Iinclude -Itests $(RESIDENT_CDU_ASM_SRC) -o $@
c64-resident-cdu-asm: build/resident-cdu-asm-lm build/resident-cdu-asm-cm
	$(RESIDENT_CDU_SIM) build/resident-cdu-asm-lm
	$(RESIDENT_CDU_SIM) build/resident-cdu-asm-cm
