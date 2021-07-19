/*
 * LoongArch cpu parameters for qemu.
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef LOONGARCH_CPU_PARAM_H
#define LOONGARCH_CPU_PARAM_H 1

#ifdef TARGET_LOONGARCH64
#define TARGET_LONG_BITS 64
#define TARGET_PHYS_ADDR_SPACE_BITS 48
#define TARGET_VIRT_ADDR_SPACE_BITS 48
#endif

#define TARGET_PAGE_BITS 12
#define NB_MMU_MODES 4

#endif
