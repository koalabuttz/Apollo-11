# SPDX-License-Identifier: MIT
# The readable C (README.md). Run from readable/.
#
#   make check            the kernel cases, then the x86 quick scenarios
#   make kernel-cases     every case of the resident-kernel corpus on the
#                         readable Executive and WAITLIST (seconds)
#   make interbank-cases  the readable inter-bank routines against the
#                         faithful port's, case by case (seconds)
#   make phase-cases      the readable phase table maintenance against the
#                         original program's digests and the faithful port's
#                         routines, case by case (seconds)
#   make restart-cases    the readable restarts against the original program's
#                         digests and the faithful port's routines (seconds)
#   make names-check      src/agc/<m>_names.inc against the listings
#   make coverage         how much of the program is readable C (COVERAGE)
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

KERNEL_SOURCES := src/agc/agc.c src/executive/executive.c src/waitlist/waitlist.c \
	src/binding/binding.c src/binding/machine.c
INTERBANK_SOURCES := src/agc/agc.c src/interbank/interbank.c src/binding/machine.c \
	src/binding/interbank_tables.c
PHASE_SOURCES := src/agc/agc.c src/phase_table/phase_table.c src/binding/machine.c \
	src/binding/phase_table_tables.c
RESTART_SOURCES := src/agc/agc.c src/restart/restarts.c src/restart/restart_tables.c \
	src/binding/machine.c src/binding/restart_routines.c
SOURCES := $(sort $(KERNEL_SOURCES) $(INTERBANK_SOURCES) $(PHASE_SOURCES) $(RESTART_SOURCES))
HEADERS := $(wildcard src/*/*.h)

.PHONY: all check kernel-cases interbank-cases phase-cases restart-cases names-check coverage x86-flight x86-scenarios-quick \
	x86-scenarios format clean
all: names-check coverage kernel-cases interbank-cases phase-cases restart-cases
check: all x86-scenarios-quick

names-check:
	python3 tools/listing_names.py --check
	python3 tools/restart_tables.py --check

coverage:
	python3 tools/readable_coverage.py --check

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

$(BUILD)/kernel-cases-lm: $(KERNEL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(RK_LM_CASES) $(RK_COMMON)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RK_LM_FLAGS) $(RK_LM_CASES) $(RK_COMMON) $(filter %.o,$^) -o $@
$(BUILD)/kernel-cases-cm: $(KERNEL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(RK_CM_CASES) $(RK_COMMON)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RK_CM_FLAGS) $(RK_CM_CASES) $(RK_COMMON) $(filter %.o,$^) -o $@

kernel-cases: $(BUILD)/kernel-cases-lm $(BUILD)/kernel-cases-cm
	./$(BUILD)/kernel-cases-lm
	./$(BUILD)/kernel-cases-cm

# ---- The inter-bank routines, case by case ----
# tests/interbank_cases.c runs each entry of the readable tables and of the
# faithful port's (built here with faithful_ before each table's name) on
# the same random states.
IB_LM_FAITHFUL := src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank_dap.c \
	src/lm_interbank.c
IB_CM_FAITHFUL := src/cm_interbank_routines.c src/cm_interbank_idle.c \
	src/cm_interbank_postjump.c src/cm_interbank_complete.c
IB_LM_NAMES := lm_interbank_routines lm_interbank_site_table lm_interbank_idle_chunk \
	lm_ib_bankcall lm_ib_swcall lm_ib_swcall_m1 lm_ib_swreturn lm_ib_postjump \
	lm_interbank_dap_hot_routines lm_interbank_postjump_hot_routines \
	lm_interbank_postjump_t4_hot_routines lm_phase_table_routines lm_phase_table_operands \
	lm_restart_routines lm_restart_operands lm_restart_select
IB_CM_NAMES := cm_interbank_routines cm_interbank_site_table cm_interbank_idle_chunk \
	cm_ib_bankcall cm_ib_swcall cm_ib_swcall_m1 cm_ib_swreturn cm_ib_postjump \
	cm_interbank_postjump_hot_routines cm_phase_table_routines cm_phase_table_operands \
	cm_restart_routines cm_restart_operands cm_restart_routines_select
IB_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c

$(BUILD)/faithful-lm/%.o: $(PORT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(IB_LM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/faithful-cm/%.o: $(PORT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(IB_CM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@

$(BUILD)/interbank-cases-lm: $(INTERBANK_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(IB_LM_FAITHFUL:src/%.c=$(BUILD)/faithful-lm/%.o) tests/interbank_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(IB_COMMON) $(filter %.o,$^) tests/interbank_cases.c -o $@
$(BUILD)/interbank-cases-cm: $(INTERBANK_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(IB_CM_FAITHFUL:src/%.c=$(BUILD)/faithful-cm/%.o) tests/interbank_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(IB_COMMON) $(filter %.o,$^) tests/interbank_cases.c -o $@

interbank-cases: $(BUILD)/interbank-cases-lm $(BUILD)/interbank-cases-cm
	./$(BUILD)/interbank-cases-lm
	./$(BUILD)/interbank-cases-cm

# ---- Phase table maintenance ----
# The port's own test (tests/test_<m>_phase_table.c) on the readable
# routines: every caller site of the original program against its yaAGC
# digest. Then tests/phase_table_cases.c: the readable routines and the
# faithful port's on the same random states.
PT_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c
PT_FAITHFUL_LM := src/lm_phase_table.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_multiword.c
PT_FAITHFUL_CM := src/cm_phase_table.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_multiword.c
PT_ORACLE_SOURCES := $(sort $(PHASE_SOURCES) $(INTERBANK_SOURCES))

$(BUILD)/phase-oracle-lm: $(PT_ORACLE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(PT_COMMON) $(PORT)/tests/lm_phase_table_cases.c \
		$(PORT)/tests/test_lm_phase_table.c $(filter %.o,$^) -o $@
$(BUILD)/phase-oracle-cm: $(PT_ORACLE_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(PT_COMMON) $(PORT)/tests/cm_phase_table_cases.c \
		$(PORT)/tests/test_cm_phase_table.c $(filter %.o,$^) -o $@
$(BUILD)/phase-cases-lm: $(PHASE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(PT_FAITHFUL_LM:src/%.c=$(BUILD)/faithful-lm/%.o) tests/phase_table_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(PT_COMMON) $(filter %.o,$^) tests/phase_table_cases.c -o $@
$(BUILD)/phase-cases-cm: $(PHASE_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(PT_FAITHFUL_CM:src/%.c=$(BUILD)/faithful-cm/%.o) tests/phase_table_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(PT_COMMON) $(filter %.o,$^) tests/phase_table_cases.c -o $@

phase-cases: $(BUILD)/phase-oracle-lm $(BUILD)/phase-oracle-cm $(BUILD)/phase-cases-lm \
		$(BUILD)/phase-cases-cm
	./$(BUILD)/phase-oracle-lm
	./$(BUILD)/phase-oracle-cm
	./$(BUILD)/phase-cases-lm
	./$(BUILD)/phase-cases-cm

# ---- Restarts ----
# The port's own tests (tests/test_lm_restart_routines.c, test_cm_restart_
# routines.c) on the readable routines: every restart table entry and
# variable phase against the original program's yaAGC digests. Then
# tests/restart_cases.c: the readable routines and the faithful port's on
# the same random states.
RS_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c
RS_FAITHFUL_LM := src/lm_restart_select.c src/lm_restart_routines.c
RS_FAITHFUL_CM := src/cm_restart_routines.c

$(BUILD)/restart-oracle-lm: $(RESTART_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(RS_COMMON) $(PORT)/tests/lm_restart_select_cases.c \
		$(PORT)/tests/lm_restart_routines_cases.c $(PORT)/tests/test_lm_restart_routines.c \
		$(filter %.o,$^) -o $@
$(BUILD)/restart-oracle-cm: $(RESTART_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(RS_COMMON) $(PORT)/tests/cm_restart_routines_cases.c \
		$(PORT)/tests/test_cm_restart_routines.c $(filter %.o,$^) -o $@
$(BUILD)/restart-cases-lm: $(RESTART_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(RS_FAITHFUL_LM:src/%.c=$(BUILD)/faithful-lm/%.o) tests/restart_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(RS_COMMON) $(filter %.o,$^) tests/restart_cases.c -o $@
$(BUILD)/restart-cases-cm: $(RESTART_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(RS_FAITHFUL_CM:src/%.c=$(BUILD)/faithful-cm/%.o) tests/restart_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(RS_COMMON) $(filter %.o,$^) tests/restart_cases.c -o $@

restart-cases: $(BUILD)/restart-oracle-lm $(BUILD)/restart-oracle-cm $(BUILD)/restart-cases-lm \
		$(BUILD)/restart-cases-cm
	./$(BUILD)/restart-oracle-lm
	./$(BUILD)/restart-oracle-cm
	./$(BUILD)/restart-cases-lm
	./$(BUILD)/restart-cases-cm

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
