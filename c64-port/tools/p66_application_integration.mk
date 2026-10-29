# SPDX-License-Identifier: MIT
include tools/p66_application.mk
include tools/p66_application_port_host.mk
all test: p66-application-port-host
sanitize: p66-application-port-host-sanitize
