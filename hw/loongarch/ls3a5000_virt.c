/*
 * QEMU loongson 3a5000 develop board emulation
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "qemu/osdep.h"
#include "qemu-common.h"
#include "qemu/units.h"
#include "qemu/datadir.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/char/serial.h"
#include "sysemu/sysemu.h"
#include "sysemu/qtest.h"
#include "hw/irq.h"
#include "net/net.h"
#include "sysemu/runstate.h"
#include "sysemu/reset.h"
#include "hw/loongarch/loongarch.h"
#include "hw/intc/loongarch_extioi.h"
#include "hw/intc/loongarch_pch_pic.h"
#include "hw/intc/loongarch_pch_msi.h"
#include "hw/pci-host/ls7a.h"
#include "hw/misc/unimp.h"

CPULoongArchState *cpu_states[LOONGARCH_MAX_VCPUS];

static void main_cpu_reset(void *opaque)
{
    LoongArchCPU *cpu = opaque;

    cpu_reset(CPU(cpu));
}

static uint64_t loongarch_pm_mem_read(void *opaque, hwaddr addr, unsigned size)
{
    return 0;
}

static void loongarch_pm_mem_write(void *opaque, hwaddr addr,
                                   uint64_t val, unsigned size)
{

    if (addr != PM_CNT_MODE) {
        return;
    }

    switch (val) {
    case 0x00:
        qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
        return;
    case 0xff:
        qemu_system_shutdown_request(SHUTDOWN_CAUSE_GUEST_SHUTDOWN);
        return;
    default:
        return;
    }
}

static const MemoryRegionOps loongarch_pm_ops = {
    .read  = loongarch_pm_mem_read,
    .write = loongarch_pm_mem_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define LOONGARCH_SIMPLE_MMIO_OPS(ADDR, NAME, SIZE) \
({\
     MemoryRegion *iomem = g_new(MemoryRegion, 1);\
     memory_region_init_io(iomem, NULL, &loongarch_qemu_ops,\
                           (void *)ADDR, NAME, SIZE);\
     memory_region_add_subregion_overlap(address_space_mem, ADDR, iomem, 1);\
})

static void loongarch_qemu_write(void *opaque, hwaddr addr,
                                 uint64_t val, unsigned size)
{
}

static uint64_t loongarch_qemu_read(void *opaque, hwaddr addr, unsigned size)
{
    uint64_t feature = 0UL;
    addr = ((hwaddr)(long)opaque) + addr;
    addr = addr & 0xffffffff;
    switch (addr) {
    case FEATURE_REG:
        feature |= 1UL << IOCSRF_MSI | 1UL << IOCSRF_EXTIOI |
                   1UL << IOCSRF_CSRIPI;
        return feature ;
    case VENDOR_REG:
        return *(uint64_t *)"Loongson-3A5000";
    case CPUNAME_REG:
        return *(uint64_t *)"3A5000";
    }
    return 0;
}

static const MemoryRegionOps loongarch_qemu_ops = {
    .read = loongarch_qemu_read,
    .write = loongarch_qemu_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
};

static void ls3a5000_irq_init(MachineState *machine, CPULoongArchState *env[])
{
    LoongarchMachineState *lsms = LOONGARCH_MACHINE(machine);
    DeviceState *extioi, *pch_pic, *pch_msi;
    SysBusDevice *d;
    int cpu, pin, i;

    extioi = qdev_new(TYPE_LOONGARCH_EXTIOI);
    d = SYS_BUS_DEVICE(extioi);
    sysbus_realize_and_unref(d, &error_fatal);
    sysbus_mmio_map(d, 0, APIC_BASE);

    for (i = 0; i < EXTIOI_IRQS; i++) {
        sysbus_connect_irq(d, i, qdev_get_gpio_in(extioi, i));
    }

    for (cpu = 0; cpu < machine->smp.cpus; cpu++) {
        /* cpu_pin[9:2] <= intc_pin[7:0] */
        for (pin = 0; pin < LS3A_INTC_IP; pin++) {
            sysbus_connect_irq(d, (EXTIOI_IRQS + cpu * 8 + pin),
                               env[cpu]->irq[pin + 2]);
        }
    }

    pch_pic = qdev_new(TYPE_LOONGARCH_PCH_PIC);
    d = SYS_BUS_DEVICE(pch_pic);
    sysbus_realize_and_unref(d, &error_fatal);
    sysbus_mmio_map(d, 0, LS7A_IOAPIC_REG_BASE);

    serial_mm_init(get_system_memory(), LS7A_UART_BASE, 0,
                   qdev_get_gpio_in(pch_pic, LS7A_UART_IRQ - 64),
                   115200, serial_hd(0), DEVICE_NATIVE_ENDIAN);

    for (int i = 0; i < 32; i++) {
        sysbus_connect_irq(d, i, lsms->pch_irq[i]);
    }

    pch_msi = qdev_new(TYPE_LOONGARCH_PCH_MSI);
    d = SYS_BUS_DEVICE(pch_msi);
    sysbus_realize_and_unref(d, &error_fatal);
    sysbus_mmio_map(d, 0, LS7A_PCH_MSI_ADDR_LOW);
    for (i = 0; i < 224; i++) {
        sysbus_connect_irq(d, i, lsms->pch_irq[i + 32]);
    }

}

/* Network support */
static void network_init(PCIBus *pci_bus)
{
    int i;

    for (i = 0; i < nb_nics; i++) {
        NICInfo *nd = &nd_table[i];

        if (!nd->model) {
            nd->model = g_strdup("virtio");
        }

        pci_nic_init_nofail(nd, pci_bus, nd->model, NULL);
    }
}

static void ls3a5000_virt_init(MachineState *machine)
{
    const char *cpu_model = machine->cpu_type;
    LoongArchCPU *cpu;
    CPULoongArchState *env;
    uint64_t lowram_size = 0, highram_size = 0;
    MemoryRegion *lowmem = g_new(MemoryRegion, 1);
    char *ramName = NULL;
    ram_addr_t ram_size = machine->ram_size;
    MemoryRegion *address_space_mem = get_system_memory();
    LoongarchMachineState *lsms = LOONGARCH_MACHINE(machine);
    int i;
    MemoryRegion *iomem = NULL;
    PCIBus *pci_bus = NULL;

    if (!cpu_model) {
        cpu_model = LOONGARCH_CPU_TYPE_NAME("Loongson-3A5000");
    }
    if (!strstr(cpu_model, "Loongson-3A5000")) {
        error_report("Loongarch/TCG needs cpu type Loongson-3A5000");
        exit(1);
    }

    /* init CPUs */
    for (i = 0; i < machine->smp.cpus; i++) {
        Object *cpuobj = NULL;
        CPUState *cs;

        cpuobj = object_new(machine->cpu_type);

        cs = CPU(cpuobj);
        cs->cpu_index = i;

        qdev_realize(DEVICE(cpuobj), NULL, &error_fatal);
        object_unref(cpuobj);

        cpu = LOONGARCH_CPU(cs);
        if (cpu == NULL) {
            fprintf(stderr, "Unable to find CPU definition\n");
            exit(1);
        }
        env = &cpu->env;
        cpu_states[i] = env;

        /* Init CPU internal devices */
        cpu_loongarch_init_irq(cpu);
        cpu_loongarch_clock_init(cpu);
        cpu_init_ipi(lsms, env->irq[IRQ_IPI], i);
        qemu_register_reset(main_cpu_reset, cpu);
    }

    ramName = g_strdup_printf("loongarch.lowram");
    lowram_size = MIN(ram_size, 256 * 0x100000);
    memory_region_init_alias(lowmem, NULL, ramName, machine->ram,
                             0, lowram_size);
    memory_region_add_subregion(address_space_mem, 0, lowmem);

    highram_size = ram_size > lowram_size ? ram_size - 256 * 0x100000 : 0;
    if (highram_size > 0) {
        MemoryRegion *highmem = g_new(MemoryRegion, 1);
        ramName = g_strdup_printf("loongarch.highram");
        memory_region_init_alias(highmem, NULL, ramName, machine->ram,
                                 lowram_size, highram_size);
        memory_region_add_subregion(address_space_mem, 0x90000000, highmem);
    }

    /*Add PM mmio memory for reboot and shutdown*/
    iomem = g_new(MemoryRegion, 1);
    memory_region_init_io(iomem, NULL, &loongarch_pm_ops, NULL,
                          "loongarch_pm", PM_MMIO_SIZE);
    memory_region_add_subregion(address_space_mem,
                                PM_MMIO_ADDR, iomem);
    /*
     * There are some invalid guest memory access.
     * Create some unimplemented devices to emulate this.
     */
    create_unimplemented_device("ls7a-lpc", 0x10002000, 0x14);
    create_unimplemented_device("pci-dma-cfg", 0x1001041c, 0x4);
    create_unimplemented_device("node-bridge", 0xEFDFB000274, 0x4);
    create_unimplemented_device("ls7a-lionlpc", 0x1fe01400, 0x38);
    create_unimplemented_device("ls7a-node0", 0x0EFDFB000274, 0x4);
    create_unimplemented_device("ls7a-node1", 0x1EFDFB000274, 0x4);
    create_unimplemented_device("ls7a-node2", 0x2EFDFB000274, 0x4);
    create_unimplemented_device("ls7a-node3", 0x3EFDFB000274, 0x4);

    /*Initialize the IO interrupt subsystem*/
    ls3a5000_irq_init(machine, cpu_states);

    /* Init the north bridge */
    pci_bus = ls7a_init(machine, lsms->pch_irq);

    /*network card*/
    network_init(pci_bus);

    /* VGA setup.  Don't bother loading the bios.  */
    pci_vga_init(pci_bus);

    pci_create_simple(pci_bus, -1, "pci-ohci");

    LOONGARCH_SIMPLE_MMIO_OPS(FEATURE_REG, "loongarch_feature", 0x8);
    LOONGARCH_SIMPLE_MMIO_OPS(VENDOR_REG, "loongarch_vendor", 0x8);
    LOONGARCH_SIMPLE_MMIO_OPS(CPUNAME_REG, "loongarch_cpuname", 0x8);
    LOONGARCH_SIMPLE_MMIO_OPS(MISC_FUNC_REG, "loongarch_misc", 0x8);
    LOONGARCH_SIMPLE_MMIO_OPS(FREQ_REG, "loongarch_freq", 0x8);
}

static void loongarch_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "Loongson-5000 LS7A1000 machine";
    mc->init = ls3a5000_virt_init;
    mc->default_ram_size = 1 * GiB;
    mc->default_cpu_type = LOONGARCH_CPU_TYPE_NAME("Loongson-3A5000");
    mc->default_ram_id = "loongarch.ram";
    mc->max_cpus = LOONGARCH_MAX_VCPUS;
    mc->is_default = 1;
    mc->default_kernel_irqchip_split = false;
    mc->block_default_type = IF_VIRTIO;
    mc->default_boot_order = "c";
    mc->no_cdrom = 1;
}

static const TypeInfo loongarch_machine_types[] = {
    {
        .name           = TYPE_LOONGARCH_MACHINE,
        .parent         = TYPE_MACHINE,
        .instance_size  = sizeof(LoongarchMachineState),
        .class_init     = loongarch_class_init,
    }
};

DEFINE_TYPES(loongarch_machine_types)
