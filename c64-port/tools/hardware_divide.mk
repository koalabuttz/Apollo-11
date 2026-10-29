# SPDX-License-Identifier: MIT
HD_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
HD_CORE = src/agc_word.c src/agc_hardware_divide.c tests/hardware_divide_cases.c
hardware_divide-host:
	gcc -O2 -Wconversion -pedantic $(HD_FLAGS) tests/test_hardware_divide.c $(HD_CORE) -o build/test-hardware-divide
	build/test-hardware-divide
hardware_divide-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(HD_FLAGS) tools/hardware_divide_snapshot.c $(HD_CORE) -o build/hardware-divide-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/hardware-divide-native-sanitize > build/hardware-divide-sanitize.snapshots
	cmp build/hardware-divide-sanitize.snapshots build/hardware-divide-reference.snapshots
hardware_divide-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DHARDWARE_DIVIDE_CHECK_ONLY $(HD_FLAGS) tools/hardware_divide_diagnostic.c tests/test_hardware_divide.c $(HD_CORE) -Wl,-Map,build/hardware_divide.map -o build/hardware_divide.prg

test: hardware_divide-host
sanitize: hardware_divide-sanitize
