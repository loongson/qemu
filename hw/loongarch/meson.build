loongarch_ss = ss.source_set()
loongarch_ss.add(files('loongarch_int.c'))
loongarch_ss.add(when: 'CONFIG_LOONGSON_3A5000', if_true: files('ls3a5000_virt.c', 'ipi.c'))
loongarch_ss.add(when: 'CONFIG_FW_CFG_LOONGARCH', if_true: files('fw_cfg.c'))
loongarch_ss.add(when: 'CONFIG_ACPI', if_true: files('acpi-build.c'))

hw_arch += {'loongarch': loongarch_ss}
