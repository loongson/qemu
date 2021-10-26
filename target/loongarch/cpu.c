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
    [EXCP_EXT_INTERRUPT] = "interrupt",
    [EXCP_DBP] = "debug breakpoint",
    [EXCP_IBE] = "instruction bus error",
    [EXCP_DBE] = "data bus error",
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

#if !defined(CONFIG_USER_ONLY)
static inline bool cpu_loongarch_hw_interrupts_enabled(CPULoongArchState *env)
{
    bool ret = 0;

    ret = (FIELD_EX64(env->CSR_CRMD, CSR_CRMD, IE) &&
          !(FIELD_EX64(env->CSR_DBG, CSR_DBG, DST)));

    return ret;
}

/* Check if there is pending and not masked out interrupt */
static inline bool cpu_loongarch_hw_interrupts_pending(CPULoongArchState *env)
{
    uint32_t pending;
    uint32_t status;
    bool r;

    pending = FIELD_EX64(env->CSR_ESTAT, CSR_ESTAT, IS);
    status  = FIELD_EX64(env->CSR_ECFG, CSR_ECFG, LIE);

    r = (pending & status) != 0;
    return r;
}

static inline unsigned int get_vint_size(CPULoongArchState *env)
{
    uint64_t vs = FIELD_EX64(env->CSR_ECFG, CSR_ECFG, VS);
    uint64_t size = 0;

    if (vs == 0) {
        return 0;
    }

    if (vs < 8) {
        size = 1 << (vs + 2);
    }

    if (vs > 8) {
        qemu_log("%s: unexpected value", __func__);
        assert(0);
    }

    return size;
}

static void loongarch_cpu_do_interrupt(CPUState *cs)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;
    bool update_badinstr = 0;
    int cause = -1;
    const char *name;

    if (qemu_loglevel_mask(CPU_LOG_INT)
        && cs->exception_index != EXCP_EXT_INTERRUPT) {
        if (cs->exception_index < 0 || cs->exception_index > EXCP_LAST) {
            name = "unknown";
        } else {
            name = excp_names[cs->exception_index];
        }

        qemu_log("%s enter: pc " TARGET_FMT_lx " ERA " TARGET_FMT_lx
                 " TLBRERA 0x%016lx" " %s exception\n", __func__,
                 env->pc, env->CSR_ERA, env->CSR_TLBRERA, name);
    }

    switch (cs->exception_index) {
    case EXCP_SYSCALL:
        cause = EXCCODE_SYS;
        update_badinstr = 1;
        break;
    case EXCP_BREAK:
        cause = EXCCODE_BRK;
        update_badinstr = 1;
        break;
    case EXCP_INE:
        cause = EXCCODE_INE;
        update_badinstr = 1;
        break;
    case EXCP_FPE:
        cause = EXCCODE_FPE;
        update_badinstr = 1;
        break;
    case EXCP_ADE:
        cause = EXCCODE_ADE;
        update_badinstr = 1;
        break;
    case EXCP_DBP:
        env->CSR_DBG = FIELD_DP64(env->CSR_DBG, CSR_DBG, DCL, 1);
        env->CSR_DBG = FIELD_DP64(env->CSR_DBG, CSR_DBG, ECODE, 0xC);
        env->CSR_DERA = env->pc;
        env->CSR_DBG = FIELD_DP64(env->CSR_DBG, CSR_DBG, DST, 1);
        env->pc = env->CSR_EENTRY + 0x480;
        break;
    case EXCP_EXT_INTERRUPT:
        cause = 0;
        break;
    case EXCP_TLBL:
        cause = EXCCODE_PIL;
        update_badinstr = 1;
        break;
    case EXCP_TLBS:
        cause = EXCCODE_PIS;
        update_badinstr = 1;
        break;
    case EXCP_INST_NOTAVAIL:
        cause = EXCCODE_PIF;
        break;
    case EXCP_TLBM:
        cause = EXCCODE_PME;
        break;
    case EXCP_TLBPE:
        cause = EXCCODE_PPI;
        break;
    case EXCP_TLBNX:
        cause = EXCCODE_PNX;
        break;
    case EXCP_TLBNR:
        cause = EXCCODE_PNR;
        update_badinstr = 1;
        break;
    case EXCP_IBE:
        cause = EXCCODE_ADE;
        break;
    case EXCP_DBE:
        cause = EXCCODE_ADE;
        break;
    default:
        qemu_log("Error: exception(%d) '%s' has not been supported\n",
                 cs->exception_index, excp_names[cs->exception_index]);
        abort();
    }

    if (env->tlbfill) {
        env->CSR_TLBRERA = env->pc;
        env->CSR_TLBRERA = FIELD_DP64(env->CSR_TLBRERA, CSR_TLBRERA, ISTLBR, 1);
    } else {
        env->CSR_ERA = env->pc;
    }

    if (update_badinstr) {
        env->CSR_BADI = cpu_ldl_code(env, env->pc);
    }

    /* save PLV and IE */
    if (env->tlbfill) {
        env->CSR_TLBRPRMD = FIELD_DP64(env->CSR_TLBRPRMD, CSR_TLBRPRMD, PPLV,
                                       FIELD_EX64(env->CSR_CRMD, CSR_CRMD, PLV));
        env->CSR_TLBRPRMD = FIELD_DP64(env->CSR_TLBRPRMD, CSR_TLBRPRMD, PIE,
                                       FIELD_EX64(env->CSR_CRMD, CSR_CRMD, IE));
    } else {
        env->CSR_PRMD = FIELD_DP64(env->CSR_PRMD, CSR_PRMD, PPLV,
                                   FIELD_EX64(env->CSR_CRMD, CSR_CRMD, PLV));
        env->CSR_PRMD = FIELD_DP64(env->CSR_PRMD, CSR_PRMD, PIE,
                                   FIELD_EX64(env->CSR_CRMD, CSR_CRMD, IE));
    }

    env->CSR_CRMD = FIELD_DP64(env->CSR_CRMD, CSR_CRMD, PLV, 0);
    env->CSR_CRMD = FIELD_DP64(env->CSR_CRMD, CSR_CRMD, IE, 0);

    uint32_t vec_size = get_vint_size(env);
    env->pc = env->CSR_EENTRY;
    env->pc += cause * vec_size;
    if (env->tlbfill) {
        /* TLB Refill */
        env->pc = env->CSR_TLBRENTRY;
    }
    if  (cs->exception_index == EXCP_EXT_INTERRUPT) {
        /* Interrupt */
        uint32_t vector = 0;
        uint32_t pending = FIELD_EX64(env->CSR_ESTAT, CSR_ESTAT, IS);
        pending &= FIELD_EX64(env->CSR_ECFG, CSR_ECFG, LIE);

        /* Find the highest-priority interrupt. */
        while (pending >>= 1) {
            vector++;
        }
        env->pc = env->CSR_EENTRY + (EXCODE_IP + vector) * vec_size;
        if (qemu_loglevel_mask(CPU_LOG_INT)) {
            qemu_log("%s: PC " TARGET_FMT_lx " ERA " TARGET_FMT_lx
                     " cause %d\n" "    A " TARGET_FMT_lx " D "
                     TARGET_FMT_lx " vector = %d ExC %08lx ExS %08lx\n",
                     __func__, env->pc, env->CSR_ERA,
                     cause, env->CSR_BADV, env->CSR_DERA, vector,
                     env->CSR_ECFG, env->CSR_ESTAT);
        }
    }

    /* Excode */
    env->CSR_ESTAT = FIELD_DP64(env->CSR_ESTAT, CSR_ESTAT, ECODE, cause);

    if (qemu_loglevel_mask(CPU_LOG_INT) && cs->exception_index != EXCP_EXT_INTERRUPT) {
        qemu_log("%s: PC " TARGET_FMT_lx " ERA 0x%08lx"  " cause %d%s\n"
                 " ESTAT %08lx EXCFG 0x%08lx BADVA 0x%08lx BADI 0x%08lx \
                 SYS_NUM %lu cpu %d asid 0x%lx" "\n",
                 __func__, env->pc, env->tlbfill ? env->CSR_TLBRERA : env->CSR_ERA,
                 cause, env->tlbfill ? "(refill)" : "", env->CSR_ESTAT, env->CSR_ECFG,
                 env->tlbfill ? env->CSR_TLBRBADV : env->CSR_BADV, env->CSR_BADI,
                 env->gpr[11], cs->cpu_index, env->CSR_ASID
         );
    }
    cs->exception_index = EXCP_NONE;
}

static void loongarch_cpu_do_transaction_failed(CPUState *cs, hwaddr physaddr,
                                    vaddr addr, unsigned size,
                                    MMUAccessType access_type,
                                    int mmu_idx, MemTxAttrs attrs,
                                    MemTxResult response, uintptr_t retaddr)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;

    if (access_type == MMU_INST_FETCH) {
        do_raise_exception(env, EXCP_IBE, retaddr);
    } else {
        do_raise_exception(env, EXCP_DBE, retaddr);
    }
}

static bool loongarch_cpu_exec_interrupt(CPUState *cs, int interrupt_request)
{
    if (interrupt_request & CPU_INTERRUPT_HARD) {
        LoongArchCPU *cpu = LOONGARCH_CPU(cs);
        CPULoongArchState *env = &cpu->env;

        if (cpu_loongarch_hw_interrupts_enabled(env) &&
            cpu_loongarch_hw_interrupts_pending(env)) {
            /* Raise it */
            cs->exception_index = EXCP_EXT_INTERRUPT;
            env->tlbfill = 0;
            loongarch_cpu_do_interrupt(cs);
            return true;
        }
    }
    return false;
}
#endif

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
#ifdef CONFIG_USER_ONLY
    return true;
#else
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;
    bool has_work = false;

    if ((cs->interrupt_request & CPU_INTERRUPT_HARD) &&
        cpu_loongarch_hw_interrupts_pending(env)) {
        has_work = true;
    }

    return has_work;
#endif
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
#ifndef CONFIG_USER_ONLY
    env->pc = env->CSR_EENTRY;
#endif
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
    env->CSR_EENTRY = 0x1C000000;
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

#ifndef CONFIG_USER_ONLY
    qemu_fprintf(f, "EUEN            0x%lx\n", env->CSR_EUEN);
    qemu_fprintf(f, "ESTAT           0x%lx\n", env->CSR_ESTAT);
    qemu_fprintf(f, "ERA             0x%lx\n", env->CSR_ERA);
    qemu_fprintf(f, "CRMD            0x%lx\n", env->CSR_CRMD);
    qemu_fprintf(f, "PRMD            0x%lx\n", env->CSR_PRMD);
    qemu_fprintf(f, "BadVAddr        0x%lx\n", env->CSR_BADV);
    qemu_fprintf(f, "TLB refill ERA  0x%lx\n", env->CSR_TLBRERA);
    qemu_fprintf(f, "TLB refill BadV 0x%lx\n", env->CSR_TLBRBADV);
    qemu_fprintf(f, "EENTRY            0x%lx\n", env->CSR_EENTRY);
    qemu_fprintf(f, "BadInstr        0x%lx\n", env->CSR_BADI);
    qemu_fprintf(f, "PRCFG1    0x%lx\nPRCFG2     0x%lx\nPRCFG3     0x%lx\n",
                 env->CSR_PRCFG1, env->CSR_PRCFG3, env->CSR_PRCFG3);
#endif

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

#if !defined(CONFIG_USER_ONLY)
    .cpu_exec_interrupt = loongarch_cpu_exec_interrupt,
    .do_interrupt = loongarch_cpu_do_interrupt,
    .do_transaction_failed = loongarch_cpu_do_transaction_failed,
#endif /* !CONFIG_USER_ONLY */
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
