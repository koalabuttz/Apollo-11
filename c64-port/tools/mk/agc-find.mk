# SPDX-License-Identifier: MIT
# c64-agc-find: the C64 resident's find (platform/c64/agc_find.S, built with
# AGC_TRANSFER_FIND_ASM) against agc_transfer.c's C find in llvm-mos's
# simulator, with cycles per lookup (tools/agc_find_check.c).
AGC_FIND_SIM_CC ?= $(subst mos-c64-clang,mos-sim-clang,$(MOS_CC))
AGC_FIND_SIM ?= $(subst mos-c64-clang,mos-sim,$(MOS_CC))
.PHONY: c64-agc-find
build/agc-find-check: tools/agc_find_check.c platform/c64/agc_find.S src/agc_transfer.c include/agc_transfer.h | build
	$(AGC_FIND_SIM_CC) -Oz -fno-static-stack -Wall -Wextra -Werror -Iinclude -Itests tools/agc_find_check.c platform/c64/agc_find.S -o $@
c64-agc-find: build/agc-find-check
	$(AGC_FIND_SIM) build/agc-find-check
