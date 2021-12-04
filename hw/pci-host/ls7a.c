/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * QEMU Loongson 7A1000 North Bridge Emulation
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#include "qemu/osdep.h"

#include "hw/pci/pci.h"
#include "hw/pci/pcie_host.h"
#include "hw/qdev-properties.h"
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
        VMSTATE_PCI_DEVICE(parent_obj, LS7APCIState),
        VMSTATE_END_OF_LIST()
    }
};

static PCIINTxRoute ls7a_route_intx_pin_to_irq(void *opaque, int pin)
{
    PCIINTxRoute route;

    route.irq = pin;
    route.mode = PCI_INTX_ENABLED;
    return route;
}

static int pci_ls7a_map_irq(PCIDevice *d, int irq_num)
{
    PCIBus *bus;
    int offset, irq;

    bus = pci_get_bus(d);
    if (bus->parent_dev) {
        irq = pci_swizzle_map_irq_fn(d, irq_num);
        return irq;
    }

    /* pci device start from irq 80 */
    offset = PCH_PIC_IRQ_OFFSET + LS7A_DEVICE_IRQS;
    irq = offset + ((PCI_SLOT(d->devfn) * 4 + irq_num)) % LS7A_PCI_IRQS;

    return irq;
}

static void pci_ls7a_set_irq(void *opaque, int irq_num, int level)
{
    LS7APCIEHost *pciehost = opaque;
    int offset = PCH_PIC_IRQ_OFFSET + LS7A_DEVICE_IRQS;

    qemu_set_irq(pciehost->irqs[irq_num - offset], level);
}

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
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void ls7a_pciehost_realize(DeviceState *dev, Error **errp)
{
    PCIHostState *pci = PCI_HOST_BRIDGE(dev);
    LS7APCIEHost *s = LS7A_HOST_DEVICE(dev);
    PCIExpressHost *pex = PCIE_HOST_BRIDGE(dev);

    pci->bus = pci_register_root_bus(dev, "pcie.0", pci_ls7a_set_irq,
                                     pci_ls7a_map_irq, s,
                                     get_system_memory(), get_system_io(),
                                     PCI_DEVFN(1, 0), 128, TYPE_PCIE_BUS);

    pci_bus_set_route_irq_fn(pci->bus, ls7a_route_intx_pin_to_irq);

    memory_region_init_io(&s->pci_conf, OBJECT(dev),
                          &pci_ls7a_config_ops, pci->bus,
                          "ls7a_pci_conf", HT1LO_PCICFG_SIZE);
    memory_region_add_subregion(get_system_memory(), HT1LO_PCICFG_BASE,
                                &s->pci_conf);

    /* Add ls7a pci-io */
    memory_region_init_alias(&s->pci_io, OBJECT(dev), "ls7a-pci-io",
                             get_system_io(), 0, LS7A_PCI_IO_SIZE);
    memory_region_add_subregion(get_system_memory(), LS7A_PCI_IO_BASE,
                                &s->pci_io);

    pcie_host_mmcfg_update(pex, true, LS_PCIECFG_BASE, LS_PCIECFG_SIZE);
    qdev_realize(DEVICE(&s->pci_dev), BUS(pci->bus), &error_fatal);
}

static void ls7a_pcie_realize(PCIDevice *d, Error **errp)
{
    /* pci status */
    d->config[0x6] = 0x01;
    /* base class code */
    d->config[0xb] = 0x06;
    /* header type */
    d->config[0xe] = 0x80;
    /* capabilities pointer */
    d->config[0x34] = 0x40;
    /* link status and control register 0 */
    d->config[0x44] = 0x20;
}

static void ls7a_pcie_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);

    set_bit(DEVICE_CATEGORY_BRIDGE, dc->categories);
    dc->desc = "LS7A1000 PCIE Host bridge";
    dc->vmsd = &vmstate_ls7a_pcie;
    k->realize = ls7a_pcie_realize;
    k->vendor_id = 0x0014;
    k->device_id = 0x7a00;
    k->revision = 0x0;
    k->class_id = PCI_CLASS_BRIDGE_HOST;
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

static void ls7a_pciehost_initfn(Object *obj)
{
    LS7APCIEHost *s = LS7A_HOST_DEVICE(obj);
    LS7APCIState *ls7a_pci = &s->pci_dev;

    object_initialize_child(obj, "ls7a_pci", ls7a_pci, TYPE_LS7A_PCIE);
    qdev_prop_set_int32(DEVICE(ls7a_pci), "addr", PCI_DEVFN(0, 0));
    qdev_prop_set_bit(DEVICE(ls7a_pci), "multifunction", false);

    qdev_init_gpio_out(DEVICE(obj), s->irqs, LS7A_PCI_IRQS);
}

static const char *ls7a_pciehost_root_bus_path(PCIHostState *host_bridge,
                                          PCIBus *rootbus)
{
    return "0000:00";
}

static void ls7a_pciehost_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    PCIHostBridgeClass *hc = PCI_HOST_BRIDGE_CLASS(klass);

    hc->root_bus_path = ls7a_pciehost_root_bus_path;
    dc->realize = ls7a_pciehost_realize;
    set_bit(DEVICE_CATEGORY_BRIDGE, dc->categories);
    dc->fw_name = "pci";
    dc->user_creatable = false;
}

static const TypeInfo ls7a_pciehost_info = {
    .name          = TYPE_LS7A_HOST_DEVICE,
    .parent        = TYPE_PCIE_HOST_BRIDGE,
    .instance_size = sizeof(LS7APCIEHost),
    .instance_init = ls7a_pciehost_initfn,
    .class_init    = ls7a_pciehost_class_init,
};

static void ls7a_register_types(void)
{
    type_register_static(&ls7a_pciehost_info);
    type_register_static(&ls7a_pcie_device_info);
}

type_init(ls7a_register_types)
