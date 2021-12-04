/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * QEMU loongson 3a5000 develop board emulation
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
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
#include "hw/loader.h"
#include "elf.h"
#include "hw/irq.h"
#include "net/net.h"
#include "sysemu/runstate.h"
#include "sysemu/reset.h"
#include "hw/loongarch/loongarch.h"
#include "hw/intc/loongarch_ipi.h"
#include "hw/intc/loongarch_extioi.h"
#include "hw/intc/loongarch_pch_pic.h"
#include "hw/intc/loongarch_pch_msi.h"
#include "hw/pci-host/ls7a.h"
#include "hw/misc/unimp.h"
#include "hw/loongarch/fw_cfg.h"
#include "hw/firmware/smbios.h"

#define LOONGSON3_BIOSNAME "loongarch_bios.bin"

static struct _loaderparams {
    unsigned long ram_size;
    const char *kernel_filename;
    const char *kernel_cmdline;
    const char *initrd_filename;
} loaderparams;

static uint64_t cpu_loongarch_virt_to_phys(void *opaque, uint64_t addr)
{
    return addr & 0x1fffffffll;
}

static void fw_cfg_add_kernel_info(FWCfgState *fw_cfg)
{
    int64_t kernel_entry, kernel_low, kernel_high, initrd_size = 0;
    long kernel_size;
    ram_addr_t initrd_offset = 0;
    void *cmdline_buf;
    int ret = 0;

    kernel_size = load_elf(loaderparams.kernel_filename, NULL,
                           cpu_loongarch_virt_to_phys, NULL,
                           (uint64_t *)&kernel_entry, (uint64_t *)&kernel_low,
                           (uint64_t *)&kernel_high, NULL, 0,
                           EM_LOONGARCH, 1, 0);

    if (kernel_size < 0) {
        error_report("could not load kernel '%s': %s",
                     loaderparams.kernel_filename,
                     load_elf_strerror(kernel_size));
        exit(1);
    }

    fw_cfg_add_i64(fw_cfg, FW_CFG_KERNEL_ENTRY, kernel_entry);

    if (loaderparams.initrd_filename) {
        initrd_size = get_image_size(loaderparams.initrd_filename);

        if (initrd_size > 0) {
            initrd_offset = MAX(INITRD_BASE,
                                ROUND_UP(kernel_high, INITRD_PAGE_SIZE));
            if (initrd_offset + initrd_size > 0x10000000) {
                error_report("ramdisk '%s' is too big",
                             loaderparams.initrd_filename);
                exit(1);
            }
            initrd_size = load_image_targphys(loaderparams.initrd_filename,
                                              initrd_offset,
                                              loaderparams.ram_size - initrd_offset);
        }
        if (initrd_size == (target_ulong) -1) {
            error_report("could not load initial ram disk '%s'",
                         loaderparams.initrd_filename);
            exit(1);
        }
    }

    cmdline_buf = g_malloc0(COMMAND_LINE_SIZE);
    if (initrd_size > 0)
        ret = (1 + snprintf(cmdline_buf, COMMAND_LINE_SIZE,
                "initrd=0x%lx,%li %s", initrd_offset,
                initrd_size, loaderparams.kernel_cmdline));
    else
        ret = (1 + snprintf(cmdline_buf, COMMAND_LINE_SIZE, "%s",
                loaderparams.kernel_cmdline));

    fw_cfg_add_i32(fw_cfg, FW_CFG_CMDLINE_SIZE, ret);
    fw_cfg_add_string(fw_cfg, FW_CFG_CMDLINE_DATA, (const char *)cmdline_buf);
}

static void loongarch_build_smbios(LoongArchMachineState *lams)
{
    MachineState *ms = MACHINE(lams);
    MachineClass *mc = MACHINE_GET_CLASS(lams);
    uint8_t *smbios_tables, *smbios_anchor;
    size_t smbios_tables_len, smbios_anchor_len;
    const char *product = "QEMU Virtual Machine";

    if (!lams->fw_cfg) {
        return;
    }

    product = "Loongson-3A5000-7A1000-TCG";

    smbios_set_defaults("QEMU", product, mc->name, false,
                        true, SMBIOS_ENTRY_POINT_30);

    smbios_get_tables(ms, NULL, 0, &smbios_tables, &smbios_tables_len,
                      &smbios_anchor, &smbios_anchor_len, &error_fatal);

    if (smbios_anchor) {
        fw_cfg_add_file(lams->fw_cfg, "etc/smbios/smbios-tables",
                        smbios_tables, smbios_tables_len);
        fw_cfg_add_file(lams->fw_cfg, "etc/smbios/smbios-anchor",
                        smbios_anchor, smbios_anchor_len);
    }
}

static
void loongarch_machine_done(Notifier *notifier, void *data)
{
    LoongArchMachineState *lams = container_of(notifier,
                                        LoongArchMachineState, machine_done);
    loongarch_build_smbios(lams);
}

static void loongarch_cpu_reset(void *opaque)
{
    LoongArchCPU *cpu = opaque;

    cpu_reset(CPU(cpu));
}

#define LOONGARCH_SIMPLE_MMIO_OPS(ADDR, NAME, SIZE) \
({\
     MemoryRegion *iomem = g_new(MemoryRegion, 1);\
     memory_region_init_io(iomem, NULL, &loongarch_qemu_ops,\
                           (void *)ADDR, NAME, SIZE);\
     memory_region_add_subregion(&lams->system_iocsr, ADDR, iomem);\
})

static void loongarch_qemu_write(void *opaque, hwaddr addr,
                                 uint64_t val, unsigned size)
{
}

static uint64_t loongarch_qemu_read(void *opaque, hwaddr addr, unsigned size)
{
    uint64_t feature = 0UL;
    addr = ((hwaddr)(long)opaque) + addr;

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
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
};

static void sysbus_mmio_map_loongarch(SysBusDevice *dev, int n,
                                      hwaddr addr, MemoryRegion *iocsr)
{
    assert(n >= 0 && n < dev->num_mmio);

    if (dev->mmio[n].addr == addr) {
        /* ??? region already mapped here. */
        return;
    }
    if (dev->mmio[n].addr != (hwaddr)-1) {
        /* Unregister previous mapping. */
        memory_region_del_subregion(iocsr, dev->mmio[n].memory);
    }
    dev->mmio[n].addr = addr;
    memory_region_add_subregion(iocsr, addr, dev->mmio[n].memory);
}

static PCIBus *loongson3_irq_init(MachineState *machine)
{
    LoongArchMachineState *lams = LOONGARCH_MACHINE(machine);
    DeviceState *ipi, *extioi, *pch_pic, *pch_msi, *cpudev, *pciehost;
    SysBusDevice *d;
    PCIBus *pci_bus;
    int cpu, pin, i;
    unsigned long ipi_addr;

    ipi = qdev_new(TYPE_LOONGARCH_IPI);
    d = SYS_BUS_DEVICE(ipi);
    sysbus_realize_and_unref(d, &error_fatal);
    for (cpu = 0; cpu < machine->smp.cpus; cpu++) {
        cpudev = DEVICE(qemu_get_cpu(cpu));
        ipi_addr = SMP_IPI_MAILBOX + cpu * 0x100;
        sysbus_mmio_map_loongarch(d, cpu, ipi_addr, &lams->system_iocsr);
        qdev_connect_gpio_out(ipi, cpu, qdev_get_gpio_in(cpudev, IRQ_IPI));
    }

    extioi = qdev_new(TYPE_LOONGARCH_EXTIOI);
    d = SYS_BUS_DEVICE(extioi);
    sysbus_realize_and_unref(d, &error_fatal);
    sysbus_mmio_map_loongarch(d, 0, APIC_BASE, &lams->system_iocsr);

    for (i = 0; i < EXTIOI_IRQS; i++) {
        sysbus_connect_irq(d, i, qdev_get_gpio_in(extioi, i));
    }

    /*
     * connect ext irq to the cpu irq
     * cpu_pin[9:2] <= intc_pin[7:0]
     */
    for (cpu = 0; cpu < machine->smp.cpus; cpu++) {
        cpudev = DEVICE(qemu_get_cpu(cpu));
        for (pin = 0; pin < LS3A_INTC_IP; pin++) {
            qdev_connect_gpio_out(extioi, (cpu * 8 + pin),
                                  qdev_get_gpio_in(cpudev, pin + 2));
        }
    }

    pch_pic = qdev_new(TYPE_LOONGARCH_PCH_PIC);
    d = SYS_BUS_DEVICE(pch_pic);
    sysbus_realize_and_unref(d, &error_fatal);
    sysbus_mmio_map(d, 0, LS7A_IOAPIC_REG_BASE);

    serial_mm_init(get_system_memory(), LS7A_UART_BASE, 0,
                   qdev_get_gpio_in(pch_pic, LS7A_UART_IRQ - PCH_PIC_IRQ_OFFSET),
                   115200, serial_hd(0), DEVICE_LITTLE_ENDIAN);

    sysbus_create_simple("ls7a_rtc", LS7A_RTC_REG_BASE,
                         qdev_get_gpio_in(pch_pic, LS7A_RTC_IRQ - PCH_PIC_IRQ_OFFSET));

    /* Connect 64 pch_pic irqs to extioi */
    for (int i = 0; i < PCH_PIC_IRQ_NUM; i++) {
        sysbus_connect_irq(d, i, qdev_get_gpio_in(extioi, i));
    }

    pch_msi = qdev_new(TYPE_LOONGARCH_PCH_MSI);
    d = SYS_BUS_DEVICE(pch_msi);
    sysbus_realize_and_unref(d, &error_fatal);
    sysbus_mmio_map(d, 0, LS7A_PCH_MSI_ADDR_LOW);
    for (i = 0; i < PCH_MSI_IRQ_NUM; i++) {
        /* Connect 192 pch_msi irqs to extioi */
        sysbus_connect_irq(d, i,
                           qdev_get_gpio_in(extioi, i + PCH_MSI_IRQ_START));
    }

    pciehost = qdev_new(TYPE_LS7A_HOST_DEVICE);
    d = SYS_BUS_DEVICE(pciehost);
    sysbus_realize_and_unref(d, &error_fatal);
    pci_bus = PCI_HOST_BRIDGE(pciehost)->bus;

    /* Connect 48 pci irq to extioi */
    for (i = 0; i < LS7A_PCI_IRQS; i++) {
        qdev_connect_gpio_out(pciehost, i,
                              qdev_get_gpio_in(extioi, i + LS7A_DEVICE_IRQS));
    }

    return pci_bus;
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

static void loongson3_init(MachineState *machine)
{
    const char *cpu_model = machine->cpu_type;
    const char *kernel_filename = machine->kernel_filename;
    const char *kernel_cmdline = machine->kernel_cmdline;
    const char *initrd_filename = machine->initrd_filename;
    LoongArchCPU *cpu;
    ram_addr_t offset = 0;
    ram_addr_t ram_size = machine->ram_size;
    uint64_t highram_size = 0;
    MemoryRegion *address_space_mem = get_system_memory();
    LoongArchMachineState *lams = LOONGARCH_MACHINE(machine);
    int i;
    PCIBus *pci_bus = NULL;
    int bios_size;
    char *filename;

    if (!cpu_model) {
        cpu_model = LOONGARCH_CPU_TYPE_NAME("Loongson-3A5000");
    }
    if (!strstr(cpu_model, "Loongson-3A5000")) {
        error_report("LoongArch/TCG needs cpu type Loongson-3A5000");
        exit(1);
    }

    memory_region_init_io(&lams->system_iocsr, NULL, NULL,
                          lams, "iocsr", UINT64_MAX);
    address_space_init(&lams->address_space_iocsr,
                       &lams->system_iocsr, "IOCSR");

    /* Init CPUs */
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
        qemu_register_reset(loongarch_cpu_reset, cpu);
    }

    if (ram_size < 1 * GiB) {
        error_report("ram_size must be greater than 1G due to the bios memory layout");
        exit(1);
    }

    memory_region_init_alias(&lams->lowmem, NULL, "loongarch.lowram",
                             machine->ram, 0, 256 * MiB);
    memory_region_add_subregion(address_space_mem, offset, &lams->lowmem);
    offset += 256 * MiB;

    highram_size = ram_size - 256 * MiB;
    memory_region_init_alias(&lams->highmem, NULL, "loongarch.highmem",
                             machine->ram, offset, highram_size);
    memory_region_add_subregion(address_space_mem, 0x90000000, &lams->highmem);
    offset += highram_size;

    /* load the BIOS image. */
    filename = qemu_find_file(QEMU_FILE_TYPE_BIOS,
                              machine->firmware ?: LOONGSON3_BIOSNAME);
    if (filename) {
        bios_size = load_image_targphys(filename, LA_BIOS_BASE, LA_BIOS_SIZE);
        lams->fw_cfg = loongarch_fw_cfg_init(ram_size, machine);
        rom_set_fw(lams->fw_cfg);
        g_free(filename);
    } else {
        bios_size = -1;
    }

    if ((bios_size < 0 || bios_size > LA_BIOS_SIZE) && !qtest_enabled()) {
        error_report("Could not load LOONGARCH bios '%s'", machine->firmware);
        exit(1);
    }

    if (kernel_filename) {
        loaderparams.ram_size = ram_size;
        loaderparams.kernel_filename = kernel_filename;
        loaderparams.kernel_cmdline = kernel_cmdline;
        loaderparams.initrd_filename = initrd_filename;
        fw_cfg_add_kernel_info(lams->fw_cfg);
    }

    memory_region_init_ram(&lams->bios, NULL, "loongarch.bios",
                           LA_BIOS_SIZE, &error_fatal);
    memory_region_set_readonly(&lams->bios, true);
    memory_region_add_subregion(get_system_memory(), LA_BIOS_BASE, &lams->bios);

    lams->machine_done.notify = loongarch_machine_done;
    qemu_add_machine_init_done_notifier(&lams->machine_done);

    /*
     * There are some invalid guest memory access.
     * Create some unimplemented devices to emulate this.
     */
    create_unimplemented_device("ls7a-lpc", 0x10002000, 0x14);
    create_unimplemented_device("pci-dma-cfg", 0x1001041c, 0x4);
    create_unimplemented_device("node-bridge", 0xEFDFB000274, 0x4);
    create_unimplemented_device("ls7a-lionlpc", 0x1fe01400, 0x38);
    create_unimplemented_device("ls7a-node0", 0x0EFDFB000274, 0x4);

    /* Initialize the IO interrupt subsystem */
    pci_bus = loongson3_irq_init(machine);

    /* Network card */
    network_init(pci_bus);

    /* VGA setup. Don't bother loading the bios. */
    pci_vga_init(pci_bus);

    pci_create_simple(pci_bus, -1, "pci-ohci");

    LOONGARCH_SIMPLE_MMIO_OPS(FEATURE_REG, "loongarch_feature", 0x8);
    LOONGARCH_SIMPLE_MMIO_OPS(VENDOR_REG, "loongarch_vendor", 0x8);
    LOONGARCH_SIMPLE_MMIO_OPS(CPUNAME_REG, "loongarch_cpuname", 0x8);
}

static void loongarch_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "Loongson-3A5000 LS7A1000 machine";
    mc->init = loongson3_init;
    mc->default_ram_size = 1 * GiB;
    mc->default_cpu_type = LOONGARCH_CPU_TYPE_NAME("Loongson-3A5000");
    mc->default_ram_id = "loongarch.ram";
    mc->max_cpus = LOONGARCH_MAX_VCPUS;
    mc->is_default = 1;
    mc->default_machine_opts = "firmware=loongarch_bios.bin";
    mc->default_kernel_irqchip_split = false;
    mc->block_default_type = IF_VIRTIO;
    mc->default_boot_order = "c";
    mc->no_cdrom = 1;
}

static const TypeInfo loongarch_machine_types[] = {
    {
        .name           = TYPE_LOONGARCH_MACHINE,
        .parent         = TYPE_MACHINE,
        .instance_size  = sizeof(LoongArchMachineState),
        .class_init     = loongarch_class_init,
    }
};

DEFINE_TYPES(loongarch_machine_types)
