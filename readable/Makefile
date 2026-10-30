# SPDX-License-Identifier: MIT
# The readable C (README.md). Run from readable/.
#
#   make check            the kernel cases, then the x86 quick scenarios
#   make kernel-cases     every case of the resident-kernel corpus on the
#                         readable Executive and WAITLIST (seconds)
#   make x86-flight       the x86_64 flight program with the readable
#                         Executive and WAITLIST linked in
#   make x86-scenarios-quick / x86-scenarios
#                         the x86 exact scenario suite on that program (needs
#                         the port's toolchain: c64-port/tools/setup_cloud.sh)
#
# The readable sources build with READABLE_CFLAGS; the faithful port's
# sources they are tested with build with the port's own flags.
PORT := ../c64-port
BUILD := build
CC ?= cc
READABLE_CFLAGS := -std=c99 -O2 -g -Wall -Wextra -Werror -pedantic
PORT_CFLAGS := -O2 -g -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic
INCLUDES := -I$(PORT)/include -I$(PORT)/src -I$(PORT)/tests
LM := -DLUMINARY099=1
CM := -DCOMANCHE055=1

SOURCES := src/agc/agc.c src/executive/executive.c src/waitlist/waitlist.c src/binding/binding.c
HEADERS := $(wildcard src/*/*.h)

.PHONY: all check kernel-cases x86-flight x86-scenarios-quick x86-scenarios format clean
all: kernel-cases
check: kernel-cases x86-scenarios-quick

# ---- The resident-kernel corpus ----
# c64-port's test (tests/test_resident_kernel.c) over every case of the six
# certified service milestones: each run must match the original program's
# yaAGC digest and the certified routines' final image word for word. Here
# the readable objects stand in for src/lm_kernel.c and src/cm_kernel.c.
RK_MK := $(PORT)/tools/mk/resident-kernel.mk
RK_LM_CASES := $(addprefix $(PORT)/,$(shell sed -n 's/^RK_LM_CASES = //p' $(RK_MK)))
RK_CM_CASES := $(addprefix $(PORT)/,$(shell sed -n 's/^RK_CM_CASES = //p' $(RK_MK)))
RK_COMMON := $(PORT)/src/agc_image_ops.c $(PORT)/tests/resident_kernel_cases.c $(PORT)/tests/test_resident_kernel.c
RK_LM_FLAGS := -DRK_LM=1 -DP66_NATIVE_DISPATCH_ONLY -DRK_SEAMS=1 -ffunction-sections -fdata-sections -Wl,--gc-sections
RK_CM_FLAGS := -DRK_CM=1 -DRK_SEAMS=1 -ffunction-sections -fdata-sections -Wl,--gc-sections

$(BUILD)/cases-lm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -DRK_SEAMS=1 -c $< -o $@
$(BUILD)/cases-cm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(CM) -DRK_SEAMS=1 -c $< -o $@

$(BUILD)/kernel-cases-lm: $(SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(RK_LM_CASES) $(RK_COMMON)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RK_LM_FLAGS) $(RK_LM_CASES) $(RK_COMMON) $(filter %.o,$^) -o $@
$(BUILD)/kernel-cases-cm: $(SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(RK_CM_CASES) $(RK_COMMON)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RK_CM_FLAGS) $(RK_CM_CASES) $(RK_COMMON) $(filter %.o,$^) -o $@

kernel-cases: $(BUILD)/kernel-cases-lm $(BUILD)/kernel-cases-cm
	./$(BUILD)/kernel-cases-lm
	./$(BUILD)/kernel-cases-cm

# ---- The x86_64 flight program ----
# tools/build_x86.py builds the port's x86 program with these objects in
# place of the faithful kernel (build/x86-flight/).
$(BUILD)/x86-lm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -c $< -o $@
$(BUILD)/x86-cm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(CM) -c $< -o $@
X86_OBJECTS := $(SOURCES:src/%.c=$(BUILD)/x86-lm/%.o) $(SOURCES:src/%.c=$(BUILD)/x86-cm/%.o)

# The harness's frozen-clock yaAGC engine (the port builds it).
HARNESS_ENGINE := $(PORT)/build/agc_engine_hardware_dv.o
$(HARNESS_ENGINE):
	$(MAKE) -C $(PORT) SKIP_EVIDENCE_RESTORE=1 build/agc_engine_hardware_dv.o

x86-flight: $(X86_OBJECTS)
	python3 tests/x86_readable.py build

x86-scenarios-quick: $(X86_OBJECTS) $(HARNESS_ENGINE)
	python3 tests/x86_readable.py scenarios --quick

x86-scenarios: $(X86_OBJECTS) $(HARNESS_ENGINE)
	python3 tests/x86_readable.py scenarios

format:
	$(PORT)/deps/llvm-mos/bin/clang-format -i --style=file:$(PORT)/.clang-format $(SOURCES) $(HEADERS)

clean:
	rm -rf $(BUILD)
