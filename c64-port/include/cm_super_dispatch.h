/* SPDX-License-Identifier: MIT */
#ifndef CM_SUPER_DISPATCH_H
#define CM_SUPER_DISPATCH_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Original public SUPDXCHZ05227/+1(05230) and actual GOTOV5604622 caller.
 * A/L contain its normal source 2CADR (location then BBCON, reversed at +1).
 * Location is ordinary compiled storage0100..07777, BBCON bits076167 only.
 * Source PC and hardware banks must be coherent, ZERO0; IRQ/SUPER legal.
 * No callback: next is the actual callee tuple for the resident dispatcher.
 * Unsupported input preserves image/io/next. Caller excludes producers. */
enum exec_status cm_super_dispatch(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
