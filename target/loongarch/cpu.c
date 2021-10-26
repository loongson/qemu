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
#include "qapi/qapi-commands-machine-target.h"
#include "migration/vmstate.h"
#include "cpu.h"
#include "internals.h"
#include "fpu/softfloat-helpers.h"

const char * const regnames[] = {
    "r0", "ra", "tp", "sp", "a0", "a1", "a2", "a3",
    "a4", "a5", "a6", "a7", "t0", "t1", "t2", "t3",
    "t4", "t5", "t6", "t7", "t8", "x0", "fp", "s0",
    "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",
};

const char * const fregnames[] = {
    "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
    "f8",  "f9",  "f10", "f11", "f12", "f13", "f14", "f15",
    "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
    "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",
};

static const char * const excp_names[EXCP_LAST + 1] = {
    [EXCP_ADE] = "Address error",
    [EXCP_SYSCALL] = "Syscall",
    [EXCP_BREAK] = "Break",
    [EXCP_INE] = "Inst. Not Exist",
    [EXCP_FPE] = "Floating Point Exception",
    [EXCP_TLBL] = "TLB load",
    [EXCP_TLBS] = "TLB store",
    [EXCP_INST_NOTAVAIL] = "TLB inst not exist",
    [EXCP_TLBM] = "TLB modify",
    [EXCP_TLBPE] = "TLB priviledged error",
    [EXCP_TLBNX] = "TLB execute-inhibit",
    [EXCP_TLBNR] = "TLB read-inhibit",
};

const char *loongarch_exception_name(int32_t exception)
{
    if (exception < 0 || exception > EXCP_LAST) {
        return "unknown";
    }
    return excp_names[exception];
}

void QEMU_NORETURN do_raise_exception(CPULoongArchState *env,
                                      uint32_t exception,
                                      uintptr_t pc)
{
    CPUState *cs = env_cpu(env);

    qemu_log_mask(CPU_LOG_INT, "%s: %d (%s)\n",
                  __func__,
                  exception,
                  loongarch_exception_name(exception));
    cs->exception_index = exception;

    cpu_loop_exit_restore(cs, pc);
}

static void loongarch_cpu_set_pc(CPUState *cs, vaddr value)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;

    env->pc = value;
}

#ifdef CONFIG_TCG
static void loongarch_cpu_synchronize_from_tb(CPUState *cs,
                                              const TranslationBlock *tb)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;

    env->pc = tb->pc;
}
#endif /* CONFIG_TCG */

static bool loongarch_cpu_has_work(CPUState *cs)
{
    return true;
}

static void set_loongarch_cpucfg(CPULoongArchState *env)
{
    int i;

    for (i = 0; i < 49; i++) {
        env->cpucfg[i] = 0x0;
    }
    env->cpucfg[0] = 0x14c010;
    env->cpucfg[1] = 0x3f2f2fe;
    env->cpucfg[2] = 0x60c3cf;
    env->cpucfg[3] = 0xcff;
    env->cpucfg[4] = 0x5f5e100;
    env->cpucfg[5] = 0x10001;
    env->cpucfg[16] = 0x2c3d;
    env->cpucfg[17] = 0x6080003;
    env->cpucfg[18] = 0x6080003;
    env->cpucfg[19] = 0x60800f;
    env->cpucfg[20] = 0x60f000f;
}

static void set_loongarch_csr(CPULoongArchState *env)
{
    uint64_t t;
    CPUState *cs = env_cpu(env);

    t = FIELD_DP64(0, CSR_CRMD, PLV, 0);
    t = FIELD_DP64(t, CSR_CRMD, IE, 0);
    t = FIELD_DP64(t, CSR_CRMD, DA, 1);
    t = FIELD_DP64(t, CSR_CRMD, PG, 0);
    t = FIELD_DP64(t, CSR_CRMD, DATF, 1);
    t = FIELD_DP64(t, CSR_CRMD, DATM, 1);
    env->CSR_CRMD = t;

    env->CSR_STLBPS  = 0xe;
    env->CSR_ASID = 0xa0000;
    env->CSR_ERA = env->pc;
    env->CSR_CPUID = (cs->cpu_index & 0x1ff);
    env->CSR_TMID = cs->cpu_index;
}

/* LoongArch CPU definitions */
static void loongarch_3a5000_initfn(Object *obj)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(obj);
    CPULoongArchState *env = &cpu->env;

    set_loongarch_cpucfg(env);
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

static void loongarch_cpu_reset(DeviceState *dev)
{
    CPUState *cs = CPU(dev);
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    LoongArchCPUClass *lacc = LOONGARCH_CPU_GET_CLASS(cpu);
    CPULoongArchState *env = &cpu->env;

    lacc->parent_reset(dev);

    set_loongarch_cpucfg(env);
    set_loongarch_csr(env);
    env->fcsr0_mask = 0x1f1f031f;
    env->fcsr0 = 0x0;

    restore_fp_status(env);
    cs->exception_index = EXCP_NONE;
}

static void loongarch_cpu_disas_set_info(CPUState *s, disassemble_info *info)
{
    info->print_insn = print_insn_loongarch;
}

static void loongarch_cpu_realizefn(DeviceState *dev, Error **errp)
{
    CPUState *cs = CPU(dev);
#ifndef CONFIG_USER_ONLY
    LoongArchCPU *cpu = LOONGARCH_CPU(dev);
    CPULoongArchState *env = &cpu->env;
#endif
    LoongArchCPUClass *lacc = LOONGARCH_CPU_GET_CLASS(dev);
    Error *local_err = NULL;

    cpu_exec_realizefn(cs, &local_err);
    if (local_err != NULL) {
        error_propagate(errp, local_err);
        return;
    }

#ifndef CONFIG_USER_ONLY
    loongarch_mmu_init(env);
#endif

    cpu_reset(cs);
    qemu_init_vcpu(cs);

    lacc->parent_realize(dev, errp);
}

static void loongarch_cpu_initfn(Object *obj)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(obj);

    cpu_set_cpustate_pointers(cpu);
}

static ObjectClass *loongarch_cpu_class_by_name(const char *cpu_model)
{
    ObjectClass *oc;
    char *typename;

    typename = g_strdup_printf(LOONGARCH_CPU_TYPE_NAME("%s"), cpu_model);
    oc = object_class_by_name(typename);
    g_free(typename);
    return oc;
}

void loongarch_cpu_dump_state(CPUState *cs, FILE *f, int flags)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;
    int i;

    qemu_fprintf(f, " PC=%016" PRIx64 " ", env->pc);
    qemu_fprintf(f, " FCSR0 0x%08x  fp_status 0x%02x\n", env->fcsr0,
                 get_float_exception_flags(&env->fp_status));

    /* gpr */
    for (i = 0; i < 32; i++) {
        if ((i & 3) == 0) {
            qemu_fprintf(f, " GPR%02d:", i);
        }
        qemu_fprintf(f, " %s %016" PRIx64, regnames[i], env->gpr[i]);
        if ((i & 3) == 3) {
            qemu_fprintf(f, "\n");
        }
    }

    /* fpr */
    if (flags & CPU_DUMP_FPU) {
        for (i = 0; i < 32; i++) {
            qemu_fprintf(f, " %s %016" PRIx64, fregnames[i], env->fpr[i]);
            if ((i & 3) == 3) {
                qemu_fprintf(f, "\n");
            }
        }
    }
}

#ifndef CONFIG_USER_ONLY
#include "hw/core/sysemu-cpu-ops.h"

static const struct SysemuCPUOps loongarch_sysemu_ops = {
    .get_phys_page_debug = loongarch_cpu_get_phys_page_debug,
};
#endif

#ifdef CONFIG_TCG
#include "hw/core/tcg-cpu-ops.h"

#ifdef CONFIG_USER_ONLY
static bool loongarch_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                       MMUAccessType access_type, int mmu_idx,
                       bool probe, uintptr_t retaddr)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;

    env->badaddr = address;
    cs->exception_index = EXCP_ADE;
    do_raise_exception(env, cs->exception_index, retaddr);
}
#endif

static struct TCGCPUOps loongarch_tcg_ops = {
    .initialize = loongarch_translate_init,
    .synchronize_from_tb = loongarch_cpu_synchronize_from_tb,
    .tlb_fill = loongarch_cpu_tlb_fill,
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

    cc->class_by_name = loongarch_cpu_class_by_name;
    cc->has_work = loongarch_cpu_has_work;
    cc->dump_state = loongarch_cpu_dump_state;
    cc->set_pc = loongarch_cpu_set_pc;
#ifndef CONFIG_USER_ONLY
    dc->vmsd = &vmstate_loongarch_cpu;
    cc->sysemu_ops = &loongarch_sysemu_ops;
#endif
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
