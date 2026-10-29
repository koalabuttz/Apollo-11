/* SPDX-License-Identifier: MIT */
#ifndef EXECUTIVE_SERVICE_CHECK_H
#define EXECUTIVE_SERVICE_CHECK_H
#include "executive_service_cases.h"
uint32_t exec_service_digest(const exec_service_case *input,
                            enum exec_status status,exec_transfer transfer);
int exec_service_check(unsigned number);
#endif
