/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * QEMU GMCH/LS7A PCI PM Emulation
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#ifndef HW_ACPI_LS7A_H
#define HW_ACPI_LS7A_H

#include "hw/acpi/acpi.h"
#include "hw/acpi/cpu_hotplug.h"
#include "hw/acpi/cpu.h"
#include "hw/acpi/memory_hotplug.h"
#include "hw/acpi/acpi_dev_interface.h"
#include "hw/acpi/tco.h"

#define LS7A_ACPI_IO_BASE         0x800
#define LS7A_ACPI_IO_SIZE         0x100
#define LS7A_PM_EVT_BLK           (0x0C) /* 4 bytes */
#define LS7A_PM_CNT_BLK           (0x14) /* 2 bytes */
#define LS7A_GPE0_STS_REG         (0x28) /* 4 bytes */
#define LS7A_GPE0_ENA_REG         (0x2C) /* 4 bytes */
#define LS7A_GPE0_RESET_REG       (0x30) /* 4 bytes */
#define LS7A_PM_TMR_BLK           (0x18) /* 4 bytes */
#define LS7A_GPE0_LEN             (8)
#define ACPI_IO_BASE              (LS7A_ACPI_REG_BASE)
#define ACPI_GPE0_LEN             (LS7A_GPE0_LEN)
#define ACPI_IO_SIZE              (LS7A_ACPI_IO_SIZE)
#define ACPI_SCI_IRQ              (LS7A_SCI_IRQ)

typedef struct LS7APCIPMRegs {
    /*
     * In ls7a spec says that pm1_cnt register is 32bit width and
     * that the upper 16bits are reserved and unused.
     * PM1a_CNT_BLK = 2 in FADT so it is defined as uint16_t.
     */
    ACPIREGS acpi_regs;

    MemoryRegion iomem;
    MemoryRegion iomem_gpe;
    MemoryRegion iomem_reset;

    qemu_irq irq;      /* SCI */

    uint32_t pm_io_base;
    Notifier powerdown_notifier;
} LS7APCIPMRegs;

void ls7a_pm_init(PCIDevice *pci_device, LS7APCIPMRegs *ls7a,
                  DeviceState *pch_pic);
extern const VMStateDescription vmstate_ls7a_pm;
#endif /* HW_ACPI_LS7A_H */
