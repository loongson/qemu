loongarch_ss = ss.source_set()
loongarch_ss.add(when: 'CONFIG_LOONGSON3_LS7A', if_true: files('loongson3.c'))
loongarch_ss.add(when: 'CONFIG_FW_CFG_LOONGARCH', if_true: files('fw_cfg.c'))
loongarch_ss.add(when: 'CONFIG_ACPI', if_true: files('acpi-build.c'))

hw_arch += {'loongarch': loongarch_ss}
