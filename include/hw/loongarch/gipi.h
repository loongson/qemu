/*
 * Loongarch ipi interrupt header files
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef HW_LOONGARCH_GIPI_H
#define HW_LOONGARCH_GIPI_H

#define SMP_GIPI_MAILBOX                0x1fe01000ULL
#define CORE_STATUS_OFF       0x000
#define CORE_EN_OFF           0x004
#define CORE_SET_OFF          0x008
#define CORE_CLEAR_OFF        0x00c
#define CORE_BUF_20           0x020
#define CORE_BUF_28           0x028
#define CORE_BUF_30           0x030
#define CORE_BUF_38           0x038

#define MAX_GIPI_CORE_NUM      4
#define MAX_GIPI_MBX_NUM       4

typedef struct gipi_core {
    uint32_t status;
    uint32_t en;
    uint32_t set;
    uint32_t clear;
    uint64_t buf[MAX_GIPI_MBX_NUM];
    qemu_irq irq;
} gipi_core;

typedef struct gipiState {
    gipi_core core[MAX_GIPI_CORE_NUM];
} gipiState;

#endif
