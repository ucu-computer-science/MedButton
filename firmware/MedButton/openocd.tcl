source [find interface/kitprog3.cfg]
source [find target/psoc6.cfg]
kitprog3 power_config on 3300
${TARGET}.cm4 configure -rtos auto -rtos-wipe-on-reset-halt 1
psoc6 sflash_restrictions 1
