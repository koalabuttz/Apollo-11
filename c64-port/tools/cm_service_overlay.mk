# SPDX-License-Identifier: MIT
CM_SERVICE_C1541 ?= c1541
.PHONY: c64-cm-service-overlay
c64-cm-service-overlay:
	python3 tools/cm_service_overlay_build.py --cc "$(MOS_CC)" --c1541 "$(CM_SERVICE_C1541)"
