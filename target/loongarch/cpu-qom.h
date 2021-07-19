/*
 * QEMU LoongArch CPU
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef QEMU_LOONGARCH_CPU_QOM_H
#define QEMU_LOONGARCH_CPU_QOM_H

#include "hw/core/cpu.h"
#include "qom/object.h"

#ifdef TARGET_LOONGARCH64
#define TYPE_LOONGARCH_CPU "loongarch64-cpu"
#else
#error LoongArch 32bit emulation is not implemented yet
#endif

OBJECT_DECLARE_TYPE(LoongArchCPU, LoongArchCPUClass,
                    LOONGARCH_CPU)

/**
 * LoongArchCPUClass:
 * @parent_realize: The parent class' realize handler.
 * @parent_reset: The parent class' reset handler.
 *
 * A LoongArch CPU model.
 */
struct LoongArchCPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    DeviceRealize parent_realize;
    DeviceReset parent_reset;
};

#endif
