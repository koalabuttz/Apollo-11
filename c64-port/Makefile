CC = cc
MOS_CC ?= mos-c64-clang
CFLAGS ?= -O2 -g -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic
CPPFLAGS = -Iinclude -Itests
CORE = src/agc_word.c src/executive.c src/native_dispatch.c src/irq_delivery.c
SCENARIOS = tests/scenarios.c tests/state_digest.c tests/dispatch_cases.c tests/dispatch_check.c
HEADERS = include/agc_word.h include/executive.h tests/scenarios.h tests/state_digest.h tests/reference_vectors.h include/native_dispatch.h include/irq_delivery.h tests/dispatch_cases.h tests/dispatch_check.h
.PHONY: all test c64 sanitize clean differential
all: build/test-waitlist build/waitlist-snapshot build/test-irq build/test-host build/test-lifecycle build/snapshot build/arithmetic-dump build/dispatch-snapshot build/lifecycle-snapshot
build:
	mkdir -p build
build/test-host: $(CORE) $(SCENARIOS) tests/test_host.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) tests/test_host.c -o $@
build/snapshot: $(CORE) $(SCENARIOS) platform/host/snapshot.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) platform/host/snapshot.c -o $@
build/arithmetic-dump: src/agc_word.c tests/arithmetic_dump.c tests/arithmetic_stream.h include/agc_word.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/agc_word.c tests/arithmetic_dump.c -o $@
test: all
	./build/test-host
	./build/test-irq
	./build/test-lifecycle
	./build/test-waitlist
sanitize: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) $(SCENARIOS) tests/test_host.c -o build/test-sanitize
	./build/test-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) tests/scenarios.c tests/test_irq.c -o build/test-irq-sanitize
	./build/test-irq-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c tests/test_lifecycle.c -o build/test-lifecycle-sanitize
	./build/test-lifecycle-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(TASK_CORE) $(WAIT_TEST) tests/test_waitlist.c -o build/test-waitlist-sanitize
	./build/test-waitlist-sanitize
c64: build/executive.prg
build/executive.prg: $(CORE) $(SCENARIOS) platform/c64/main.c $(HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -Wall -Wextra -Werror -Wl,-Map,build/executive.map $(CORE) $(SCENARIOS) platform/c64/main.c -o $@
differential: all
	python3 tools/differential.py --reference "$(REFERENCE)"
clean:
	rm -rf build

build/dispatch-snapshot: $(CORE) $(SCENARIOS) platform/host/dispatch_snapshot.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) platform/host/dispatch_snapshot.c -o $@

build/test-irq: $(CORE) tests/scenarios.c tests/test_irq.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) tests/scenarios.c tests/test_irq.c -o $@

LIFECYCLE = tests/lifecycle_cases.c
build/lifecycle-snapshot: $(CORE) $(SCENARIOS) $(LIFECYCLE) platform/host/lifecycle_snapshot.c $(HEADERS) tests/lifecycle_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) $(LIFECYCLE) platform/host/lifecycle_snapshot.c -o $@
build/test-lifecycle: $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c tests/test_lifecycle.c $(HEADERS) tests/lifecycle_cases.h tests/lifecycle_check.h tests/lifecycle_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c tests/test_lifecycle.c -o $@

.PHONY: c64-irq
c64-irq: build/irq.prg
build/irq.prg: $(CORE) $(SCENARIOS) platform/c64/irq_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_platform.h platform/c64/irq_gate.S $(HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -Iplatform/c64 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/irq.map $(CORE) $(SCENARIOS) platform/c64/irq_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_gate.S -o $@

.PHONY: c64-lifecycle
c64-lifecycle: build/lifecycle.prg
build/lifecycle.prg: $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c platform/c64/lifecycle_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_platform.h platform/c64/irq_gate.S $(HEADERS) tests/lifecycle_cases.h tests/lifecycle_check.h tests/lifecycle_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -Iplatform/c64 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lifecycle.map $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c platform/c64/lifecycle_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_gate.S -o $@

TASK_CORE = src/agc_word.c src/waitlist.c src/native_tasks.c
WAIT_TEST = tests/waitlist_cases.c tests/waitlist_check.c
WAIT_HEADERS = include/waitlist.h include/native_tasks.h tests/waitlist_cases.h tests/waitlist_check.h tests/waitlist_vectors.h
build/test-waitlist: $(TASK_CORE) $(WAIT_TEST) tests/test_waitlist.c $(WAIT_HEADERS) $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TASK_CORE) $(WAIT_TEST) tests/test_waitlist.c -o $@
build/waitlist-snapshot: $(TASK_CORE) tests/waitlist_cases.c platform/host/waitlist_snapshot.c $(WAIT_HEADERS) $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TASK_CORE) tests/waitlist_cases.c platform/host/waitlist_snapshot.c -o $@

.PHONY: c64-waitlist
c64-waitlist: build/waitlist.prg
build/waitlist.prg: $(CORE) src/waitlist.c src/native_tasks.c $(WAIT_TEST) platform/c64/waitlist_diagnostic.c platform/c64/timer_platform.c platform/c64/timer_platform.h platform/c64/timer_gate.S $(HEADERS) $(WAIT_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -Iplatform/c64 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/waitlist.map $(CORE) src/waitlist.c src/native_tasks.c $(WAIT_TEST) platform/c64/waitlist_diagnostic.c platform/c64/timer_platform.c platform/c64/timer_gate.S -o $@

RESTART_CORE = src/agc_word.c src/executive.c src/waitlist.c src/restart.c
RESTART_HEADERS = $(HEADERS) include/waitlist.h include/restart.h tests/restart_cases.h
all: build/restart-snapshot
build/restart-snapshot: $(RESTART_CORE) tests/restart_cases.c platform/host/restart_snapshot.c $(RESTART_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RESTART_CORE) tests/restart_cases.c platform/host/restart_snapshot.c -o $@

RESTART_TEST = tests/restart_cases.c tests/restart_check.c
all: build/test-restart
test: test-restart
.PHONY: test-restart
test-restart: build/test-restart
	./build/test-restart
build/test-restart: $(CORE) src/waitlist.c src/restart.c $(RESTART_TEST) tests/test_restart.c $(RESTART_HEADERS) tests/restart_vectors.h tests/restart_check.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) src/waitlist.c src/restart.c $(RESTART_TEST) tests/test_restart.c -o $@
sanitize: sanitize-restart
.PHONY: sanitize-restart
sanitize-restart: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) src/waitlist.c src/restart.c $(RESTART_TEST) tests/test_restart.c -o build/test-restart-sanitize
	./build/test-restart-sanitize

.PHONY: c64-restart
c64-restart: build/restart.prg
build/restart.prg: $(CORE) src/waitlist.c src/restart.c $(RESTART_TEST) platform/c64/restart_diagnostic.c $(RESTART_HEADERS) tests/restart_vectors.h tests/restart_check.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/restart.map $(CORE) src/waitlist.c src/restart.c $(RESTART_TEST) platform/c64/restart_diagnostic.c -o $@

GUIDANCE_CORE = src/agc_word.c src/agc_multiword.c src/guidance.c
GUIDANCE_INPUT = tests/guidance_cases.c
GUIDANCE_HEADERS = include/agc_word.h include/agc_multiword.h include/guidance.h tests/guidance_cases.h
all: build/guidance-snapshot
build/guidance-snapshot: $(GUIDANCE_CORE) $(GUIDANCE_INPUT) platform/host/guidance_snapshot.c $(GUIDANCE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GUIDANCE_CORE) $(GUIDANCE_INPUT) platform/host/guidance_snapshot.c -o $@
GUIDANCE_TEST = $(GUIDANCE_INPUT) tests/guidance_check.c
all: build/test-guidance
test: test-guidance
.PHONY: test-guidance sanitize-guidance c64-guidance
test-guidance: build/test-guidance
	./build/test-guidance
build/test-guidance: $(GUIDANCE_CORE) $(GUIDANCE_TEST) tests/test_guidance.c $(GUIDANCE_HEADERS) tests/guidance_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GUIDANCE_CORE) $(GUIDANCE_TEST) tests/test_guidance.c -o $@
sanitize: sanitize-guidance
sanitize-guidance: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(GUIDANCE_CORE) $(GUIDANCE_TEST) tests/test_guidance.c -o build/test-guidance-sanitize
	./build/test-guidance-sanitize
c64-guidance: build/guidance.prg
build/guidance.prg: $(GUIDANCE_CORE) $(GUIDANCE_TEST) platform/c64/guidance_diagnostic.c $(GUIDANCE_HEADERS) tests/guidance_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/guidance.map $(GUIDANCE_CORE) $(GUIDANCE_TEST) platform/c64/guidance_diagnostic.c -o $@

PREPARE_CORE = $(GUIDANCE_CORE) src/guidance_prepare.c
PREPARE_INPUT = tests/prepare_cases.c
PREPARE_HEADERS = $(GUIDANCE_HEADERS) include/guidance_prepare.h tests/prepare_cases.h
all: build/prepare-snapshot
build/prepare-snapshot: $(PREPARE_CORE) $(PREPARE_INPUT) platform/host/prepare_snapshot.c $(PREPARE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PREPARE_CORE) $(PREPARE_INPUT) platform/host/prepare_snapshot.c -o $@
PREPARE_TEST = $(PREPARE_INPUT) tests/prepare_check.c
all: build/test-prepare
test: test-prepare
.PHONY: test-prepare sanitize-prepare c64-prepare
test-prepare: build/test-prepare
	./build/test-prepare
build/test-prepare: $(PREPARE_CORE) $(PREPARE_TEST) tests/test_prepare.c $(PREPARE_HEADERS) tests/prepare_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PREPARE_CORE) $(PREPARE_TEST) tests/test_prepare.c -o $@
sanitize: sanitize-prepare
sanitize-prepare: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(PREPARE_CORE) $(PREPARE_TEST) tests/test_prepare.c -o build/test-prepare-sanitize
	./build/test-prepare-sanitize
c64-prepare: build/prepare.prg
build/prepare.prg: $(PREPARE_CORE) $(PREPARE_TEST) platform/c64/prepare_diagnostic.c $(PREPARE_HEADERS) tests/prepare_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/prepare.map $(PREPARE_CORE) $(PREPARE_TEST) platform/c64/prepare_diagnostic.c -o $@

SCALE_CORE = $(PREPARE_CORE) src/guidance_scale.c
SCALE_INPUT = $(PREPARE_INPUT) tests/scale_cases.c
SCALE_HEADERS = $(PREPARE_HEADERS) include/guidance_scale.h tests/scale_cases.h
all: build/scale-snapshot
build/scale-snapshot: $(SCALE_CORE) $(SCALE_INPUT) platform/host/scale_snapshot.c $(SCALE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SCALE_CORE) $(SCALE_INPUT) platform/host/scale_snapshot.c -o $@

SCALE_TEST = $(SCALE_INPUT) tests/scale_check.c
all: build/test-scale
test: test-scale
.PHONY: test-scale sanitize-scale c64-scale
test-scale: build/test-scale
	./build/test-scale
build/test-scale: $(SCALE_CORE) $(SCALE_TEST) tests/test_scale.c $(SCALE_HEADERS) tests/scale_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SCALE_CORE) $(SCALE_TEST) tests/test_scale.c -o $@
sanitize: sanitize-scale
sanitize-scale: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SCALE_CORE) $(SCALE_TEST) tests/test_scale.c -o build/test-scale-sanitize
	./build/test-scale-sanitize
c64-scale: build/scale.prg
build/scale.prg: $(SCALE_CORE) $(SCALE_TEST) platform/c64/scale_diagnostic.c $(SCALE_HEADERS) tests/scale_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/scale.map $(SCALE_CORE) $(SCALE_TEST) platform/c64/scale_diagnostic.c -o $@

PUSH_CORE = $(SCALE_CORE) src/guidance_push.c
PUSH_INPUT = $(SCALE_INPUT) tests/push_cases.c
PUSH_HEADERS = $(SCALE_HEADERS) include/guidance_push.h tests/push_cases.h
all: build/push-snapshot
build/push-snapshot: $(PUSH_CORE) $(PUSH_INPUT) platform/host/push_snapshot.c $(PUSH_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PUSH_CORE) $(PUSH_INPUT) platform/host/push_snapshot.c -o $@

PUSH_TEST = $(PUSH_INPUT) tests/push_check.c
all: build/test-push
test: test-push
.PHONY: test-push sanitize-push c64-push
test-push: build/test-push
	./build/test-push
build/test-push: $(PUSH_CORE) $(PUSH_TEST) tests/test_push.c $(PUSH_HEADERS) tests/push_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PUSH_CORE) $(PUSH_TEST) tests/test_push.c -o $@
sanitize: sanitize-push
sanitize-push: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(PUSH_CORE) $(PUSH_TEST) tests/test_push.c -o build/test-push-sanitize
	./build/test-push-sanitize
c64-push: build/push.prg
build/push.prg: $(PUSH_CORE) $(PUSH_TEST) platform/c64/push_diagnostic.c $(PUSH_HEADERS) tests/push_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/push.map $(PUSH_CORE) $(PUSH_TEST) platform/c64/push_diagnostic.c -o $@

ELAPSED_CORE = $(PUSH_CORE) src/guidance_elapsed.c
ELAPSED_INPUT = $(PUSH_INPUT) tests/elapsed_cases.c
ELAPSED_HEADERS = $(PUSH_HEADERS) include/guidance_elapsed.h tests/elapsed_cases.h
all: build/elapsed-snapshot
build/elapsed-snapshot: $(ELAPSED_CORE) $(ELAPSED_INPUT) platform/host/elapsed_snapshot.c $(ELAPSED_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ELAPSED_CORE) $(ELAPSED_INPUT) platform/host/elapsed_snapshot.c -o $@

ELAPSED_TEST = $(ELAPSED_INPUT) tests/elapsed_check.c
all: build/test-elapsed
test: test-elapsed
.PHONY: test-elapsed sanitize-elapsed c64-elapsed
test-elapsed: build/test-elapsed
	./build/test-elapsed
build/test-elapsed: $(ELAPSED_CORE) $(ELAPSED_TEST) tests/test_elapsed.c $(ELAPSED_HEADERS) tests/elapsed_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ELAPSED_CORE) $(ELAPSED_TEST) tests/test_elapsed.c -o $@
sanitize: sanitize-elapsed
sanitize-elapsed: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(ELAPSED_CORE) $(ELAPSED_TEST) tests/test_elapsed.c -o build/test-elapsed-sanitize
	./build/test-elapsed-sanitize
c64-elapsed: build/elapsed.prg
build/elapsed.prg: $(ELAPSED_CORE) $(ELAPSED_TEST) platform/c64/elapsed_diagnostic.c $(ELAPSED_HEADERS) tests/elapsed_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/elapsed.map $(ELAPSED_CORE) $(ELAPSED_TEST) platform/c64/elapsed_diagnostic.c -o $@

STORE_CORE = $(ELAPSED_CORE) src/guidance_store.c
STORE_INPUT = $(ELAPSED_INPUT) tests/store_cases.c
STORE_HEADERS = $(ELAPSED_HEADERS) include/guidance_store.h tests/store_cases.h
all: build/store-snapshot
build/store-snapshot: $(STORE_CORE) $(STORE_INPUT) platform/host/store_snapshot.c $(STORE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STORE_CORE) $(STORE_INPUT) platform/host/store_snapshot.c -o $@

STORE_TEST = $(STORE_INPUT) tests/store_check.c
all: build/test-store
test: test-store
.PHONY: test-store sanitize-store c64-store
test-store: build/test-store
	./build/test-store
build/test-store: $(STORE_CORE) $(STORE_TEST) tests/test_store.c $(STORE_HEADERS) tests/store_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STORE_CORE) $(STORE_TEST) tests/test_store.c -o $@
sanitize: sanitize-store
sanitize-store: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(STORE_CORE) $(STORE_TEST) tests/test_store.c -o build/test-store-sanitize
	./build/test-store-sanitize
c64-store: build/store.prg
build/store.prg: $(STORE_CORE) $(STORE_TEST) platform/c64/store_diagnostic.c $(STORE_HEADERS) tests/store_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/store.map $(STORE_CORE) $(STORE_TEST) platform/c64/store_diagnostic.c -o $@

DIVIDE_CORE = $(STORE_CORE) src/guidance_divide.c
DIVIDE_INPUT = $(STORE_INPUT) tests/divide_cases.c
DIVIDE_HEADERS = $(STORE_HEADERS) include/guidance_divide.h tests/divide_cases.h
all: build/divide-snapshot
build/divide-snapshot: $(DIVIDE_CORE) $(DIVIDE_INPUT) platform/host/divide_snapshot.c $(DIVIDE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DIVIDE_CORE) $(DIVIDE_INPUT) platform/host/divide_snapshot.c -o $@

DIVIDE_TEST = $(DIVIDE_INPUT) tests/divide_check.c
all: build/test-divide
test: test-divide
.PHONY: test-divide sanitize-divide c64-divide
test-divide: build/test-divide
	./build/test-divide
build/test-divide: $(DIVIDE_CORE) $(DIVIDE_TEST) tests/test_divide.c $(DIVIDE_HEADERS) tests/divide_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DIVIDE_CORE) $(DIVIDE_TEST) tests/test_divide.c -o $@
sanitize: sanitize-divide
sanitize-divide: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(DIVIDE_CORE) $(DIVIDE_TEST) tests/test_divide.c -o build/test-divide-sanitize
	./build/test-divide-sanitize
c64-divide: build/divide.prg
build/divide.prg: $(DIVIDE_CORE) $(DIVIDE_TEST) platform/c64/divide_diagnostic.c $(DIVIDE_HEADERS) tests/divide_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/divide.map $(DIVIDE_CORE) $(DIVIDE_TEST) platform/c64/divide_diagnostic.c -o $@

GRAVITY_CORE = $(DIVIDE_CORE) src/guidance_gravity.c
GRAVITY_INPUT = $(DIVIDE_INPUT) tests/gravity_cases.c
GRAVITY_HEADERS = $(DIVIDE_HEADERS) include/guidance_gravity.h tests/gravity_cases.h
all: build/gravity-snapshot
build/gravity-snapshot: $(GRAVITY_CORE) $(GRAVITY_INPUT) platform/host/gravity_snapshot.c $(GRAVITY_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GRAVITY_CORE) $(GRAVITY_INPUT) platform/host/gravity_snapshot.c -o $@
GRAVITY_TEST = $(GRAVITY_INPUT) tests/gravity_check.c tests/frozen_words.c
all: build/test-gravity
test: test-gravity
.PHONY: test-gravity sanitize-gravity c64-gravity
test-gravity: build/test-gravity
	./build/test-gravity
build/test-gravity: $(GRAVITY_CORE) $(GRAVITY_TEST) tests/test_gravity.c $(GRAVITY_HEADERS) tests/gravity_vectors.h tests/frozen_words.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GRAVITY_CORE) $(GRAVITY_TEST) tests/test_gravity.c -o $@
sanitize: sanitize-gravity
sanitize-gravity: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(GRAVITY_CORE) $(GRAVITY_TEST) tests/test_gravity.c -o build/test-gravity-sanitize
	./build/test-gravity-sanitize
c64-gravity: build/gravity.prg
build/gravity.prg: $(GRAVITY_CORE) $(GRAVITY_TEST) platform/c64/gravity_diagnostic.c $(GRAVITY_HEADERS) tests/gravity_vectors.h tests/frozen_words.h | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/gravity.map $(GRAVITY_CORE) $(GRAVITY_TEST) platform/c64/gravity_diagnostic.c -o $@

include tools/vector.mk

include tools/executive_services.mk

include tools/phase.mk

include tools/product.mk

include tools/shift.mk

include tools/dp_divide.mk

include tools/velocity.mk

include tools/norm.mk

include tools/recovery.mk

include tools/dot.mk

include tools/sum.mk

.PHONY: coverage
coverage:
	python3 tools/port_coverage.py

include tools/interpreter_native.mk

include tools/enema.mk

include tools/cm_executive.mk

include tools/unit.mk

include tools/radial.mk

include tools/acceleration.mk

include tools/overlay.mk

include tools/waitlist_zero.mk

include tools/cm_dispatch.mk

include tools/p66_startup.mk

include tools/trig.mk

include tools/p66_native.mk

include tools/rotation.mk

include tools/projection.mk

include tools/hardware_divide.mk

include tools/p66_initialization.mk

include tools/throttle_feedback.mk

include tools/throttle_service.mk

include tools/p66_bias.mk

include tools/p66_vertical_start.mk

include tools/cm_waitlist.mk

include tools/cm_irq.mk

include tools/p66_major_mode.mk

include tools/p66_launch.mk

include tools/horizontal.mk

include tools/throttle_bridge.mk

include tools/p66_mode_display.mk

include tools/cm_phase.mk
include tools/cm_restart.mk

include tools/cm_restart_native.mk

include tools/cm_native_image.mk

include tools/cm_enema.mk

include tools/unit_phased.mk
include tools/radial_phased.mk
include tools/projection_phased.mk
include tools/guidance_phased_frozen.mk
.PHONY: test-guidance-phased
test: test-guidance-phased
test-guidance-phased:
	$(MAKE) guidance-phased-host KIND=unit
	$(MAKE) guidance-phased-host KIND=radial
	$(MAKE) guidance-phased-host KIND=projection

include tools/dsky_relay.mk

include tools/dsky_display.mk

include tools/cm_startup.mk

include tools/acceleration_phased.mk
include tools/control_observer.mk

include tools/velocity_phased.mk

include tools/throttle_feedback_phased.mk

include tools/display_control.mk

include tools/regodspr.mk

include tools/acceleration_store.mk

include tools/nv_entry.mk

include tools/nv_words.mk

include tools/dsky_pinball.mk

include tools/dsky_decimal.mk

include tools/dsky_scale.mk

include tools/dsky_two_digit.mk

include tools/cm_numeric.mk

include tools/cm_rotation.mk

include tools/cm_rectify.mk

include tools/p66_guidance.mk

include tools/cm_difeq.mk

include tools/cm_orbital_initial.mk

include tools/dsky_pinball_zero.mk
include tools/p66_mode_zero.mk

include tools/p66_status.mk

include tools/nv_rows.mk

include tools/nv_render.mk

include tools/goplay.mk

include tools/lm_priority.mk

include tools/runtime_bridge.mk

include tools/fixed_data.mk

include tools/p66_fine_phases.mk

include tools/p66_basic.mk

include tools/dsky_nv_key.mk

include tools/dsky_charin_nv.mk

include tools/dsky_keycom.mk

include tools/dsky_keyrupt.mk

include tools/cm_tig5.mk

include tools/cm_entry_tasks.mk

include tools/runtime_regodspr.mk

include tools/runtime_fetch.mk

include tools/lm_wake.mk

include tools/makeplay.mk

include tools/cm_restart_timing.mk

include tools/cm_longcall.mk

include tools/cm_restart_select.mk

include tools/cm_clocktask.mk

include tools/cm_waitlist_image.mk

include tools/cm_task_transfer.mk

include tools/cm_admission_image.mk

include tools/cm_svct3.mk

include tools/cm_delay_image.mk

include tools/cm_trig_port.mk

include tools/cm_single_return_port.mk

include tools/lm_sleep.mk

include tools/makeplay_sleep.mk

include tools/lm_busy.mk

include tools/cm_display.mk

include tools/cm_pinball.mk

include tools/cm_formatter.mk

include tools/noun_reader.mk

include tools/layout.mk

include tools/lm_single_trig_port.mk

include tools/lm_finish.mk

include tools/cm_mysubs_port.mk

include tools/cm_task_interrupt.mk

include tools/cm_service_loop.mk

include tools/cm_service_overlay.mk

include tools/cm_relay.mk

include tools/cm_scale.mk

include tools/lm_torque.mk

test: test-documentation-coverage
test-documentation-coverage:
	python3 tests/test_documentation_coverage.py
.PHONY: test-documentation-coverage

include tools/native_source_adoption.mk
all: native_source_adoption-host
test: native_source_adoption-host
sanitize: native_source_adoption-sanitize

include tools/cm_basic_finish.mk

include tools/dsky_charin_number.mk

include tools/cm_clockjob_port.mk

include tools/cm_display_control_port.mk

include tools/dsky_key_irq.mk
include tools/dsky_keyboard_flow.mk
include tools/dsky_keyboard_irq_flow.mk

include tools/display_job.mk

include tools/native_scheduler.mk

include tools/dsky_keyboard_number_flow.mk

include tools/cm_danzig.mk

include tools/cm_powered_trig_port.mk

include tools/gimbal_port.mk

include tools/cm_charin_number.mk

include tools/cm_mission_loop.mk
all: build/cm-mission-loop-native

include tools/p66_complete.mk

include tools/cm_powered_axis_port.mk

include tools/cm_powered_interface_port.mk

include tools/cm_makeplay_port.mk

include tools/cm_wake_port.mk

include tools/cm_keyboard_irq.mk

include tools/cm_charin_nv.mk

include tools/cm_waitlist_extended.mk

include tools/delcomp_port.mk

include tools/cm_interbank.mk

include tools/cm_idle_dispatch.mk

include tools/cm_makeplay_sleep_port.mk

include tools/cm_sleep_port.mk

include tools/cm_clock_mission.mk

include tools/p66_direct_launch.mk

include tools/cm_keyboard_runtime.mk

include tools/gimbal_complete_port.mk

include tools/cm_display_sleep_lifecycle_port.mk

include tools/cm_smcdure_port.mk

include tools/cm_nbsm_port.mk

include tools/cm_powered_quick_port.mk

include tools/cm_powered_fixed_return_port.mk

include tools/lm_interpreter_change.mk
all: build/test-lm-interpreter-change
build/test-lm-interpreter-change: $(LM_INTERPRETER_CHANGE_CORE) $(LM_INTERPRETER_CHANGE_TEST) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-lm-interpreter-change
sanitize: sanitize-lm-interpreter-change

include tools/p66_direct_pending.mk

include tools/dsky_charin_sign.mk

include tools/p66_launch_phased_port.mk

include tools/matrix_port.mk

include tools/dsky_charin_clear.mk

include tools/passive_storage_port.mk

include tools/cm_fixed_compat.mk
all: build/cm-fixed-compat-native

include tools/dsky_charin_reset.mk

include tools/native_exchange.mk
all: build/test-native-exchange
build/test-native-exchange: $(NATIVE_EXCHANGE_CORE) $(NATIVE_EXCHANGE_TEST) tests/test_native_exchange.c | build
	$(CC) -O2 $(NATIVE_EXCHANGE_FLAGS) $^ -o $@
test: test-native-exchange
sanitize: sanitize-native-exchange

include tools/cm_interbank_extended.mk

# The complete original fixed-memory caller proofs require this fresh host
# assembly fixture, including in a clean checkout. It is never target code.
.PHONY: prepare-cm-original-rope
prepare-cm-original-rope: | build
	python3 tools/prepare_cm_original_rope.py
cm-fixed-compat-differential cm-interbank-extended-differential: prepare-cm-original-rope

include tools/cm_dyndisp_port.mk

include tools/dsky_charin_alarm.mk

include tools/lm_powered_trig_port.mk

include tools/lm_powered_axis_port.mk

include tools/lm_powered_quick_port.mk

include tools/lm_powered_interface_port.mk

include tools/lm_powered_fixed_return_port.mk

include tools/lm_fleshpot_port.mk

include tools/lm_fleshpot_interface_port.mk

include tools/dsky_charin_release.mk

include tools/cm_nv85_port.mk

include tools/cm_context_change.mk
all: build/cm-context-change-native

include tools/inverse_trig_port.mk

include tools/cm_dynamic_display_port.mk

include tools/cm_admission_complete.mk
all: build/cm-admission-complete-native

include tools/cm_rcs_matrix_port.mk

include tools/dsky_blank.mk

include tools/cm_priority_complete.mk
all: build/cm-priority-complete-native

include tools/standard_lead_integration.mk

include tools/dsky_pinbranch.mk

include tools/inverse_trig_sweep_port.mk

include tools/dsky_blanksub.mk

include tools/dsky_enter.mk

include tools/cm_minutes_seconds_port.mk

include tools/p66_application_integration.mk

include tools/lm_sps_rcs_port.mk

include tools/dsky_enter_zero.mk

include tools/lm_recurrence_integration.mk

include tools/cm_normal_display.mk

include tools/dsky_mixed_noun.mk

include tools/interpreted_sign_port.mk

include tools/cm_clock_display_mission.mk
include tools/cm_clock_display_busy.mk
all: build/cm-clock-display-mission-native build/cm-clock-display-busy-native

include tools/dsky_verb_dispatch.mk

include tools/cm_clock_normal_display.mk

include tools/interpreted_square_port.mk

include tools/dsky_display_release.mk

include tools/dsky_extended_dispatch.mk

include tools/lm_interbank_integration.mk

include tools/dsky_source_alarm.mk

include tools/cm_init_display_mixed.mk
all: build/cm-init-display-mixed-native

include tools/lm_rcs_monitor_integration.mk

include tools/dsky_nv_admission_integration.mk

include tools/dsky_load_control.mk

include tools/lm_admission_image_integration.mk

include tools/cm_alarm_record.mk
all: build/cm-alarm-record-native

include tools/dsky_nv_return_integration.mk

include tools/dsky_nv_busy_integration.mk

include tools/cm_vac5_store.mk
all: build/cm-vac5-store-native

include tools/dsky_jam_control.mk

include tools/p66_application_recurrence_integration.mk

include tools/cm_interbank_complete_integration.mk

include tools/cm_display_rest_integration.mk

include tools/cm_clock_blocked_display_integration.mk

include tools/lm_waitlist_image_integration.mk

include tools/dsky_nv_continuation_integration.mk

include tools/dsky_nv_update_integration.mk

include tools/lm_basic_change_integration.mk

include tools/dsky_octal_display.mk

include tools/dsky_machine_cadr.mk

include tools/dsky_five_blank.mk

include tools/rtb_angles_integration.mk

include tools/cm_abort_snapshot.mk
all: build/cm-abort-snapshot-native

include tools/cm_abort_whimper.mk
all: build/cm-abort-whimper-native

include tools/cm_phase_cleanup.mk
all: build/cm-phase-cleanup-native

include tools/rtb_agreement_integration.mk

include tools/dsky_address_request.mk

include tools/dsky_nv_blank_integration.mk

include tools/dsky_nv_consumer_integration.mk

include tools/service_routines_integration.mk

include tools/dsky_alarm_cycle.mk

include tools/dsky_nv_update_general_integration.mk

include tools/lm_danzig_change_integration.mk

include tools/rtb_vector_integration.mk

include tools/cm_enema_image.mk
all: build/cm-enema-image-native

include tools/dsky_alarm_recycle.mk

include tools/dsky_decimal_load_integration.mk

include tools/cm_abort_recovery.mk
all: build/cm-abort-recovery-native

include tools/dsky_major_request.mk

include tools/cm_admission_zero.mk
all: build/cm-admission-zero-native

include tools/rtb_unit_integration.mk

include tools/dsky_request_control.mk

include tools/dsky_decimal_select_integration.mk
