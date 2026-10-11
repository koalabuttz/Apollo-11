/* SPDX-License-Identifier: MIT */
#ifndef CM_DISPATCH_CHECK_H
#define CM_DISPATCH_CHECK_H
#if defined(CM_CHECK_BASIC)
#define CM_CHECK_COUNT 109u
#elif defined(CM_CHECK_LIFECYCLE)
#define CM_CHECK_COUNT 115u
#elif defined(CM_CHECK_INTERPRETER)
#define CM_CHECK_COUNT 512u
#elif defined(CM_CHECK_MIXED)
#define CM_CHECK_COUNT 140u
#else
#define CM_CHECK_ALL
#define CM_CHECK_COUNT 876u
#endif
unsigned cm_dispatch_run_checks(void);
#endif
