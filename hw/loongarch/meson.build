loongarch_ss = ss.source_set()
loongarch_ss.add(when: 'CONFIG_LOONGSON_3A5000', if_true: files('ls3a5000_virt.c'))

hw_arch += {'loongarch': loongarch_ss}
