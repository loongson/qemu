/*
 * Loongarch 7A1000 I/O interrupt controller definitions
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define TYPE_LOONGARCH_PCH_PIC "loongarch_pch_pic"
DECLARE_INSTANCE_CHECKER(struct loongarch_pch_pic, LOONGARCH_PCH_PIC,
                         TYPE_LOONGARCH_PCH_PIC)

#define PCH_PIC_ROUTE_ENTRY_OFFSET      0x100
#define PCH_PIC_INT_ID_OFFSET           0x00
#define PCH_PIC_INT_ID_VAL              0x7000000UL
#define PCH_PIC_INT_ID_VER              0x1f0001UL
#define PCH_PIC_INT_MASK_OFFSET         0x20
#define PCH_PIC_INT_EDGE_OFFSET         0x60
#define PCH_PIC_INT_CLEAR_OFFSET        0x80
#define PCH_PIC_INT_STATUS_OFFSET       0x3a0
#define PCH_PIC_INT_POL_OFFSET          0x3e0
#define PCH_PIC_HTMSI_EN_OFFSET         0x40
#define PCH_PIC_HTMSI_VEC_OFFSET        0x200
#define PCH_PIC_AUTO_CTRL0_OFFSET       0xc0
#define PCH_PIC_AUTO_CTRL1_OFFSET       0xe0

typedef struct loongarch_pch_pic {
    SysBusDevice parent_obj;
    qemu_irq parent_irq[32];
    uint32_t int_id;
    uint32_t int_mask; /*0x020 interrupt mask register*/
    uint32_t htmsi_en;/*0x040 1=msi*/
    uint32_t intedge; /*0x060 edge=1 level  =0*/
    uint32_t intclr; /*0x080 for clean edge int,set 1 clean,set 0 is noused*/
    uint32_t auto_crtl0; /*0x0c0*/
    uint32_t auto_crtl1; /*0x0e0*/
    uint8_t route_entry[32]; /*0x100 - 0x120*/
    uint8_t htmsi_vector[32]; /*0x200 - 0x220*/
    uint32_t last_intirr;    /* edge detection */
    uint32_t intirr; /* 0x380 interrupt request register */
    uint32_t intisr; /* 0x3a0 interrupt service register */
    /*
     * 0x3e0 interrupt level polarity selection
     * register 0 for high level trigger
     */
    uint32_t int_polarity;
    MemoryRegion iomem;
} loongarch_pch_pic;


