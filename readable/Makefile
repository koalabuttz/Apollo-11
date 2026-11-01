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
#   make machine-cases    the readable machine's instructions against the
#                         faithful port's on random states (seconds)
#   make interpreter-cases
#                         the readable interpreter against the faithful port's
#                         interpreter core on random interpretive programs
#                         (a minute; needs the assembled ropes, which
#                         c64-port/tools/setup_cloud.sh makes)
#   make alarm-cases      the readable alarms and aborts against the original
#                         program's digests and the faithful port's routines
#                         (seconds)
#   make sp-cases         the readable SPSIN and SPCOS against the original
#                         program's return states and the faithful port's
#                         routines (seconds)
#   make service-cases    the readable service routines against the original
#                         program's digests and the faithful port's routines
#                         (seconds)
#   make downtelemetry-cases
#                         the readable down-telemetry against the original
#                         program's digests and the faithful port's routines
#                         (seconds)
#   make keyrupt-cases    the readable keyboard and uplink interrupts against
#                         the original program's digests and the faithful
#                         port's routines (seconds)
#   make rupt-cases       the readable interrupt lead-ins, RESUME and the LM's
#                         T6 interrupt against the original program's digests
#                         and the faithful port's routines (seconds)
#   make t4rupt-cases     the readable T4RUPT routines against the original
#                         program's digests and the faithful port's routines
#                         (seconds)
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
INTERPRETER_SOURCES := src/agc/agc.c $(sort $(wildcard src/interpreter/*.c)) src/interbank/interbank.c \
	src/binding/machine.c src/binding/interpreter_core.c src/binding/interpreter_basic.c
ALARM_SOURCES := src/agc/agc.c src/alarm/alarm.c src/interbank/interbank.c src/binding/machine.c \
	src/binding/interbank_tables.c src/binding/alarm_routines.c
SP_SOURCES := src/agc/agc.c src/single_precision/single_precision.c src/binding/machine.c \
	src/binding/sp_routines.c
SERVICE_SOURCES := src/agc/agc.c src/service/service.c $(sort $(wildcard src/interpreter/*.c)) \
	src/interbank/interbank.c src/binding/machine.c src/binding/service_routines.c
DNTM_SOURCES := src/agc/agc.c src/downtelemetry/downtelemetry.c src/downtelemetry/downlink_lists.c \
	src/binding/machine.c src/binding/downtelemetry_routines.c
FS_SOURCES := src/agc/agc.c src/fresh_start/fresh_start.c src/binding/machine.c \
	src/binding/fresh_start_routines.c
V37_SOURCES := $(FS_SOURCES) src/fresh_start/v37.c src/binding/v37_routines.c
STATEUP_SOURCES := $(INTERPRETER_SOURCES) src/fresh_start/stateup.c src/binding/stateup_routines.c
KEYRUPT_SOURCES := src/agc/agc.c src/keyrupt/keyrupt.c src/binding/machine.c src/binding/keyrupt_routines.c
T4RUPT_SOURCES := src/agc/agc.c src/t4rupt/t4rupt.c src/binding/machine.c src/binding/t4rupt_routines.c
T4IMU_SOURCES := src/agc/agc.c src/t4rupt/imu_monitor.c src/binding/machine.c \
	src/binding/t4rupt_imu_routines.c
T4RR_SOURCES := src/agc/agc.c src/t4rupt/rr_monitor.c src/binding/machine.c \
	src/binding/t4rupt_rr_routines.c
T4OPT_SOURCES := src/agc/agc.c src/t4rupt/optics.c src/binding/machine.c \
	src/binding/t4rupt_optics_routines.c
RCSMON_SOURCES := src/agc/agc.c src/rcs_failure_monitor/rcs_failure_monitor.c src/binding/machine.c \
	src/binding/rcs_failure_monitor_routines.c
AGS_SOURCES := src/agc/agc.c src/ags_initialization/ags_initialization.c src/binding/machine.c \
	$(INTERPRETER_SOURCES) src/binding/ags_initialization_routines.c
AOTMARK_SOURCES := $(INTERPRETER_SOURCES) src/aotmark/aotmark.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/aotmark_routines.c
SXTMARK_SOURCES := src/agc/agc.c src/sxtmark/sxtmark.c src/binding/machine.c \
	src/binding/sxtmark_routines.c
EV_SOURCES := $(INTERPRETER_SOURCES) src/extended_verbs/front.c src/extended_verbs/short_verbs.c \
	src/extended_verbs/align.c src/extended_verbs/radar.c src/extended_verbs/dap.c \
	src/extended_verbs/time.c src/extended_verbs/track.c \
	src/binding/extended_verbs_routines.c
NOUNS_SOURCES := src/agc/agc.c src/pinball_noun_tables/readers.c src/pinball_noun_tables/tables.c \
	src/binding/noun_reader_routines.c
GEOMETRY_SOURCES := $(INTERPRETER_SOURCES) src/geometry/geometry.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/geometry_routines.c
IMU_COMP_SOURCES := src/agc/agc.c src/imu_compensation/imu_compensation.c src/binding/machine.c \
	src/binding/imu_compensation_routines.c
R63_SOURCES := $(INTERPRETER_SOURCES) src/r63/r63.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/r63_routines.c
AMR_SOURCES := $(INTERPRETER_SOURCES) src/attitude_maneuver/attitude_maneuver.c \
	src/rtb_op_codes/rtb_op_codes.c src/binding/attitude_maneuver_routines.c
GLA_SOURCES := $(INTERPRETER_SOURCES) src/gimbal_lock/gimbal_lock.c \
	src/binding/gimbal_lock_routines.c
KAL_SOURCES := $(INTERPRETER_SOURCES) src/kalcmanu_steering/kalcmanu_steering.c \
	src/rtb_op_codes/rtb_op_codes.c src/binding/kalcmanu_steering_routines.c
RUPT_SOURCES := src/agc/agc.c src/lead_ins/lead_ins.c src/waitlist/waitlist.c src/t6rupt/t6rupt.c \
	src/binding/machine.c src/binding/rupt_routines.c
SOURCES := $(sort $(KERNEL_SOURCES) $(INTERBANK_SOURCES) $(PHASE_SOURCES) $(RESTART_SOURCES) \
	$(INTERPRETER_SOURCES) $(ALARM_SOURCES) $(SP_SOURCES) $(SERVICE_SOURCES) $(DNTM_SOURCES) $(V37_SOURCES) $(STATEUP_SOURCES) \
	$(KEYRUPT_SOURCES) $(RUPT_SOURCES) $(T4RUPT_SOURCES) $(T4IMU_SOURCES) $(T4RR_SOURCES) $(T4OPT_SOURCES) \
	$(RCSMON_SOURCES) $(AGS_SOURCES) $(AOTMARK_SOURCES) $(SXTMARK_SOURCES) $(EV_SOURCES) \
	$(NOUNS_SOURCES) $(GEOMETRY_SOURCES) $(IMU_COMP_SOURCES) $(R63_SOURCES) $(AMR_SOURCES) $(GLA_SOURCES) $(KAL_SOURCES))
HEADERS := $(wildcard src/*/*.h)

.PHONY: all check kernel-cases interbank-cases phase-cases restart-cases machine-cases interpreter-cases alarm-cases \
	sp-cases service-cases downtelemetry-cases fresh-start-cases v37-cases stateup-cases keyrupt-cases rupt-cases t4rupt-cases rcs-monitor-cases ags-cases aotmark-cases sxtmark-cases extended-verbs-cases pinball-noun-tables-cases geometry-cases imu-compensation-cases r63-cases attitude-maneuver-cases gimbal-lock-cases kalcmanu-steering-cases names-check \
	coverage x86-flight x86-scenarios-quick x86-scenarios format clean
all: names-check coverage kernel-cases interbank-cases phase-cases restart-cases machine-cases interpreter-cases \
	alarm-cases sp-cases service-cases downtelemetry-cases fresh-start-cases v37-cases stateup-cases \
	keyrupt-cases rupt-cases t4rupt-cases rcs-monitor-cases ags-cases aotmark-cases sxtmark-cases \
	extended-verbs-cases pinball-noun-tables-cases geometry-cases imu-compensation-cases r63-cases \
	attitude-maneuver-cases gimbal-lock-cases kalcmanu-steering-cases
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

# ---- The machine's instructions ----
# tests/machine_cases.c: each instruction of the readable machine (src/agc/)
# and the faithful port's (src/agc_image_ops.c) on the same random states.
MC_FAITHFUL := $(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c $(PORT)/src/agc_word.c \
	$(PORT)/src/agc_multiword.c

$(BUILD)/machine-cases: $(BUILD)/cases-lm/agc/agc.o tests/machine_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(MC_FAITHFUL) $(filter %.o,$^) tests/machine_cases.c -o $@

machine-cases: $(BUILD)/machine-cases
	./$(BUILD)/machine-cases

# ---- The interpreter ----
# tests/interpreter_fuzz.c: the readable interpreter and the faithful port's
# interpreter core (src/<m>_interp_core.c, built with faithful_ before its
# names) on the same random interpretive programs, pass by pass, two seeds
# per mission. Its generator is the port's own fuzz's
# (tools/interp_core_fuzz.c).
INTERPRETER_CASES ?= 20000
ROPES := $(PORT)/build/fixed-data/Luminary099/MAIN.agc.bin $(PORT)/build/fixed-data/Comanche055/MAIN.agc.bin
IC_COMMON := $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c $(PORT)/src/agc_hardware_divide.c
IC_CFLAGS := -std=gnu99 -O2 -g -Wall -Wextra -Werror -Dexit=interp_core_fuzz_exit

$(BUILD)/faithful-%/interp_core.o: $(PORT)/src/%_interp_core.c
	@mkdir -p $(dir $@)
	$(CC) $(filter-out -Wconversion,$(PORT_CFLAGS)) $(INCLUDES) $(foreach n,interp_core interp_core_basic interp_core_mct,-D$*_$(n)=faithful_$*_$(n)) \
		-c $< -o $@

$(BUILD)/interpreter-cases-lm: $(INTERPRETER_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/faithful-lm/interp_core.o tests/interpreter_fuzz.c $(PORT)/tools/interp_core_fuzz.c
	$(CC) $(IC_CFLAGS) $(INCLUDES) $(LM) $(IC_COMMON) $(filter %.o,$^) tests/interpreter_fuzz.c -o $@
$(BUILD)/interpreter-cases-cm: $(INTERPRETER_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/faithful-cm/interp_core.o tests/interpreter_fuzz.c $(PORT)/tools/interp_core_fuzz.c
	$(CC) $(IC_CFLAGS) $(INCLUDES) $(CM) $(IC_COMMON) $(filter %.o,$^) tests/interpreter_fuzz.c -o $@

interpreter-cases: $(BUILD)/interpreter-cases-lm $(BUILD)/interpreter-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/interpreter-cases-lm $(ROPES) $(INTERPRETER_CASES) 11
	./$(BUILD)/interpreter-cases-lm $(ROPES) $(INTERPRETER_CASES) 12
	./$(BUILD)/interpreter-cases-cm $(ROPES) $(INTERPRETER_CASES) 11
	./$(BUILD)/interpreter-cases-cm $(ROPES) $(INTERPRETER_CASES) 12

# ---- Alarms and aborts ----
# The port's own tests (tests/test_lm_alarm.c, test_cm_alarm.c) on the
# readable routines: 4,096 cases per mission, every alarm site of the
# original program and every entry, against the original program's yaAGC
# digests, with the refusals they check. Then tests/alarm_cases.c: the
# readable routines and the faithful port's on the same random states.
AL_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c
AL_FAITHFUL := src/agc_image_ops.c src/agc_hardware_divide.c
AL_LM_NAMES := lm_alarm_routines lm_alarm_operand_table lm_interbank_routines lm_interbank_idle_chunk \
	lm_interbank_dap_hot_routines lm_interbank_postjump_hot_routines lm_interbank_postjump_t4_hot_routines \
	lm_interbank_site_table lm_ib_bankcall lm_ib_swcall lm_ib_swcall_m1 lm_ib_swreturn lm_ib_postjump
AL_CM_NAMES := cm_alarm_routines cm_alarm_operand_table

$(BUILD)/alarm-faithful-lm/%.o: $(PORT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AL_LM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/alarm-faithful-cm/%.o: $(PORT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AL_CM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@

$(BUILD)/alarm-oracle-lm: $(ALARM_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AL_COMMON) $(PORT)/tests/lm_alarm_cases.c \
		$(PORT)/tests/test_lm_alarm.c $(filter %.o,$^) -o $@
$(BUILD)/alarm-oracle-cm: $(ALARM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(AL_COMMON) $(PORT)/tests/cm_alarm_cases.c \
		$(PORT)/tests/test_cm_alarm.c $(filter %.o,$^) -o $@
$(BUILD)/alarm-cases-lm: $(ALARM_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(patsubst src/%.c,$(BUILD)/alarm-faithful-lm/%.o,src/lm_alarm.c $(AL_FAITHFUL)) tests/alarm_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AL_COMMON) $(filter %.o,$^) tests/alarm_cases.c -o $@
$(BUILD)/alarm-cases-cm: $(ALARM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(patsubst src/%.c,$(BUILD)/alarm-faithful-cm/%.o,src/cm_alarm.c $(AL_FAITHFUL)) tests/alarm_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(AL_COMMON) $(filter %.o,$^) tests/alarm_cases.c -o $@

alarm-cases: $(BUILD)/alarm-oracle-lm $(BUILD)/alarm-oracle-cm $(BUILD)/alarm-cases-lm \
		$(BUILD)/alarm-cases-cm
	./$(BUILD)/alarm-oracle-lm
	./$(BUILD)/alarm-oracle-cm
	./$(BUILD)/alarm-cases-lm
	./$(BUILD)/alarm-cases-cm

# ---- Single precision sine and cosine ----
# The port's own test (tests/test_sp_routines.c) on the readable SPSIN and
# SPCOS: every frozen yaAGC return state, entered at the original address.
# Then tests/sp_cases.c: readable and faithful on the same random states.
SP_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/tests/frozen_words.c
SP_ORACLE_LM := $(PORT)/src/lm_single_trig.c $(PORT)/tests/lm_single_trig_cases.c
SP_ORACLE_CM := $(PORT)/src/cm_single_return.c $(PORT)/src/cm_quictrig.c \
	$(PORT)/tests/cm_single_return_cases.c $(PORT)/tests/cm_trig_cases.c
SP_NAMES := lm_sp_routines cm_sp_routines cm_sp_hot_routines

$(BUILD)/sp-faithful-%.o: $(PORT)/src/sp_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DRK_$(shell echo $* | tr a-z A-Z)=1 \
		$(foreach n,$(SP_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@

$(BUILD)/sp-oracle-lm: $(SP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) -DRK_LM=1 $(SP_COMMON) $(SP_ORACLE_LM) \
		$(PORT)/tests/test_sp_routines.c $(filter %.o,$^) -o $@
$(BUILD)/sp-oracle-cm: $(SP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) -DRK_CM=1 $(SP_COMMON) $(SP_ORACLE_CM) \
		$(PORT)/tests/test_sp_routines.c $(filter %.o,$^) -o $@
$(BUILD)/sp-cases-lm: $(SP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/sp-faithful-lm.o tests/sp_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SP_COMMON) $(PORT)/src/lm_single_trig.c \
		$(filter %.o,$^) tests/sp_cases.c -o $@
$(BUILD)/sp-cases-cm: $(SP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sp-faithful-cm.o tests/sp_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SP_COMMON) $(PORT)/src/cm_single_return.c \
		$(PORT)/src/cm_quictrig.c $(filter %.o,$^) tests/sp_cases.c -o $@

sp-cases: $(BUILD)/sp-oracle-lm $(BUILD)/sp-oracle-cm $(BUILD)/sp-cases-lm $(BUILD)/sp-cases-cm
	./$(BUILD)/sp-oracle-lm
	./$(BUILD)/sp-oracle-cm
	./$(BUILD)/sp-cases-lm
	./$(BUILD)/sp-cases-cm

# ---- Service routines ----
# The port's own test (tests/test_service_transfer.c) on the readable
# routines: every UPFLAG, DOWNFLAG and GENTRAN site of both missions and
# every entry, against the original program's yaAGC digests, with the
# refusals it checks. It runs both missions in one program, so each
# mission's readable objects are linked into one object with only its
# table left global. Then tests/service_cases.c: the readable routines and
# the faithful port's on the same random states.
SV_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c $(PORT)/src/service_routines.c
SV_ORACLE := $(PORT)/tests/service_transfer_cases.c $(PORT)/tests/service_transfer_check.c \
	$(PORT)/tests/test_service_transfer.c
SV_NAMES := lm_service_routines cm_service_routines

$(BUILD)/service-faithful.o: $(PORT)/src/service_transfer.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SV_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/service-readable-lm.o: $(SERVICE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_service_routines $@.all $@
$(BUILD)/service-readable-cm.o: $(SERVICE_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_service_routines $@.all $@

$(BUILD)/service-oracle: $(BUILD)/service-faithful.o $(BUILD)/service-readable-lm.o \
		$(BUILD)/service-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SV_COMMON) $(SV_ORACLE) $^ -o $@
$(BUILD)/service-cases-lm: $(SERVICE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/service-faithful.o \
		tests/service_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SV_COMMON) $(filter %.o,$^) tests/service_cases.c -o $@
$(BUILD)/service-cases-cm: $(SERVICE_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/service-faithful.o \
		tests/service_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SV_COMMON) $(filter %.o,$^) tests/service_cases.c -o $@

service-cases: $(BUILD)/service-oracle $(BUILD)/service-cases-lm $(BUILD)/service-cases-cm
	./$(BUILD)/service-oracle
	./$(BUILD)/service-cases-lm
	./$(BUILD)/service-cases-cm

# ---- Down-telemetry ----
# The port's own tests (tests/test_lm_downtelemetry.c, test_cm_downtelemetry.c)
# on the readable routines: whole downlists and erasable dumps, DOWNRUPT by
# DOWNRUPT, against the original program's yaAGC digests, with every entry
# reached, every downlist word sent and the refusals they check. Then
# tests/downtelemetry_cases.c: the readable routines and the faithful
# port's, step by step on the same states.
DNTM_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c

$(BUILD)/dntm-faithful-%.o: $(PORT)/src/%_downtelemetry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_downtelemetry_routines=faithful_$*_downtelemetry_routines \
		-c $< -o $@
$(BUILD)/dntm-oracle-lm: $(DNTM_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(DNTM_COMMON) $(PORT)/tests/lm_downtelemetry_cases.c \
		$(PORT)/tests/test_lm_downtelemetry.c $^ -o $@
$(BUILD)/dntm-oracle-cm: $(DNTM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(DNTM_COMMON) $(PORT)/tests/cm_downtelemetry_cases.c \
		$(PORT)/tests/test_cm_downtelemetry.c $^ -o $@
$(BUILD)/dntm-cases-lm: $(DNTM_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/dntm-faithful-lm.o \
		tests/downtelemetry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(DNTM_COMMON) $(filter %.o,$^) tests/downtelemetry_cases.c -o $@
$(BUILD)/dntm-cases-cm: $(DNTM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/dntm-faithful-cm.o \
		tests/downtelemetry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(DNTM_COMMON) $(filter %.o,$^) tests/downtelemetry_cases.c -o $@

downtelemetry-cases: $(BUILD)/dntm-oracle-lm $(BUILD)/dntm-oracle-cm $(BUILD)/dntm-cases-lm \
		$(BUILD)/dntm-cases-cm
	./$(BUILD)/dntm-oracle-lm
	./$(BUILD)/dntm-oracle-cm
	./$(BUILD)/dntm-cases-lm
	./$(BUILD)/dntm-cases-cm

# ---- Fresh start and restart ----
# The port's own tests (tests/test_lm_fresh_start.c, test_cm_fresh_start.c)
# on the readable routines: every entry from random states, against the
# original program's yaAGC digests, with the refusals they check. Then
# tests/fresh_start_cases.c: the readable routines and the faithful port's
# on the same random states.
FS_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c
FS_LM_INTERBANK := $(addprefix $(PORT)/src/,lm_interbank_routines.c lm_interbank_idle.c lm_interbank.c)
FS_CM_INTERBANK := $(addprefix $(PORT)/src/,cm_interbank_routines.c cm_interbank_idle.c cm_interbank_complete.c)

$(BUILD)/fs-faithful-%.o: $(PORT)/src/%_fresh_start.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_fresh_start_routines=faithful_$*_fresh_start_routines \
		-D$*_fresh_entries=faithful_$*_fresh_entries -c $< -o $@
$(BUILD)/fs-oracle-lm: $(FS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FS_COMMON) $(FS_LM_INTERBANK) $(PORT)/tests/lm_fresh_start_cases.c \
		$(PORT)/tests/test_lm_fresh_start.c $^ -o $@
$(BUILD)/fs-oracle-cm: $(FS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FS_COMMON) $(FS_CM_INTERBANK) $(PORT)/tests/cm_fresh_start_cases.c \
		$(PORT)/tests/test_cm_fresh_start.c $^ -o $@
$(BUILD)/fs-cases-lm: $(FS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/fs-faithful-lm.o \
		tests/fresh_start_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(FS_COMMON) $(FS_LM_INTERBANK) $(filter %.o,$^) \
		$(PORT)/tests/lm_fresh_start_cases.c tests/fresh_start_cases.c -o $@
$(BUILD)/fs-cases-cm: $(FS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/fs-faithful-cm.o \
		tests/fresh_start_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(FS_COMMON) $(FS_CM_INTERBANK) $(filter %.o,$^) \
		$(PORT)/tests/cm_fresh_start_cases.c tests/fresh_start_cases.c -o $@

fresh-start-cases: $(BUILD)/fs-oracle-lm $(BUILD)/fs-oracle-cm $(BUILD)/fs-cases-lm $(BUILD)/fs-cases-cm
	./$(BUILD)/fs-oracle-lm
	./$(BUILD)/fs-oracle-cm
	./$(BUILD)/fs-cases-lm
	./$(BUILD)/fs-cases-cm

# ---- Verb 37 and P00 ----
# The port's own tests (tests/test_lm_v37.c, test_cm_v37.c) on the readable
# routines, with the readable fresh start beside them: every entry from
# random states, against the original program's yaAGC digests. Then
# tests/v37_cases.c: the readable routines and the faithful port's on the
# same random states, and V37's tables against the listing's words.
$(BUILD)/v37-faithful-%.o: $(PORT)/src/%_v37.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_v37_routines=faithful_$*_v37_routines \
		-D$*_v37_entries=faithful_$*_v37_entries -c $< -o $@
$(BUILD)/v37-oracle-lm: $(V37_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FS_COMMON) $(FS_LM_INTERBANK) $(PORT)/tests/lm_v37_cases.c \
		$(PORT)/tests/test_lm_v37.c $^ -o $@
$(BUILD)/v37-oracle-cm: $(V37_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FS_COMMON) $(FS_CM_INTERBANK) $(PORT)/tests/cm_v37_cases.c \
		$(PORT)/tests/test_cm_v37.c $^ -o $@
$(BUILD)/v37-cases-lm: $(V37_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/v37-faithful-lm.o \
		tests/v37_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(FS_COMMON) $(FS_LM_INTERBANK) $(filter %.o,$^) \
		$(PORT)/tests/lm_v37_cases.c tests/v37_cases.c -o $@
$(BUILD)/v37-cases-cm: $(V37_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/v37-faithful-cm.o \
		tests/v37_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(FS_COMMON) $(FS_CM_INTERBANK) $(filter %.o,$^) \
		$(PORT)/tests/cm_v37_cases.c tests/v37_cases.c -o $@

v37-cases: $(BUILD)/v37-oracle-lm $(BUILD)/v37-oracle-cm $(BUILD)/v37-cases-lm $(BUILD)/v37-cases-cm
	./$(BUILD)/v37-oracle-lm
	./$(BUILD)/v37-oracle-cm
	./$(BUILD)/v37-cases-lm
	./$(BUILD)/v37-cases-cm

# ---- STATEUP: interpretive code ----
# The port's own tests (tests/test_lm_stateup.c, test_cm_stateup.c) on the
# readable interpreter running the listing's words (src/fresh_start/
# stateup.c): every order pair from random states, against the original
# program's yaAGC digests. Then tests/stateup_cases.c: the words against
# the rope's, and the readable pairs and the faithful port's on the same
# random states.
STATEUP_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c $(PORT)/src/rope_image_host.c
LM_STATEUP_FAITHFUL := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c
CM_STATEUP_FAITHFUL := $(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c
$(BUILD)/stateup-faithful-%.o: $(PORT)/src/%_stateup.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_stateup_interpretive=faithful_$*_stateup_interpretive \
		-D$*_stateup_entries=faithful_$*_stateup_entries -c $< -o $@
# The port's test with its main() renamed: tests/with_ropes.c loads the
# ropes the readable interpreter reads, then runs it.
$(BUILD)/stateup-test-%.o: $(PORT)/tests/test_%_stateup.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/stateup-oracle-lm: $(STATEUP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/stateup-test-lm.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(STATEUP_COMMON) $(PORT)/tests/lm_stateup_cases.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/stateup-oracle-cm: $(STATEUP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/stateup-test-cm.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(STATEUP_COMMON) $(PORT)/tests/cm_stateup_cases.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/stateup-cases-lm: $(STATEUP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/stateup-faithful-lm.o tests/stateup_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(STATEUP_COMMON) $(LM_STATEUP_FAITHFUL) $(filter %.o,$^) \
		$(PORT)/tests/lm_stateup_cases.c tests/stateup_cases.c -o $@
$(BUILD)/stateup-cases-cm: $(STATEUP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/stateup-faithful-cm.o tests/stateup_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(STATEUP_COMMON) $(CM_STATEUP_FAITHFUL) $(filter %.o,$^) \
		$(PORT)/tests/cm_stateup_cases.c tests/stateup_cases.c -o $@

stateup-cases: $(BUILD)/stateup-oracle-lm $(BUILD)/stateup-oracle-cm $(BUILD)/stateup-cases-lm \
		$(BUILD)/stateup-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/stateup-oracle-lm $(ROPES)
	./$(BUILD)/stateup-oracle-cm $(ROPES)
	./$(BUILD)/stateup-cases-lm $(ROPES)
	./$(BUILD)/stateup-cases-cm $(ROPES)

# ---- The keyboard and uplink interrupts ----
# The port's own test (tests/test_keyrupt_uprupt.c) on the readable
# routines: 1,536 states per mission over the six entries, against the
# original program's yaAGC digests, with every uplink path reached and the
# refusals it checks. It runs both missions in one program, so each
# mission's readable objects are linked into one object with only its
# table left global. Then tests/keyrupt_cases.c: the readable routines and
# the faithful port's on the same random states.
KU_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c
KU_NAMES := lm_keyrupt_routines cm_keyrupt_routines

$(BUILD)/keyrupt-faithful.o: $(PORT)/src/keyrupt_uprupt.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(KU_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/keyrupt-readable-lm.o: $(KEYRUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_keyrupt_routines $@.all $@
$(BUILD)/keyrupt-readable-cm.o: $(KEYRUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_keyrupt_routines $@.all $@

$(BUILD)/keyrupt-oracle: $(BUILD)/keyrupt-readable-lm.o $(BUILD)/keyrupt-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KU_COMMON) $(PORT)/tests/keyrupt_uprupt_cases.c \
		$(PORT)/tests/test_keyrupt_uprupt.c $^ -o $@
$(BUILD)/keyrupt-cases-lm: $(KEYRUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/keyrupt-faithful.o \
		tests/keyrupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) tests/keyrupt_cases.c -o $@
$(BUILD)/keyrupt-cases-cm: $(KEYRUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/keyrupt-faithful.o \
		tests/keyrupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(filter %.o,$^) tests/keyrupt_cases.c -o $@

keyrupt-cases: $(BUILD)/keyrupt-oracle $(BUILD)/keyrupt-cases-lm $(BUILD)/keyrupt-cases-cm
	./$(BUILD)/keyrupt-oracle
	./$(BUILD)/keyrupt-cases-lm
	./$(BUILD)/keyrupt-cases-cm

# ---- The interrupt lead-ins, RESUME and the LM's T6 interrupt ----
# The port's own tests on the readable routines: tests/test_rupt_lead_ins.c
# (both missions' lead-ins and the CM's RESUME entries, 128 states per
# entry) and tests/test_lm_t6rupt.c (the LM's T6RUPT lead-in, T6 routines
# and RESUME entries, every path), against the original program's yaAGC
# digests, with the refusals they check. Each mission's readable objects
# are linked into one object with only its tables left global. Then
# tests/rupt_cases.c: the readable routines and the faithful port's on the
# same random states.
RUPT_COMMON := $(KU_COMMON)
RUPT_LM_NAMES := lm_lead_in_routines lm_lead_in_hot_routines lm_t6rupt_routines \
	lm_t6rupt_hot_routines lm_t6rupt_jets_hot_routines lm_dntm_rupt_routines
RUPT_CM_NAMES := cm_lead_in_routines cm_dntm_rupt_routines
RUPT_NAMES := $(RUPT_LM_NAMES) $(RUPT_CM_NAMES)

RUPT_FAITHFUL := rupt_lead_ins lm_t6rupt lm_dntm_rupt cm_dntm_rupt
$(BUILD)/rupt-faithful.o: $(RUPT_FAITHFUL:%=$(PORT)/src/%.c)
	@mkdir -p $(dir $@)
	$(foreach f,$(RUPT_FAITHFUL),$(CC) $(PORT_CFLAGS) $(INCLUDES) \
		$(foreach n,$(RUPT_NAMES),-D$(n)=faithful_$(n)) -c $(PORT)/src/$(f).c -o $@.$(f) &&) true
	ld -r $(RUPT_FAITHFUL:%=$@.%) -o $@
$(BUILD)/rupt-readable-lm.o: $(RUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(RUPT_LM_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/rupt-readable-cm.o: $(RUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(RUPT_CM_NAMES),--keep-global-symbol=$(n)) $@.all $@

$(BUILD)/rupt-lead-ins-oracle: $(BUILD)/rupt-readable-lm.o $(BUILD)/rupt-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RUPT_COMMON) $(PORT)/tests/rupt_lead_ins_cases.c \
		$(PORT)/tests/test_rupt_lead_ins.c $^ -o $@
$(BUILD)/lm-t6rupt-oracle: $(BUILD)/rupt-readable-lm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RUPT_COMMON) $(PORT)/tests/lm_t6rupt_cases.c \
		$(PORT)/tests/test_lm_t6rupt.c $^ -o $@
$(BUILD)/rupt-cases-lm: $(RUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/rupt-faithful.o \
		tests/rupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(RUPT_COMMON) $(filter %.o,$^) tests/rupt_cases.c -o $@
$(BUILD)/rupt-cases-cm: $(RUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/rupt-faithful.o \
		tests/rupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(RUPT_COMMON) $(filter %.o,$^) tests/rupt_cases.c -o $@

rupt-cases: $(BUILD)/rupt-lead-ins-oracle $(BUILD)/lm-t6rupt-oracle $(BUILD)/rupt-cases-lm \
		$(BUILD)/rupt-cases-cm
	./$(BUILD)/rupt-lead-ins-oracle
	./$(BUILD)/lm-t6rupt-oracle
	./$(BUILD)/rupt-cases-lm
	./$(BUILD)/rupt-cases-cm

# ---- T4RUPT: the DSKY's relays and the once-a-second checks ----
# The port's own test (tests/test_t4rupt_core.c) on the readable core:
# 2,048 states at T4RUPT and 64 at each return entry per mission, every
# continuation and display path, against the original program's yaAGC
# digests, with the refusals it checks. Each mission's readable objects
# are linked into one object with only its tables left global. Then
# tests/t4rupt_cases.c: the readable routines and the faithful port's on
# the same random states.
T4RUPT_LM_NAMES := lm_t4rupt_core_routines lm_t4rupt_core_hot_routines
T4RUPT_NAMES := $(T4RUPT_LM_NAMES) cm_t4rupt_core_routines

$(BUILD)/t4rupt-faithful.o: $(PORT)/src/t4rupt_core.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(T4RUPT_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/t4rupt-readable-lm.o: $(T4RUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(T4RUPT_LM_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/t4rupt-readable-cm.o: $(T4RUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_t4rupt_core_routines $@.all $@

$(BUILD)/t4rupt-oracle: $(BUILD)/t4rupt-readable-lm.o $(BUILD)/t4rupt-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KU_COMMON) $(PORT)/tests/t4rupt_core_cases.c \
		$(PORT)/tests/test_t4rupt_core.c $^ -o $@
$(BUILD)/t4rupt-cases-lm: $(T4RUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/t4rupt-faithful.o \
		tests/t4rupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_cases.c -o $@
$(BUILD)/t4rupt-cases-cm: $(T4RUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/t4rupt-faithful.o \
		tests/t4rupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_cases.c -o $@

# Part 2, the IMU monitor: the port's own tests (tests/test_lm_t4rupt_imu.c
# and test_cm_t4rupt_imu.c) on the readable routines, against the original
# program's yaAGC digests, with the refusals they check; then
# tests/t4rupt_imu_cases.c, the readable routines and the faithful port's
# on the same random states.
$(BUILD)/t4imu-faithful-%.o: $(PORT)/src/%_t4rupt_imu.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_t4rupt_imu_routines=faithful_$*_t4rupt_imu_routines -c $< -o $@
$(BUILD)/t4imu-readable-lm.o: $(T4IMU_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_t4rupt_imu_routines $@.all $@
$(BUILD)/t4imu-readable-cm.o: $(T4IMU_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_t4rupt_imu_routines $@.all $@
$(BUILD)/t4imu-oracle-%: $(BUILD)/t4imu-readable-%.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KU_COMMON) $(PORT)/tests/$*_t4rupt_imu_cases.c \
		$(PORT)/tests/test_$*_t4rupt_imu.c $^ -o $@
$(BUILD)/t4imu-cases-lm: $(T4IMU_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/t4imu-faithful-lm.o \
		tests/t4rupt_imu_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_imu_cases.c -o $@
$(BUILD)/t4imu-cases-cm: $(T4IMU_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/t4imu-faithful-cm.o \
		tests/t4rupt_imu_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_imu_cases.c -o $@

# Part 3, the LM's radar checks and GPMATRIX: the port's test
# (tests/test_lm_t4rupt_rr.c) on the readable routines, then
# tests/t4rupt_rr_cases.c side by side.
$(BUILD)/t4rr-faithful.o: $(PORT)/src/lm_t4rupt_rr.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_t4rupt_rr_routines=faithful_lm_t4rupt_rr_routines -c $< -o $@
$(BUILD)/t4rr-readable.o: $(T4RR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_t4rupt_rr_routines $@.all $@
$(BUILD)/t4rr-oracle: $(BUILD)/t4rr-readable.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KU_COMMON) $(PORT)/tests/lm_t4rupt_rr_cases.c \
		$(PORT)/tests/test_lm_t4rupt_rr.c $^ -o $@
$(BUILD)/t4rr-cases: $(T4RR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/t4rr-faithful.o \
		tests/t4rupt_rr_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_rr_cases.c -o $@

# Part 4, the CM's optics: the port's test (tests/test_cm_t4rupt_optics.c,
# its frozen cases run to their stops through the program of
# tests/cm_t4rupt_optics_target.c) on the readable routines, then
# tests/t4rupt_optics_cases.c side by side.
T4OPT_NAMES := cm_t4rupt_optics_basic cm_t4rupt_optics_interpretive
T4OPT_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c $(KU_COMMON) $(PORT)/tests/interp_case.c

$(BUILD)/t4opt-faithful.o: $(PORT)/src/cm_t4rupt_optics.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(T4OPT_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/t4opt-readable.o: $(T4OPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(T4OPT_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/t4opt-oracle: $(BUILD)/t4opt-readable.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(T4OPT_COMMON) $(PORT)/tests/cm_t4rupt_optics_target.c \
		$(PORT)/tests/test_cm_t4rupt_optics.c $^ -o $@
$(BUILD)/t4opt-cases: $(T4OPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/t4opt-faithful.o \
		tests/t4rupt_optics_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_optics_cases.c -o $@

t4rupt-cases: $(BUILD)/t4rupt-oracle $(BUILD)/t4rupt-cases-lm $(BUILD)/t4rupt-cases-cm \
		$(BUILD)/t4imu-oracle-lm $(BUILD)/t4imu-oracle-cm $(BUILD)/t4imu-cases-lm $(BUILD)/t4imu-cases-cm \
		$(BUILD)/t4rr-oracle $(BUILD)/t4rr-cases $(BUILD)/t4opt-oracle $(BUILD)/t4opt-cases
	./$(BUILD)/t4rupt-oracle
	./$(BUILD)/t4rupt-cases-lm
	./$(BUILD)/t4rupt-cases-cm
	./$(BUILD)/t4imu-oracle-lm
	./$(BUILD)/t4imu-oracle-cm
	./$(BUILD)/t4imu-cases-lm
	./$(BUILD)/t4imu-cases-cm
	./$(BUILD)/t4rr-oracle
	./$(BUILD)/t4rr-cases
	./$(BUILD)/t4opt-oracle
	./$(BUILD)/t4opt-cases

# RCS_FAILURE_MONITOR, the LM's: the port's test (tests/test_lm_rcsmon.c,
# its frozen cases run to their stops through the program of
# tests/lm_rcsmon_target.c) on the readable routines, then
# tests/rcs_failure_monitor_cases.c side by side.
RCSMON_NAMES := lm_rcsmon_basic lm_rcsmon_interpretive
RCSMON_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/tests/interp_case.c

$(BUILD)/rcsmon-faithful.o: $(PORT)/src/lm_rcsmon.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(RCSMON_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/rcsmon-readable.o: $(RCSMON_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(RCSMON_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/rcsmon-oracle: $(BUILD)/rcsmon-readable.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RCSMON_COMMON) $(PORT)/tests/lm_rcsmon_target.c \
		$(PORT)/tests/test_lm_rcsmon.c $^ -o $@
$(BUILD)/rcsmon-cases: $(RCSMON_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/rcsmon-faithful.o \
		tests/rcs_failure_monitor_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) \
		tests/rcs_failure_monitor_cases.c -o $@

rcs-monitor-cases: $(BUILD)/rcsmon-oracle $(BUILD)/rcsmon-cases
	./$(BUILD)/rcsmon-oracle
	./$(BUILD)/rcsmon-cases

# AGS_INITIALIZATION, the LM's R47: tests/ags_initialization_oracle.c runs
# the port's frozen cases (c64-port/tests/lm_ags_init_vectors.h, yaAGC
# digests) and refusals on the readable routines; then
# tests/ags_initialization_cases.c runs readable and faithful side by side,
# as the host runs them and as the flight program does (stopping at the
# interpreter, -DAGC_FLIGHT_INTERPRETER).
AGS_NAMES := lm_ags_init_routines lm_ags_init_interpretive ags_routine_keys
AGS_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rope_image_host.c $(PORT)/tests/lm_ags_init_cases.c
AGS_FLIGHT_OBJECTS := $(AGS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
AGS_FLIGHT_OBJECTS := $(filter-out %/ags_initialization_routines.o,$(AGS_FLIGHT_OBJECTS)) \
	$(BUILD)/ags-flight/ags_initialization_routines.o

$(BUILD)/ags-flight/ags_initialization_routines.o: src/binding/ags_initialization_routines.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -DRK_SEAMS=1 -DAGC_FLIGHT_INTERPRETER -c $< -o $@
$(BUILD)/ags-faithful.o: $(PORT)/src/lm_ags_init.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY \
		$(foreach n,$(AGS_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ags-faithful-flight.o: $(PORT)/src/lm_ags_init.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY -DAGC_FLIGHT_INTERPRETER \
		$(foreach n,$(AGS_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ags-oracle: $(AGS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) tests/ags_initialization_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AGS_COMMON) $(filter %.o,$^) \
		tests/ags_initialization_oracle.c -o $@
$(BUILD)/ags-cases: $(AGS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/ags-faithful.o \
		tests/ags_initialization_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AGS_COMMON) $(filter %.o,$^) \
		tests/ags_initialization_cases.c -o $@
$(BUILD)/ags-cases-flight: $(AGS_FLIGHT_OBJECTS) $(BUILD)/ags-faithful-flight.o \
		tests/ags_initialization_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) -DAGC_FLIGHT_INTERPRETER $(AGS_COMMON) $(filter %.o,$^) \
		tests/ags_initialization_cases.c -o $@

ags-cases: $(BUILD)/ags-oracle $(BUILD)/ags-cases $(BUILD)/ags-cases-flight
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ags-oracle $(ROPES)
	./$(BUILD)/ags-cases $(ROPES)
	./$(BUILD)/ags-cases-flight $(ROPES)

# AOTMARK, the LM's sighting marks: the port's test (tests/test_lm_aotmark.c,
# its frozen cases run to their stops through the program of
# tests/lm_aotmark_target.c, against the original program's yaAGC digests)
# on the readable routines, with tests/with_ropes.c loading the ropes the
# readable interpreter reads; then tests/aotmark_cases.c side by side.
AOTMARK_NAMES := lm_aotmark_basic lm_aotmark_interpretive
AOTMARK_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rope_image_host.c $(PORT)/tests/interp_case.c

$(BUILD)/aotmark-faithful.o: $(PORT)/src/lm_aotmark.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AOTMARK_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/aotmark-test.o: $(PORT)/tests/test_lm_aotmark.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/aotmark-oracle: $(AOTMARK_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/aotmark-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(AOTMARK_COMMON) $(PORT)/tests/lm_aotmark_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/aotmark-cases: $(AOTMARK_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/aotmark-faithful.o \
		tests/aotmark_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AOTMARK_COMMON) $(PORT)/src/rtb_angles.c \
		$(filter %.o,$^) tests/aotmark_cases.c -o $@

aotmark-cases: $(BUILD)/aotmark-oracle $(BUILD)/aotmark-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/aotmark-oracle $(ROPES)
	./$(BUILD)/aotmark-cases $(ROPES)

# SXTMARK, the CM's sextant marks: the port's test (tests/test_cm_sxtmark.c,
# its frozen cases run to their stops through the program of
# tests/cm_sxtmark_target.c, against the original program's yaAGC digests)
# on the readable routines; then tests/sxtmark_cases.c side by side.
SXTMARK_NAMES := cm_sxtmark_basic cm_sxtmark_interpretive

$(BUILD)/sxtmark-faithful.o: $(PORT)/src/cm_sxtmark.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SXTMARK_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/sxtmark-readable.o: $(SXTMARK_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(SXTMARK_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/sxtmark-oracle: $(BUILD)/sxtmark-readable.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(T4OPT_COMMON) $(PORT)/tests/cm_sxtmark_target.c \
		$(PORT)/tests/test_cm_sxtmark.c $^ -o $@
$(BUILD)/sxtmark-cases: $(SXTMARK_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sxtmark-faithful.o \
		tests/sxtmark_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(PORT)/tests/interp_case.c \
		$(filter %.o,$^) tests/sxtmark_cases.c -o $@

sxtmark-cases: $(BUILD)/sxtmark-oracle $(BUILD)/sxtmark-cases
	./$(BUILD)/sxtmark-oracle
	./$(BUILD)/sxtmark-cases

# EXTENDED_VERBS, both missions, part by part. Each readable part takes
# the place of the faithful port's module for the part
# (c64-port/src/lanec_ev_<part>.c) under its name. The tests link both
# missions' readable objects, each built into one object with the
# module's name renamed readable_<m>_..., and tests/extended_verbs_missions.c
# puts them back under the module's name. First the port's frozen oracle
# rows (the original program's results in Virtual AGC) on the readable
# routines, then tests/extended_verbs_<part>_cases.c side by side with the
# faithful module (built with its dispatch renamed faithful_...; from the
# short verbs on, it also gives the port's suite its list of entries).
# (The readable parts link the readable interpreter, part 8's, which reads
# the rope: rope_image_host.c. Only parts 8 and 9 load the ropes.)
LANEC_COMMON := $(PORT)/src/agc_word.c $(PORT)/src/lanec.c $(PORT)/tests/lanec_case.c \
	$(PORT)/src/rope_image_host.c
EV_PARTS := front short align radar dap time track lmi
EV_NAMES := $(EV_PARTS:%=lanec_ev_%_dispatch)
# Part 8's tables, which its test checks against the rope: the LM's only
# (ATTACHIT_CODE and TCPIN_CODE are both missions' names, so the CM's stay
# local here; part 9's test links the CM's own).
EV_TABLES := RRLOSDSP_CODE TRMTRAK1_CODE ATTACHIT_CODE ATTACHIT_CODE_2 TCPIN_CODE

$(BUILD)/ev-%-faithful.o: $(PORT)/src/lanec_ev_%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlanec_ev_$*_dispatch=faithful_lanec_ev_$*_dispatch -c $< -o $@
$(BUILD)/ev-readable-lm.o: $(EV_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
$(BUILD)/ev-readable-cm.o: $(EV_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
$(BUILD)/ev-readable-%.o:
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(EV_NAMES) $(if $(filter lm,$*),$(EV_TABLES)),--keep-global-symbol=$(n)) \
		$@.all $@.kept
	objcopy $(foreach n,$(EV_NAMES),--redefine-sym $(n)=readable_$*_$(n:lanec_%=%)) $@.kept $@
EV_READABLE := $(BUILD)/ev-readable-lm.o $(BUILD)/ev-readable-cm.o tests/extended_verbs_missions.c
$(BUILD)/ev-front-oracle: $(EV_READABLE) tests/extended_verbs_front_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_front_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-front-cases: $(EV_READABLE) $(BUILD)/ev-front-faithful.o tests/extended_verbs_front_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_front_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-short-oracle: $(EV_READABLE) $(BUILD)/ev-short-faithful.o tests/extended_verbs_short_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_short_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-short-cases: $(EV_READABLE) $(BUILD)/ev-short-faithful.o tests/extended_verbs_short_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_short_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-align-oracle: $(EV_READABLE) $(BUILD)/ev-align-faithful.o tests/extended_verbs_align_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_align_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-align-cases: $(EV_READABLE) $(BUILD)/ev-align-faithful.o tests/extended_verbs_align_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_align_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-radar-oracle: $(EV_READABLE) $(BUILD)/ev-radar-faithful.o tests/extended_verbs_radar_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_radar_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-radar-cases: $(EV_READABLE) $(BUILD)/ev-radar-faithful.o tests/extended_verbs_radar_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_radar_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-dap-oracle: $(EV_READABLE) $(BUILD)/ev-dap-faithful.o tests/extended_verbs_dap_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_dap_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-dap-cases: $(EV_READABLE) $(BUILD)/ev-dap-faithful.o tests/extended_verbs_dap_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_dap_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-time-oracle: $(EV_READABLE) $(BUILD)/ev-time-faithful.o tests/extended_verbs_time_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_time_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-time-cases: $(EV_READABLE) $(BUILD)/ev-time-faithful.o tests/extended_verbs_time_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_time_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-track-oracle: $(EV_READABLE) $(BUILD)/ev-track-faithful.o tests/extended_verbs_track_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_track_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-track-cases: $(EV_READABLE) $(BUILD)/ev-track-faithful.o tests/extended_verbs_track_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_track_cases.c \
		$(filter %.o %.c,$^) -o $@

# Part 8's words run on the readable interpreter, which reads the
# program's constants from the rope: its tests load the ropes.
EV_INTERP_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c $(PORT)/src/agc_multiword.c
$(BUILD)/ev-lmi-oracle: $(EV_READABLE) $(BUILD)/ev-lmi-faithful.o tests/extended_verbs_lmi_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(EV_INTERP_COMMON) \
		$(PORT)/tests/lanec_ev_lmi_cases.c $(filter %.o %.c,$^) -o $@
$(BUILD)/ev-lmi-cases: $(EV_READABLE) $(BUILD)/ev-lmi-faithful.o tests/extended_verbs_lmi_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(EV_INTERP_COMMON) \
		$(PORT)/tests/lanec_ev_lmi_cases.c $(filter %.o %.c,$^) -o $@

# Part 9, the CM's interpretive words, and with them the CM's file whole:
# the readable file's two tables (cm_extended_verbs_file_basic and
# _interpretive, the names of the port's compiled copy of the file) and
# the parts, as one program (tests/extended_verbs_file_program.c). First
# the port's frozen cases for the compiled file (the original program's
# results in Virtual AGC) on it, then tests/extended_verbs_file_cases.c
# side by side with the compiled file.
EV_FILE_NAMES := cm_extended_verbs_file_basic cm_extended_verbs_file_interpretive cm_interpreter_basic \
	$(filter-out lanec_ev_radar_dispatch lanec_ev_lmi_dispatch,$(EV_NAMES)) \
	TRACKTRM_CODE SETVHFLG_CODE RESETVHF_CODE ATTACHIT_CODE TACHEXIT_CODE TCPIN_CODE LMTOCM_CODE
EV_FILE_COMMON := $(LANEC_COMMON) $(EV_INTERP_COMMON) $(PORT)/src/cm_interp_orders.c \
	$(PORT)/src/cm_interp_steps.c $(PORT)/src/agc_transfer.c $(PORT)/tests/interp_case.c \
	-DCM_ROPE='"$(word 2,$(ROPES))"' tests/extended_verbs_file_program.c
$(BUILD)/ev-file-cm.o: $(EV_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(EV_FILE_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/ev-file-faithful.o: $(PORT)/src/cm_extended_verbs_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) \
		-Dcm_extended_verbs_file_basic=faithful_cm_extended_verbs_file_basic \
		-Dcm_extended_verbs_file_interpretive=faithful_cm_extended_verbs_file_interpretive \
		-c $< -o $@
$(BUILD)/ev-file-oracle: $(BUILD)/ev-file-cm.o tests/extended_verbs_file_program.c \
		tests/extended_verbs_file_target.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Itests $(EV_FILE_COMMON) $(BUILD)/ev-file-cm.o \
		tests/extended_verbs_file_target.c $(PORT)/tests/test_cm_extended_verbs_file.c -o $@
$(BUILD)/ev-file-cases: $(BUILD)/ev-file-cm.o $(BUILD)/ev-file-faithful.o \
		tests/extended_verbs_file_program.c tests/extended_verbs_file_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Itests $(EV_FILE_COMMON) $(BUILD)/ev-file-cm.o \
		$(BUILD)/ev-file-faithful.o tests/extended_verbs_file_cases.c -o $@

extended-verbs-cases: $(BUILD)/ev-front-oracle $(BUILD)/ev-front-cases $(BUILD)/ev-short-oracle \
		$(BUILD)/ev-short-cases $(BUILD)/ev-align-oracle $(BUILD)/ev-align-cases \
		$(BUILD)/ev-radar-oracle $(BUILD)/ev-radar-cases $(BUILD)/ev-dap-oracle \
		$(BUILD)/ev-dap-cases $(BUILD)/ev-time-oracle $(BUILD)/ev-time-cases \
		$(BUILD)/ev-track-oracle $(BUILD)/ev-track-cases $(BUILD)/ev-lmi-oracle \
		$(BUILD)/ev-lmi-cases $(BUILD)/ev-file-oracle $(BUILD)/ev-file-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ev-front-oracle
	./$(BUILD)/ev-front-cases
	./$(BUILD)/ev-short-oracle
	./$(BUILD)/ev-short-cases
	./$(BUILD)/ev-align-oracle
	./$(BUILD)/ev-align-cases
	./$(BUILD)/ev-radar-oracle
	./$(BUILD)/ev-radar-cases
	./$(BUILD)/ev-dap-oracle
	./$(BUILD)/ev-dap-cases
	./$(BUILD)/ev-time-oracle
	./$(BUILD)/ev-time-cases
	./$(BUILD)/ev-track-oracle
	./$(BUILD)/ev-track-cases
	./$(BUILD)/ev-lmi-oracle $(ROPES)
	./$(BUILD)/ev-lmi-cases $(ROPES)
	./$(BUILD)/ev-file-oracle
	./$(BUILD)/ev-file-cases

# PINBALL_NOUN_TABLES, both missions: the readers in the faithful port's
# place, as apollo_noun_reader (c64-port/include/noun_reader.h). Each
# mission's objects are built into one object with that name, and the
# tables' word function, renamed readable_<m>_...; tests/
# noun_reader_missions.c puts the first back, by mission. First the port's
# own test (c64-port/tests/test_noun_reader.c: every supported case runs,
# the unsupported ones are refused) on them, then the original program's
# frozen results (tests/pinball_noun_tables_oracle.c), then
# tests/pinball_noun_tables_cases.c: the tables against the ropes, and the
# readers side by side with the faithful one.
NOUNS_NAMES := apollo_noun_reader noun_table_word
$(BUILD)/nouns-readable-lm.o: $(NOUNS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
$(BUILD)/nouns-readable-cm.o: $(NOUNS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
$(BUILD)/nouns-readable-%.o:
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(NOUNS_NAMES),--keep-global-symbol=$(n)) $@.all $@.kept
	objcopy --redefine-sym apollo_noun_reader=readable_$*_noun_reader \
		--redefine-sym noun_table_word=readable_$*_noun_table_word $@.kept $@
NOUNS_READABLE := $(BUILD)/nouns-readable-lm.o $(BUILD)/nouns-readable-cm.o tests/noun_reader_missions.c
$(BUILD)/nouns-faithful.o: $(PORT)/src/noun_reader.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dapollo_noun_reader=faithful_apollo_noun_reader -c $< -o $@
$(BUILD)/nouns-port-test: $(NOUNS_READABLE)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(PORT)/tests/noun_reader_cases.c $(PORT)/tests/test_noun_reader.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/nouns-oracle: $(NOUNS_READABLE) tests/pinball_noun_tables_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(PORT)/tests/noun_reader_cases.c $(filter %.o %.c,$^) -o $@
$(BUILD)/nouns-cases: $(NOUNS_READABLE) $(BUILD)/nouns-faithful.o tests/pinball_noun_tables_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(PORT)/src/agc_word.c $(PORT)/src/fixed_data.c \
		$(PORT)/src/rope_image_host.c $(PORT)/tests/noun_reader_cases.c $(filter %.o %.c,$^) -o $@

pinball-noun-tables-cases: $(BUILD)/nouns-port-test $(BUILD)/nouns-oracle $(BUILD)/nouns-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/nouns-port-test
	./$(BUILD)/nouns-oracle
	./$(BUILD)/nouns-cases $(ROPES)

# ---- LEM_GEOMETRY, CSM_GEOMETRY ----
# The port's test (tests/test_geometry.c, its frozen cases run to their
# stops through the program of tests/geometry_target.c, against the
# original program's yaAGC digests) with each mission's readable routines
# and the other mission's faithful ones, tests/with_ropes.c loading the
# ropes the readable interpreter reads; then tests/geometry_cases.c side by
# side, for each mission.
GEOMETRY_NAMES_LM := lm_geometry_basic lm_geometry_interpretive
GEOMETRY_NAMES_CM := cm_geometry_basic cm_geometry_interpretive
GEOMETRY_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rtb_angles.c $(PORT)/src/rtb_agreement.c $(PORT)/src/geometry_rtb.c \
	$(PORT)/src/cm_integration_moves.c $(PORT)/src/rope_image_host.c $(PORT)/tests/interp_case.c

$(BUILD)/geometry-faithful-lm.o: $(PORT)/src/lm_geometry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(GEOMETRY_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/geometry-faithful-cm.o: $(PORT)/src/cm_geometry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(GEOMETRY_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/geometry-test.o: $(PORT)/tests/test_geometry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/geometry-oracle-lm: $(GEOMETRY_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/geometry-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(GEOMETRY_COMMON) $(PORT)/src/cm_geometry.c \
		$(PORT)/tests/geometry_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/geometry-oracle-cm: $(GEOMETRY_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/geometry-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(GEOMETRY_COMMON) $(PORT)/src/lm_geometry.c \
		$(PORT)/tests/geometry_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/geometry-cases-lm: $(GEOMETRY_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/geometry-faithful-lm.o tests/geometry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(GEOMETRY_COMMON) $(filter %.o,$^) \
		tests/geometry_cases.c -o $@
$(BUILD)/geometry-cases-cm: $(GEOMETRY_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/geometry-faithful-cm.o tests/geometry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(GEOMETRY_COMMON) $(filter %.o,$^) \
		tests/geometry_cases.c -o $@

geometry-cases: $(BUILD)/geometry-oracle-lm $(BUILD)/geometry-oracle-cm $(BUILD)/geometry-cases-lm \
		$(BUILD)/geometry-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/geometry-oracle-lm $(ROPES)
	./$(BUILD)/geometry-oracle-cm $(ROPES)
	./$(BUILD)/geometry-cases-lm $(ROPES)
	./$(BUILD)/geometry-cases-cm $(ROPES)

# ---- The x86_64 flight program ----
# tools/build_x86.py builds the port's x86 program with these objects in
# place of the faithful kernel (build/x86-flight/).
# R47's routines stop where they reach the interpreter in the flight
# program, as the port's flight build of its file does.
$(BUILD)/x86-lm/binding/ags_initialization_routines.o: FLIGHT_DEFINES := -DAGC_FLIGHT_INTERPRETER
$(BUILD)/x86-lm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) $(FLIGHT_DEFINES) -c $< -o $@
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

# IMU_COMPENSATION_PACKAGE, both missions: the port's test
# (tests/test_imu_compensation.c, its frozen cases run to their stops,
# composed with the faithful INTER-BANK_COMMUNICATION and Executive,
# against the original program's yaAGC digests) on the readable routines,
# both missions' in one program: each mission's readable objects built
# into one object that keeps only its table's name global. Then
# tests/imu_compensation_cases.c side by side, mission by mission.
ICP_MK := $(PORT)/tools/mk/imu-compensation.mk
ICP_PORT := $(addprefix $(PORT)/,$(filter-out src/imu_compensation.c,$(shell sed -n 's/^ICP_SRC = //p' $(ICP_MK))))
ICP_PORT_FLAGS := -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections

$(BUILD)/imu-compensation-readable-lm.o: $(IMU_COMP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_imu_compensation_routines $@.all $@
$(BUILD)/imu-compensation-readable-cm.o: $(IMU_COMP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_imu_compensation_routines $@.all $@
$(BUILD)/imu-compensation-oracle: $(BUILD)/imu-compensation-readable-lm.o \
		$(BUILD)/imu-compensation-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(ICP_PORT_FLAGS) $(ICP_PORT) $(PORT)/tests/test_imu_compensation.c \
		$^ -o $@
$(BUILD)/imu-compensation-faithful.o: $(PORT)/src/imu_compensation.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_imu_compensation_routines=faithful_lm_imu_compensation_routines \
		-Dcm_imu_compensation_routines=faithful_cm_imu_compensation_routines -c $< -o $@
$(BUILD)/imu-compensation-cases-lm: $(IMU_COMP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/imu-compensation-faithful.o tests/imu_compensation_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c \
		$(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
		$(filter %.o,$^) tests/imu_compensation_cases.c -o $@
$(BUILD)/imu-compensation-cases-cm: $(IMU_COMP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/imu-compensation-faithful.o tests/imu_compensation_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c \
		$(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
		$(filter %.o,$^) tests/imu_compensation_cases.c -o $@

imu-compensation-cases: $(BUILD)/imu-compensation-oracle $(BUILD)/imu-compensation-cases-lm \
		$(BUILD)/imu-compensation-cases-cm
	./$(BUILD)/imu-compensation-oracle
	./$(BUILD)/imu-compensation-cases-lm
	./$(BUILD)/imu-compensation-cases-cm

# R63, the LM's V89 final attitude: the port's test (tests/test_lm_r63.c,
# its frozen cases run to their stops through the program of
# tests/lm_r63_target.c, against the original program's yaAGC digests) on
# the readable routines, with tests/with_ropes.c loading the ropes the
# readable interpreter reads; then tests/r63_cases.c side by side.
R63_NAMES := lm_r63_basic lm_r63_interpretive
R63_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rtb_angles.c $(PORT)/src/rope_image_host.c $(PORT)/tests/interp_case.c

$(BUILD)/r63-faithful.o: $(PORT)/src/lm_r63.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(R63_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/r63-test.o: $(PORT)/tests/test_lm_r63.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/r63-oracle: $(R63_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r63-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R63_COMMON) $(PORT)/tests/lm_r63_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/r63-cases: $(R63_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r63-faithful.o \
		tests/r63_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R63_COMMON) $(filter %.o,$^) tests/r63_cases.c -o $@

r63-cases: $(BUILD)/r63-oracle $(BUILD)/r63-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/r63-oracle $(ROPES)
	./$(BUILD)/r63-cases $(ROPES)

# ATTITUDE_MANEUVER_ROUTINE (the LM's) and ANGLFIND (the CM's), KALCMANU's
# start and matrix routines: the port's test (tests/test_attitude_maneuver.c,
# its frozen cases run to their stops through the programs of
# tests/attitude_maneuver_target.c, against the original programs' yaAGC
# digests) with each mission's readable routines and the other mission's
# faithful ones, tests/with_ropes.c loading the ropes the readable
# interpreter reads; then tests/attitude_maneuver_cases.c side by side, for
# each mission.
AMR_NAMES_LM := lm_attitude_maneuver_basic lm_attitude_maneuver_interpretive
AMR_NAMES_CM := cm_attitude_maneuver_basic cm_attitude_maneuver_interpretive
AMR_COMMON := $(R63_COMMON)

$(BUILD)/amr-faithful-lm.o: $(PORT)/src/lm_attitude_maneuver.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AMR_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/amr-faithful-cm.o: $(PORT)/src/cm_attitude_maneuver.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AMR_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/amr-test.o: $(PORT)/tests/test_attitude_maneuver.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/amr-oracle-lm: $(AMR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/amr-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(AMR_COMMON) $(PORT)/src/cm_attitude_maneuver.c \
		$(PORT)/tests/attitude_maneuver_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/amr-oracle-cm: $(AMR_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/amr-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(AMR_COMMON) $(PORT)/src/lm_attitude_maneuver.c \
		$(PORT)/tests/attitude_maneuver_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/amr-cases-lm: $(AMR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/amr-faithful-lm.o \
		tests/attitude_maneuver_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AMR_COMMON) $(filter %.o,$^) \
		tests/attitude_maneuver_cases.c -o $@
$(BUILD)/amr-cases-cm: $(AMR_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/amr-faithful-cm.o \
		tests/attitude_maneuver_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(AMR_COMMON) $(filter %.o,$^) \
		tests/attitude_maneuver_cases.c -o $@

attitude-maneuver-cases: $(BUILD)/amr-oracle-lm $(BUILD)/amr-oracle-cm $(BUILD)/amr-cases-lm \
		$(BUILD)/amr-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/amr-oracle-lm $(ROPES)
	./$(BUILD)/amr-oracle-cm $(ROPES)
	./$(BUILD)/amr-cases-lm $(ROPES)
	./$(BUILD)/amr-cases-cm $(ROPES)

# GIMBAL_LOCK_AVOIDANCE, both missions' LOCSKIRT: the port's test
# (tests/test_gimbal_lock.c, its frozen cases run to their stops through
# the programs of tests/gimbal_lock_target.c, against the original
# programs' yaAGC digests) with each mission's readable routines and the
# other mission's faithful ones, tests/with_ropes.c loading the ropes the
# readable interpreter reads; then tests/gimbal_lock_cases.c side by side,
# for each mission.
GLA_NAMES_LM := lm_gimbal_lock_basic lm_gimbal_lock_interpretive
GLA_NAMES_CM := cm_gimbal_lock_basic cm_gimbal_lock_interpretive
GLA_COMMON := $(R63_COMMON)

$(BUILD)/gla-faithful-lm.o: $(PORT)/src/lm_gimbal_lock.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(GLA_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/gla-faithful-cm.o: $(PORT)/src/cm_gimbal_lock.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(GLA_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/gla-test.o: $(PORT)/tests/test_gimbal_lock.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/gla-oracle-lm: $(GLA_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/gla-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(GLA_COMMON) $(PORT)/src/cm_gimbal_lock.c \
		$(PORT)/tests/gimbal_lock_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/gla-oracle-cm: $(GLA_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/gla-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(GLA_COMMON) $(PORT)/src/lm_gimbal_lock.c \
		$(PORT)/tests/gimbal_lock_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/gla-cases-lm: $(GLA_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/gla-faithful-lm.o \
		tests/gimbal_lock_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(GLA_COMMON) $(filter %.o,$^) \
		tests/gimbal_lock_cases.c -o $@
$(BUILD)/gla-cases-cm: $(GLA_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/gla-faithful-cm.o \
		tests/gimbal_lock_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(GLA_COMMON) $(filter %.o,$^) \
		tests/gimbal_lock_cases.c -o $@

gimbal-lock-cases: $(BUILD)/gla-oracle-lm $(BUILD)/gla-oracle-cm $(BUILD)/gla-cases-lm \
		$(BUILD)/gla-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/gla-oracle-lm $(ROPES)
	./$(BUILD)/gla-oracle-cm $(ROPES)
	./$(BUILD)/gla-cases-lm $(ROPES)
	./$(BUILD)/gla-cases-cm $(ROPES)

# KALCMANU_STEERING, both missions' maneuver steering: the port's test
# (tests/test_kalcmanu_steering.c, its frozen cases run to their stops
# through the programs of tests/kalcmanu_steering_target.c, against the
# original programs' yaAGC digests) with each mission's readable routines
# and the other mission's faithful ones, tests/with_ropes.c loading the
# ropes the readable interpreter reads; then tests/kalcmanu_steering_cases.c
# side by side, for each mission.
KAL_NAMES_LM := lm_kalcmanu_steering_basic lm_kalcmanu_steering_interpretive
KAL_NAMES_CM := cm_kalcmanu_steering_basic cm_kalcmanu_steering_interpretive
KAL_COMMON := $(R63_COMMON)

$(BUILD)/kal-faithful-lm.o: $(PORT)/src/lm_kalcmanu_steering.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(KAL_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/kal-faithful-cm.o: $(PORT)/src/cm_kalcmanu_steering.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(KAL_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/kal-test.o: $(PORT)/tests/test_kalcmanu_steering.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/kal-oracle-lm: $(KAL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/kal-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KAL_COMMON) $(PORT)/src/cm_kalcmanu_steering.c \
		$(PORT)/tests/kalcmanu_steering_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/kal-oracle-cm: $(KAL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/kal-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KAL_COMMON) $(PORT)/src/lm_kalcmanu_steering.c \
		$(PORT)/tests/kalcmanu_steering_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/kal-cases-lm: $(KAL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/kal-faithful-lm.o \
		tests/kalcmanu_steering_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KAL_COMMON) $(filter %.o,$^) \
		tests/kalcmanu_steering_cases.c -o $@
$(BUILD)/kal-cases-cm: $(KAL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/kal-faithful-cm.o \
		tests/kalcmanu_steering_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KAL_COMMON) $(filter %.o,$^) \
		tests/kalcmanu_steering_cases.c -o $@

kalcmanu-steering-cases: $(BUILD)/kal-oracle-lm $(BUILD)/kal-oracle-cm $(BUILD)/kal-cases-lm \
		$(BUILD)/kal-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/kal-oracle-lm $(ROPES)
	./$(BUILD)/kal-oracle-cm $(ROPES)
	./$(BUILD)/kal-cases-lm $(ROPES)
	./$(BUILD)/kal-cases-cm $(ROPES)
