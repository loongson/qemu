/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * LoongArch emulation helpers for iocsr read/write
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 */

#include "qemu/osdep.h"
#include "qemu/main-loop.h"
#include "cpu.h"
#include "internals.h"
#include "qemu/host-utils.h"
#include "exec/helper-proto.h"
#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "hw/irq.h"
#include "cpu-csr.h"
#include "hw/loongarch/loongarch.h"
#include "tcg/tcg-ldst.h"

/*
 * For per core address 0x10xx(IPI) 0x18xx(EXTIOI)
 * need extra adjust the iocsr addr.
 */
uint64_t helper_iocsr_read(CPULoongArchState *env, target_ulong r_addr,
                           uint32_t size)
{
    LoongArchMachineState *lams = LOONGARCH_MACHINE(qdev_get_machine());
    int cpuid = env_cpu(env)->cpu_index;

    if (((r_addr & 0xff00) == 0x1000) || ((r_addr & 0xff00) == 0x1800)) {
        r_addr = r_addr + ((target_ulong)(cpuid & 0x3) << 8);
    }

    if (size == 1) {
        return address_space_ldub(&lams->address_space_iocsr, r_addr,
                                  MEMTXATTRS_UNSPECIFIED, NULL);
    } else if (size == 2) {
        return address_space_lduw(&lams->address_space_iocsr, r_addr,
                                  MEMTXATTRS_UNSPECIFIED, NULL);
    } else if (size == 4) {
        return address_space_ldl(&lams->address_space_iocsr, r_addr,
                                 MEMTXATTRS_UNSPECIFIED, NULL);
    } else if (size == 8) {
        return address_space_ldq(&lams->address_space_iocsr, r_addr,
                                 MEMTXATTRS_UNSPECIFIED, NULL);
    }
    return 0;
}

void helper_iocsr_write(CPULoongArchState *env, target_ulong w_addr,
                        target_ulong val, uint32_t size)
{
    LoongArchMachineState *lams = LOONGARCH_MACHINE(qdev_get_machine());
    int cpuid = env_cpu(env)->cpu_index;
    int mask, i;

    /*
     * For IPI send, Mail send, ANY send adjust addr and val
     * according to their real meaning. The iocsr write
     * will finally lead to the corresponding mmio write
     * all operations handled there.
     */
    if (w_addr == 0x1040) { /* IPI send */
        cpuid = (val >> 16) & 0x3ff;
        val = 1UL << (val & 0x1f);
        w_addr = 0x1008;
    } else if (w_addr == 0x1048) { /* Mail Send */
        cpuid = (val >> 16) & 0x3ff;
        w_addr = 0x1020 + (val & 0x1c);
        val = val >> 32;
        mask = (val >> 27) & 0xf;
        size = 4;
    } else if (w_addr == 0x1158) { /* ANY send */
        cpuid = (val >> 16) & 0x3ff;
        w_addr = val & 0xffff;
        val = val >> 32;
        mask = (val >> 27) & 0xf;
        size = 1;

        for (i = 0; i < 4; i++) {
            if (!((mask >> i) & 1)) {
                address_space_stb(&lams->address_space_iocsr, w_addr,
                                  val, MEMTXATTRS_UNSPECIFIED, NULL);
            }
            w_addr = w_addr + 1;
            val = val >> 8;
        }
        return;
    }

    if (((w_addr & 0xff00) == 0x1000) || ((w_addr & 0xff00) == 0x1800)) {
        w_addr = w_addr + ((target_ulong)(cpuid & 0x3) << 8);
    }

    if (size == 1) {
        address_space_stb(&lams->address_space_iocsr, w_addr,
                          val, MEMTXATTRS_UNSPECIFIED, NULL);
    } else if (size == 2) {
        address_space_stw(&lams->address_space_iocsr, w_addr,
                          val, MEMTXATTRS_UNSPECIFIED, NULL);
    } else if (size == 4) {
        address_space_stl(&lams->address_space_iocsr, w_addr,
                          val, MEMTXATTRS_UNSPECIFIED, NULL);
    } else if (size == 8) {
        address_space_stq(&lams->address_space_iocsr, w_addr,
                          val, MEMTXATTRS_UNSPECIFIED, NULL);
    }
}
