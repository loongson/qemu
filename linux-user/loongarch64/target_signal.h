/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef LOONGARCH_TARGET_SIGNAL_H
#define LOONGARCH_TARGET_SIGNAL_H

/* this struct defines a stack used during syscall handling */
typedef struct target_sigaltstack {
        abi_long ss_sp;
        abi_int ss_flags;
        abi_ulong ss_size;
} target_stack_t;

/*
 * sigaltstack controls
 */
#define TARGET_SS_ONSTACK     1
#define TARGET_SS_DISABLE     2

#define TARGET_MINSIGSTKSZ    2048
#define TARGET_SIGSTKSZ       8192

#include "../generic/signal.h"

#endif /* LOONGARCH_TARGET_SIGNAL_H */
