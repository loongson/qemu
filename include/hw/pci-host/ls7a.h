/*
 * LoongArch ACPI implementation
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef HW_LS7A_H
#define HW_LS7A_H

#include "hw/pci/pci.h"
#include "hw/pci/pcie_host.h"
#include "hw/pci-host/pam.h"
#include "hw/acpi/ls7a.h"
#include "qemu/units.h"
#include "qemu/range.h"
#include "qom/object.h"

#define HT1LO_PCICFG_BASE        0x1a000000
#define HT1LO_PCICFG_SIZE        0x02000000

#define LS_PCIECFG_BASE          0x20000000
#define LS_PCIECFG_SIZE          0x08000000

#define LS7A_PCI_IO_BASE        0x18000000UL
#define LS7A_PCI_IO_SIZE        0x00010000
#define LS7A_PCI_MEM_BASE       0x20000000
#define LS7A_PCI_MEM_SIZE       0x3fffffff

#define LS7A_PCH_REG_BASE       0x10000000UL
#define LS7A_IOAPIC_REG_BASE    (LS7A_PCH_REG_BASE)
#define LS7A_PCH_MSI_ADDR_LOW   0x2FF00000UL

#define LOONGARCH_PCH_IRQ_BASE  64
#define LS7A_UART_IRQ           (LOONGARCH_PCH_IRQ_BASE + 2)
#define LS7A_UART_BASE          0x1fe001e0
#define LS7A_RTC_IRQ            (LOONGARCH_PCH_IRQ_BASE + 3)
#define LS7A_MISC_REG_BASE      (LS7A_PCH_REG_BASE + 0x00080000)
#define LS7A_RTC_REG_BASE       (LS7A_MISC_REG_BASE + 0x00050100)
#define LS7A_RTC_LEN            0x100
#define LS7A_ACPI_REG_BASE      (LS7A_MISC_REG_BASE + 0x00050000)
#define LS7A_SCI_IRQ            (LOONGARCH_PCH_IRQ_BASE + 4)

typedef struct LS7APCIState LS7APCIState;
typedef struct LS7APCIEHost {
    PCIExpressHost parent_obj;
    LS7APCIState *pci_dev;
    qemu_irq *pic;
    MemoryRegion pci_conf;
    MemoryRegion pci_io;
} LS7APCIEHost;

struct LS7APCIState {
    PCIDevice dev;
    LS7APCIEHost *pciehost;
    LS7APCIPMRegs pm;
};

#define TYPE_LS7A_PCIE_HOST_BRIDGE "ls7a1000-pciehost"
OBJECT_DECLARE_SIMPLE_TYPE(LS7APCIEHost, LS7A_PCIE_HOST_BRIDGE)

#define TYPE_LS7A_PCIE "ls7a1000_pcie"
OBJECT_DECLARE_SIMPLE_TYPE(LS7APCIState, LS7A_PCIE)

PCIBus *ls7a_init(MachineState *machine, DeviceState *pch_pic, qemu_irq *irq);
#endif /* HW_LS7A_H */
