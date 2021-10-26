/*
 * QEMU LoongArch machine State
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "cpu.h"
#include "migration/cpu.h"
#include "internals.h"

/* TLB state */
static int get_tlb(QEMUFile *f, void *pv, size_t size,
                   const VMStateField *field)
{
    loongarch_tlb *v = pv;

    qemu_get_betls(f, &v->VPN);
    qemu_get_be64s(f, &v->PageMask);
    qemu_get_be32s(f, &v->PageSize);
    qemu_get_be16s(f, &v->ASID);
    qemu_get_be64s(f, &v->V0);
    qemu_get_be64s(f, &v->V1);
    qemu_get_be64s(f, &v->D0);
    qemu_get_be64s(f, &v->D1);
    qemu_get_be64s(f, &v->PLV0);
    qemu_get_be64s(f, &v->PLV1);
    qemu_get_be64s(f, &v->MAT0);
    qemu_get_be64s(f, &v->MAT1);
    qemu_get_be64s(f, &v->G);
    qemu_get_be64s(f, &v->PPN0);
    qemu_get_be64s(f, &v->PPN1);
    qemu_get_be64s(f, &v->NR0);
    qemu_get_be64s(f, &v->NR1);
    qemu_get_be64s(f, &v->NX0);
    qemu_get_be64s(f, &v->NX1);
    qemu_get_be64s(f, &v->NE);
    qemu_get_be64s(f, &v->RPLV0);
    qemu_get_be64s(f, &v->RPLV1);

    return 0;
}

static int put_tlb(QEMUFile *f, void *pv, size_t size,
                   const VMStateField *field, JSONWriter *vmdesc)
{
    loongarch_tlb *v = pv;

    qemu_put_betls(f, &v->VPN);
    qemu_put_be64s(f, &v->PageMask);
    qemu_put_be32s(f, &v->PageSize);
    qemu_put_be16s(f, &v->ASID);
    qemu_put_be64s(f, &v->V0);
    qemu_put_be64s(f, &v->V1);
    qemu_put_be64s(f, &v->D0);
    qemu_put_be64s(f, &v->D1);
    qemu_put_be64s(f, &v->PLV0);
    qemu_put_be64s(f, &v->PLV1);
    qemu_put_be64s(f, &v->MAT0);
    qemu_put_be64s(f, &v->MAT1);
    qemu_put_be64s(f, &v->G);
    qemu_put_be64s(f, &v->PPN0);
    qemu_put_be64s(f, &v->PPN1);
    qemu_put_be64s(f, &v->NR0);
    qemu_put_be64s(f, &v->NR1);
    qemu_put_be64s(f, &v->NX0);
    qemu_put_be64s(f, &v->NX1);
    qemu_put_be64s(f, &v->NE);
    qemu_put_be64s(f, &v->RPLV0);
    qemu_put_be64s(f, &v->RPLV1);

    return 0;
}

const VMStateInfo vmstate_info_tlb = {
    .name = "tlb_entry",
    .get  = get_tlb,
    .put  = put_tlb,
};

#define VMSTATE_TLB_ARRAY_V(_f, _s, _n, _v)                     \
    VMSTATE_ARRAY(_f, _s, _n, _v, vmstate_info_tlb, loongarch_tlb)

#define VMSTATE_TLB_ARRAY(_f, _s, _n)                           \
    VMSTATE_TLB_ARRAY_V(_f, _s, _n, 0)

const VMStateDescription vmstate_tlb = {
    .name = "cpu/tlb",
    .version_id = 0,
    .minimum_version_id = 0,
    .fields = (VMStateField[]) {
        VMSTATE_TLB_ARRAY(env.tlb, LoongArchCPU, LOONGARCH_TLB_MAX),
        VMSTATE_END_OF_LIST()
    }
};

/* LoongArch CPU state */

const VMStateDescription vmstate_loongarch_cpu = {
    .name = "cpu",
    .version_id = 0,
    .minimum_version_id = 0,
    .fields = (VMStateField[]) {

        VMSTATE_UINTTL_ARRAY(env.gpr, LoongArchCPU, 32),
        VMSTATE_UINTTL(env.pc, LoongArchCPU),
        VMSTATE_UINT64_ARRAY(env.fpr, LoongArchCPU, 32),
        VMSTATE_UINT32(env.fcsr0, LoongArchCPU),

        /* TLB */
        VMSTATE_UINT32(env.stlb_size, LoongArchCPU),
        VMSTATE_UINT32(env.mtlb_size, LoongArchCPU),

        /* Remaining CSR registers */
        VMSTATE_UINT64(env.CSR_CRMD, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PRMD, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_EUEN, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_MISC, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_ECFG, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_ESTAT, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_ERA, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_BADV, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_BADI, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_EENTRY, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBIDX, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBEHI, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBELO0, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBELO1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PGDL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PGDH, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PGD, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PWCL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PWCH, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_STLBPS, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_RVACFG, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_CPUID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PRCFG1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PRCFG2, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PRCFG3, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_SAVE0, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_SAVE1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_SAVE2, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_SAVE3, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_SAVE4, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_SAVE5, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_SAVE6, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_SAVE7, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TMID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TCFG, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TVAL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_CNTC, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TINTCLR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_LLBCTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IMPCTL1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IMPCTL2, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBRENTRY, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBRBADV, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBRERA, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBRSAVE, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBRELO0, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBRELO1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBREHI, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_TLBRPRMD, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_MERRCTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_MERRINFO, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_MERRINFO1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_MERRENT, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_MERRERA, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_MERRSAVE, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_CTAG, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DMWIN0, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DMWIN1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DMWIN2, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DMWIN3, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PERFCTRL0, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PERFCNTR0, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PERFCTRL1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PERFCNTR1, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PERFCTRL2, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PERFCNTR2, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PERFCTRL3, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_PERFCNTR3, LoongArchCPU),
        /* debug */
        VMSTATE_UINT64(env.CSR_MWPC, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_MWPS, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB0ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB0MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB0CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB0ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB1ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB1MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB1CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB1ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB2ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB2MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB2CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB2ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB3ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB3MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB3CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DB3ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_FWPC, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_FWPS, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB0ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB0MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB0CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB0ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB1ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB1MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB1CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB1ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB2ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB2MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB2CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB2ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB3ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB3MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB3CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB3ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB4ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB4MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB4CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB4ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB5ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB5MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB5CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB5ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB6ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB6MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB6CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB6ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB7ADDR, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB7MASK, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB7CTL, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_IB7ASID, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DBG, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DERA, LoongArchCPU),
        VMSTATE_UINT64(env.CSR_DESAVE, LoongArchCPU),

        VMSTATE_END_OF_LIST()
    },
    .subsections = (const VMStateDescription*[]) {
        &vmstate_tlb,
        NULL
    }
};
