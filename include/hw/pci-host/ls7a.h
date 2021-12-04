/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * QEMU LoongArch CPU
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 */

#ifndef HW_LS7A_H
#define HW_LS7A_H

#include "hw/pci/pci.h"
#include "hw/pci/pcie_host.h"
#include "hw/pci-host/pam.h"
#include "qemu/units.h"
#include "qemu/range.h"
#include "qom/object.h"

#define HT1LO_PCICFG_BASE        0x1a000000
#define HT1LO_PCICFG_SIZE        0x02000000

#define LS_PCIECFG_BASE          0x20000000
#define LS_PCIECFG_SIZE          0x08000000

#define LS7A_PCI_IO_BASE         0x18000000UL
#define LS7A_PCI_IO_SIZE         0x00010000

struct LS7APCIState {
    /*< private >*/
    PCIDevice parent_obj;
    /*< public >*/
};

typedef struct LS7APCIState LS7APCIState;
typedef struct LS7APCIEHost {
    /*< private >*/
    PCIExpressHost parent_obj;
    /*< public >*/

    LS7APCIState pci_dev;

    MemoryRegion pci_conf;
    MemoryRegion pci_io;
} LS7APCIEHost;

#define TYPE_LS7A_HOST_DEVICE "ls7a1000-pciehost"
OBJECT_DECLARE_SIMPLE_TYPE(LS7APCIEHost, LS7A_HOST_DEVICE)

#define TYPE_LS7A_PCIE "ls7a1000_pcie"
OBJECT_DECLARE_SIMPLE_TYPE(LS7APCIState, LS7A_PCIE)

#endif /* HW_LS7A_H */
