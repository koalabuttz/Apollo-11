PASSIVE_MOS_CC ?= mos-c64-clang
PASSIVE_C1541 ?= c1541
PASSIVE_VICE ?= x64sc
PASSIVE_ROM_DIR ?= /workspace/toolchains/roms/C64
PASSIVE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
.PHONY: passive-storage-host passive-storage-target passive-storage-allocation
passive-storage-host:
	python3 tools/passive_storage_audit.py
passive-storage-target:
	python3 tools/passive_storage_build.py --mission lm --cc $(PASSIVE_MOS_CC) --c1541 $(PASSIVE_C1541)
	python3 tools/passive_storage_vice_test.py --mission lm --vice $(PASSIVE_VICE) --rom-dir $(PASSIVE_ROM_DIR) --data-dir $(PASSIVE_DATA_DIR)
	python3 tools/passive_storage_build.py --mission cm --cc $(PASSIVE_MOS_CC) --c1541 $(PASSIVE_C1541)
	python3 tools/passive_storage_vice_test.py --mission cm --vice $(PASSIVE_VICE) --rom-dir $(PASSIVE_ROM_DIR) --data-dir $(PASSIVE_DATA_DIR)
passive-storage-allocation:
	python3 tools/passive_storage_allocate.py --c1541 $(PASSIVE_C1541)
