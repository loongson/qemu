/*
 * QEMU LoongArch CPU
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "qemu/qemu-print.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "sysemu/qtest.h"
#include "exec/exec-all.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "qapi/qapi-commands-machine-target.h"
#include "cpu.h"
#include "cpu-csr.h"
#include "cpu-qom.h"

static const char * const excp_names[EXCP_LAST + 1] = {
    [EXCP_INTE] = "Interrupt error",
    [EXCP_ADE] = "Address error",
    [EXCP_SYSCALL] = "Syscall",
    [EXCP_BREAK] = "Break",
    [EXCP_FPDIS] = "FPU Disabled",
    [EXCP_INE] = "Inst. Not Exist",
    [EXCP_TRAP] = "Trap",
    [EXCP_FPE] = "Floating Point Exception",
    [EXCP_TLBM] = "TLB modified fault",
    [EXCP_TLBL] = "TLB miss on a load",
    [EXCP_TLBS] = "TLB miss on a store",
    [EXCP_TLBPE] = "TLB Privilege Error",
    [EXCP_TLBXI] = "TLB Execution-Inhibit exception",
    [EXCP_TLBRI] = "TLB Read-Inhibit exception",
};

const char *loongarch_exception_name(int32_t exception)
{
    if (exception < 0 || exception > EXCP_LAST) {
        return "unknown";
    }
    return excp_names[exception];
}

target_ulong exception_resume_pc(CPULoongArchState *env)
{
    target_ulong bad_pc;

    bad_pc = env->active_tc.PC;

    return bad_pc;
}

void QEMU_NORETURN do_raise_exception_err(CPULoongArchState *env,
                                          uint32_t exception,
                                          int error_code,
                                          uintptr_t pc)
{
    CPUState *cs = env_cpu(env);

    qemu_log_mask(CPU_LOG_INT, "%s: %d (%s) %d\n",
                  __func__,
                  exception,
                  loongarch_exception_name(exception),
                  error_code);
    cs->exception_index = exception;
    env->error_code = error_code;

    cpu_loop_exit_restore(cs, pc);
}

static void loongarch_cpu_set_pc(CPUState *cs, vaddr value)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;

    env->active_tc.PC = value & ~(target_ulong)1;
}

bool loongarch_cpu_exec_interrupt(CPUState *cs, int interrupt_request)
{
    if (interrupt_request & CPU_INTERRUPT_HARD) {
        LoongArchCPU *cpu = LOONGARCH_CPU(cs);
        CPULoongArchState *env = &cpu->env;

        if (cpu_loongarch_hw_interrupts_enabled(env) &&
            cpu_loongarch_hw_interrupts_pending(env)) {
            cs->exception_index = EXCP_INTE;
            env->error_code = 0;
            loongarch_cpu_do_interrupt(cs);
            return true;
        }
    }
    return false;
}

void loongarch_cpu_do_interrupt(CPUState *cs)
{
    cs->exception_index = EXCP_NONE;
}

#ifdef CONFIG_TCG
static void loongarch_cpu_synchronize_from_tb(CPUState *cs,
                                              const TranslationBlock *tb)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;

    env->active_tc.PC = tb->pc;
    env->hflags &= ~LOONGARCH_HFLAG_BMASK;
    env->hflags |= tb->flags & LOONGARCH_HFLAG_BMASK;
}
#endif /* CONFIG_TCG */

static bool loongarch_cpu_has_work(CPUState *cs)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;
    bool has_work = false;

    if ((cs->interrupt_request & CPU_INTERRUPT_HARD) &&
        cpu_loongarch_hw_interrupts_pending(env)) {
            has_work = true;
    }

    return has_work;
}

static void set_loongarch_feature(CPULoongArchState *env, int feature)
{
    env->features |= (1ULL << feature);
}

static void set_loongarch_csr(CPULoongArchState *env)
{
    env->CSR_PRCFG1 = 0x72f8;
    env->CSR_PRCFG2 = 0x3ffff000;
    env->CSR_PRCFG3 = 0x8073f2;
    env->CSR_CRMD = 0xa8;
    env->CSR_ECFG = 0x70000;
    env->CSR_STLBPGSIZE = 0xe;
    env->CSR_RVACFG = 0x0;
    env->CSR_MCSR0 = 0x3f2f2fe0014c010;
    env->CSR_MCSR1 = 0xcff0060c3cf;
    env->CSR_MCSR2 = 0x1000105f5e100;
    env->CSR_MCSR3 = 0x0;
    env->CSR_MCSR8 = 0x608000300002c3d;
    env->CSR_MCSR9 = 0x608000f06080003;
    env->CSR_MCSR10 = 0x60f000f;
    env->CSR_MCSR24 = 0x0;
}

/* LoongArch CPU definitions */
static void loongarch_3a5000_initfn(Object *obj)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(obj);
    CPULoongArchState *env = &cpu->env;

    set_loongarch_feature(env, LA_FEATURE_3A5000);
    set_loongarch_csr(env);
}

static void loongarch_cpu_list_entry(gpointer data, gpointer user_data)
{
    const char *typename = object_class_get_name(OBJECT_CLASS(data));

    qemu_printf("%s\n", typename);
}

void loongarch_cpu_list(void)
{
    GSList *list;
    list = object_class_get_list_sorted(TYPE_LOONGARCH_CPU, false);
    g_slist_foreach(list, loongarch_cpu_list_entry, NULL);
    g_slist_free(list);
}

static void fpu_init(CPULoongArchState *env)
{
    memcpy(&env->active_fpu, &env->fpus[0], sizeof(env->active_fpu));
}

static void loongarch_cpu_reset(DeviceState *dev)
{
    CPUState *cs = CPU(dev);
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    LoongArchCPUClass *lacc = LOONGARCH_CPU_GET_CLASS(cpu);
    CPULoongArchState *env = &cpu->env;

    lacc->parent_reset(dev);

    memset(env, 0, offsetof(CPULoongArchState, end_reset_fields));

    set_loongarch_csr(env);
    env->current_tc = 0;
    env->active_fpu.fcsr0_mask = 0x1f1f03df;
    env->active_fpu.fcsr0 = 0x0;

    compute_hflags(env);
    cs->exception_index = EXCP_NONE;
}

static void loongarch_cpu_disas_set_info(CPUState *s, disassemble_info *info)
{
    info->print_insn = print_insn_loongarch;
}

static void loongarch_cpu_realizefn(DeviceState *dev, Error **errp)
{
    CPUState *cs = CPU(dev);
    LoongArchCPU *cpu = LOONGARCH_CPU(dev);
    CPULoongArchState *env = &cpu->env;
    LoongArchCPUClass *lacc = LOONGARCH_CPU_GET_CLASS(dev);
    Error *local_err = NULL;

    cpu_exec_realizefn(cs, &local_err);
    if (local_err != NULL) {
        error_propagate(errp, local_err);
        return;
    }

    env->exception_base = 0x1C000000;

    fpu_init(env);

    cpu_reset(cs);
    qemu_init_vcpu(cs);

    lacc->parent_realize(dev, errp);
}

static void loongarch_cpu_initfn(Object *obj)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(obj);

    cpu_set_cpustate_pointers(cpu);
    cpu->clock = qdev_init_clock_in(DEVICE(obj), "clk-in", NULL, cpu, 0);
}

static char *loongarch_cpu_type_name(const char *cpu_model)
{
    return g_strdup_printf(LOONGARCH_CPU_TYPE_NAME("%s"), cpu_model);
}

static ObjectClass *loongarch_cpu_class_by_name(const char *cpu_model)
{
    ObjectClass *oc;
    char *typename;

    typename = loongarch_cpu_type_name(cpu_model);
    oc = object_class_by_name(typename);
    g_free(typename);
    return oc;
}

static Property loongarch_cpu_properties[] = {
    DEFINE_PROP_INT32("core-id", LoongArchCPU, core_id, -1),
    DEFINE_PROP_UINT32("id", LoongArchCPU, id, UNASSIGNED_CPU_ID),
    DEFINE_PROP_INT32("node-id", LoongArchCPU, node_id, CPU_UNSET_NUMA_NODE_ID),
    DEFINE_PROP_END_OF_LIST()
};

#ifdef CONFIG_TCG
#include "hw/core/tcg-cpu-ops.h"

static struct TCGCPUOps loongarch_tcg_ops = {
    .initialize = loongarch_tcg_init,
    .synchronize_from_tb = loongarch_cpu_synchronize_from_tb,
    .cpu_exec_interrupt = loongarch_cpu_exec_interrupt,
};
#endif /* CONFIG_TCG */

static void loongarch_cpu_class_init(ObjectClass *c, void *data)
{
    LoongArchCPUClass *lacc = LOONGARCH_CPU_CLASS(c);
    CPUClass *cc = CPU_CLASS(c);
    DeviceClass *dc = DEVICE_CLASS(c);

    device_class_set_parent_realize(dc, loongarch_cpu_realizefn,
                                    &lacc->parent_realize);
    device_class_set_parent_reset(dc, loongarch_cpu_reset, &lacc->parent_reset);
    device_class_set_props(dc, loongarch_cpu_properties);

    cc->class_by_name = loongarch_cpu_class_by_name;
    cc->has_work = loongarch_cpu_has_work;
    cc->dump_state = loongarch_cpu_dump_state;
    cc->set_pc = loongarch_cpu_set_pc;
    cc->disas_set_info = loongarch_cpu_disas_set_info;
#ifdef CONFIG_TCG
    cc->tcg_ops = &loongarch_tcg_ops;
#endif
}

#define DEFINE_LOONGARCH_CPU_TYPE(model, initfn) \
    { \
        .parent = TYPE_LOONGARCH_CPU, \
        .instance_init = initfn, \
        .name = LOONGARCH_CPU_TYPE_NAME(model), \
    }

static const TypeInfo loongarch_cpu_type_infos[] = {
    {
        .name = TYPE_LOONGARCH_CPU,
        .parent = TYPE_CPU,
        .instance_size = sizeof(LoongArchCPU),
        .instance_init = loongarch_cpu_initfn,
        .abstract = true,
        .class_size = sizeof(LoongArchCPUClass),
        .class_init = loongarch_cpu_class_init,
    },
    DEFINE_LOONGARCH_CPU_TYPE("Loongson-3A5000", loongarch_3a5000_initfn),
};

DEFINE_TYPES(loongarch_cpu_type_infos)

static void loongarch_cpu_add_definition(gpointer data, gpointer user_data)
{
    ObjectClass *oc = data;
    CpuDefinitionInfoList **cpu_list = user_data;
    CpuDefinitionInfo *info;
    const char *typename;

    typename = object_class_get_name(oc);
    info = g_malloc0(sizeof(*info));
    info->name = g_strndup(typename,
                           strlen(typename) - strlen("-" TYPE_LOONGARCH_CPU));
    info->q_typename = g_strdup(typename);

    QAPI_LIST_PREPEND(*cpu_list, info);
}

CpuDefinitionInfoList *qmp_query_cpu_definitions(Error **errp)
{
    CpuDefinitionInfoList *cpu_list = NULL;
    GSList *list;

    list = object_class_get_list(TYPE_LOONGARCH_CPU, false);
    g_slist_foreach(list, loongarch_cpu_add_definition, &cpu_list);
    g_slist_free(list);

    return cpu_list;
}
