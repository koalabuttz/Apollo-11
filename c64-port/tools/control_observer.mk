# SPDX-License-Identifier: MIT
CO_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CO_CORE = src/guidance_control_observer.c src/agc_word.c src/agc_multiword.c
control_observer-host:
	gcc -O2 -Wconversion -pedantic $(CO_FLAGS) tests/test_control_observer.c $(CO_CORE) -o build/test-control-observer
	build/test-control-observer
control_observer-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CO_FLAGS) tests/test_control_observer.c $(CO_CORE) -o build/test-control-observer-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-control-observer-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CO_FLAGS) tools/acceleration_control_observer_snapshot.c $(CO_CORE) -o build/control-observer-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/control-observer-native-sanitize < build/acceleration-control-inputs.snapshots > build/control-observer-sanitize.snapshots
	cmp build/control-observer-sanitize.snapshots build/acceleration-control-observer.snapshots
control_observer-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCONTROL_OBSERVER_CHECK_ONLY $(CO_FLAGS) tools/control_observer_diagnostic.c tests/test_control_observer.c $(CO_CORE) -Wl,-Map,build/control_observer.map -o build/control_observer.prg

test: control_observer-host
sanitize: control_observer-sanitize
