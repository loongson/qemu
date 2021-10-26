/*
 * QEMU Loongson 7A1000 North Bridge Emulation
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"

#include "hw/pci/pci.h"
#include "hw/pci/pcie_host.h"
#include "qapi/error.h"
#include "hw/irq.h"
#include "hw/pci/pci_bridge.h"
#include "hw/pci/pci_bus.h"
#include "sysemu/reset.h"
#include "hw/pci-host/ls7a.h"
#include "migration/vmstate.h"

static const VMStateDescription vmstate_ls7a_pcie = {
    .name = "LS7A_PCIE",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_PCI_DEVICE(dev, LS7APCIState),
        VMSTATE_END_OF_LIST()
    }
};

static void pci_ls7a_config_write(void *opaque, hwaddr addr,
                                  uint64_t val, unsigned size)
{
    pci_data_write(opaque, addr, val, size);
}

static uint64_t pci_ls7a_config_read(void *opaque,
                                     hwaddr addr, unsigned size)
{
    uint64_t val;

    val = pci_data_read(opaque, addr, size);

    return val;
}

static const MemoryRegionOps pci_ls7a_config_ops = {
    .read = pci_ls7a_config_read,
    .write = pci_ls7a_config_write,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4,
    },
    .impl = {
        .min_access_size = 1,
        .max_access_size = 4,
    },
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void ls7a_pciehost_realize(DeviceState *dev, Error **errp)
{
    LS7APCIEHost *pciehost = LS7A_PCIE_HOST_BRIDGE(dev);
    PCIExpressHost *e = PCIE_HOST_BRIDGE(dev);
    PCIHostState *phb = PCI_HOST_BRIDGE(e);

    phb->bus = pci_register_root_bus(dev, "pcie.0", NULL,
                                     NULL, pciehost,
                                     get_system_memory(), get_system_io(),
                                     PCI_DEVFN(1, 0), 128, TYPE_PCIE_BUS);

    memory_region_init_io(&pciehost->pci_conf, OBJECT(dev),
                          &pci_ls7a_config_ops, phb->bus,
                          "ls7a_pci_conf", HT1LO_PCICFG_SIZE);
    memory_region_add_subregion(get_system_memory(), HT1LO_PCICFG_BASE,
                                &pciehost->pci_conf);

    /* Add ls7a pci-io */
    memory_region_init_alias(&pciehost->pci_io, OBJECT(dev), "ls7a-pci-io",
                             get_system_io(), 0, LS7A_PCI_IO_SIZE);
    memory_region_add_subregion(get_system_memory(), LS7A_PCI_IO_BASE,
                                &pciehost->pci_io);

    pcie_host_mmcfg_update(e, true, LS_PCIECFG_BASE, LS_PCIECFG_SIZE);
}

PCIBus *ls7a_init(MachineState *machine, qemu_irq *pic)
{
    DeviceState *dev;
    PCIHostState *phb;
    LS7APCIState *pbs;
    LS7APCIEHost *pciehost;
    PCIDevice *pci_dev;
    PCIExpressHost *e;

    dev = qdev_new(TYPE_LS7A_PCIE_HOST_BRIDGE);
    e = PCIE_HOST_BRIDGE(dev);
    phb = PCI_HOST_BRIDGE(e);
    pciehost = LS7A_PCIE_HOST_BRIDGE(dev);
    pciehost->pic = pic;

    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);

    pci_dev = pci_new(PCI_DEVFN(0, 0), TYPE_LS7A_PCIE);
    pbs = LS7A_PCIE(pci_dev);
    pbs->pciehost = pciehost;
    pbs->pciehost->pci_dev = pbs;

    pci_realize_and_unref(pci_dev, phb->bus, &error_fatal);

    return phb->bus;
}

static void ls7a_reset(DeviceState *qdev)
{
    uint64_t wmask;
    wmask = ~(-1);
    PCIDevice *dev = PCI_DEVICE(qdev);

    pci_set_word(dev->config + PCI_STATUS, 0x0010);
    pci_set_word(dev->wmask + PCI_STATUS, wmask & 0xffff);
    pci_set_word(dev->cmask + PCI_STATUS, 0xffff);
    pci_set_byte(dev->config + PCI_HEADER_TYPE, 0x1);
    pci_set_byte(dev->wmask + PCI_HEADER_TYPE, wmask & 0xff);
    pci_set_byte(dev->cmask + PCI_HEADER_TYPE, 0xff);
    pci_set_word(dev->config + PCI_SUBSYSTEM_VENDOR_ID, 0x0014);
    pci_set_word(dev->wmask + PCI_SUBSYSTEM_VENDOR_ID, wmask & 0xffff);
    pci_set_word(dev->cmask + PCI_SUBSYSTEM_VENDOR_ID, 0xffff);
    pci_set_word(dev->config + PCI_SUBSYSTEM_ID, 0x7a00);
    pci_set_word(dev->wmask + PCI_SUBSYSTEM_ID, wmask & 0xffff);
    pci_set_word(dev->cmask + PCI_SUBSYSTEM_ID, 0xffff);
    pci_set_byte(dev->config + PCI_CAPABILITY_LIST, 0x40);
    pci_set_byte(dev->wmask + PCI_CAPABILITY_LIST, wmask & 0xff);
    pci_set_byte(dev->cmask + PCI_CAPABILITY_LIST, 0xff);
}

static void ls7a_pcie_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);

    k->vendor_id = 0x0014;
    k->device_id = 0x7a00;
    k->revision = 0x00;
    k->class_id = PCI_CLASS_BRIDGE_HOST;
    dc->reset = ls7a_reset;
    dc->desc = "LS7A1000 PCIE Host bridge";
    dc->vmsd = &vmstate_ls7a_pcie;
    /*
     * PCI-facing part of the host bridge, not usable without the
     * host-facing part, which can't be device_add'ed, yet.
     */
    dc->user_creatable = false;
}

static const TypeInfo ls7a_pcie_device_info = {
    .name          = TYPE_LS7A_PCIE,
    .parent        = TYPE_PCI_DEVICE,
    .instance_size = sizeof(LS7APCIState),
    .class_init    = ls7a_pcie_class_init,
    .interfaces = (InterfaceInfo[]) {
        { INTERFACE_CONVENTIONAL_PCI_DEVICE },
        { },
    },
};

static void ls7a_pciehost_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = ls7a_pciehost_realize;
    dc->fw_name = "pci";
    dc->user_creatable = false;
}

static const TypeInfo ls7a_pciehost_info = {
    .name          = TYPE_LS7A_PCIE_HOST_BRIDGE,
    .parent        = TYPE_PCIE_HOST_BRIDGE,
    .instance_size = sizeof(LS7APCIEHost),
    .class_init    = ls7a_pciehost_class_init,
};

static void ls7a_register_types(void)
{
    type_register_static(&ls7a_pciehost_info);
    type_register_static(&ls7a_pcie_device_info);
}

type_init(ls7a_register_types)
