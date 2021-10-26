/*
 * Loongarch 7A1000 I/O interrupt controller definitions
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define TYPE_LOONGARCH_PCH_MSI "loongarch_pch_msi"
DECLARE_INSTANCE_CHECKER(struct loongarch_pch_msi, LOONGARCH_PCH_MSI,
                         TYPE_LOONGARCH_PCH_MSI)

typedef struct loongarch_pch_msi {
    SysBusDevice parent_obj;
    qemu_irq pch_msi_irq[224];
    MemoryRegion msi_mmio;
} loongarch_pch_msi;
