include ../common.mak

REBUILD_TDTS_MODULE=0

ifeq ($(wildcard $(TOP)/bwdpi_source/prebuilt/*),)
pre_build:
ifeq ($(RTCONFIG_SOC_IPQ8064),y)
	@cp include/tdts/sysdeps/ipq806x-tmcfg.h include/tdts/tmcfg.h -f                     # ipq806x
	@cp include/udb/sysdeps/ipq806x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq806x
	@cp lib/ipq806x/* lib/ -f                                                            # ipq806x
	@cp bin/ipq806x/* bin/ -rf                                                           # ipq806x
	@cp modules/sysdeps/ipq806x-Module.symvers modules/sysdeps/Module.symvers -rf        # ipq806x
	@cp src/tdts_udb_shell/sysdeps/ipq806x-kbuild.inc src/tdts_udb_shell/kbuild.inc -f   # ipq806x
	@cp src/tdts_udbfw/sysdeps/ipq806x/* src/tdts_udbfw/ -f                              # ipq806x
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	@cp include/tdts/sysdeps/dakota-tmcfg.h include/tdts/tmcfg.h -f                      # dakota
	@cp include/udb/sysdeps/dakota-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # dakota
	@cp lib/dakota/* lib/ -f                                                             # dakota
	@cp bin/dakota/* bin/ -rf                                                            # dakota
	@cp modules/sysdeps/dakota-Module.symvers modules/sysdeps/Module.symvers -rf         # dakota
	@cp src/tdts_udb_shell/sysdeps/dakota-kbuild.inc src/tdts_udb_shell/kbuild.inc -f    # dakota
	@cp src/tdts_udbfw/sysdeps/dakota/* src/tdts_udbfw/ -f                               # dakota
else ifeq ($(RTCONFIG_RALINK),y)
	@cp include/tdts/sysdeps/mtk-tmcfg.h include/tdts/tmcfg.h -f                         # mtk
	@cp include/udb/sysdeps/mtk-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # mtk
	@cp lib/mtk/* lib/ -f                                                                # mtk
	@cp bin/mtk/* bin/ -rf                                                               # mtk
	@cp modules/sysdeps/mtk-Module.symvers modules/sysdeps/Module.symvers -rf            # mtk
	@cp src/tdts_udb_shell/sysdeps/mtk-kbuild.inc src/tdts_udb_shell/kbuild.inc -f       # mtk
	@cp src/tdts_udbfw/sysdeps/mtk/* src/tdts_udbfw/ -f                                  # mtk
else ifeq ($(RTCONFIG_ALPINE),y)
	@cp include/tdts/sysdeps/alpine-tmcfg.h include/tdts/tmcfg.h -f                      # gt-ac9600
	@cp include/udb/sysdeps/alpine-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # gt-ac9600
	@cp lib/alpine/* lib/ -f                                                             # gt-ac9600
	@cp bin/alpine/* bin/ -rf                                                            # gt-ac9600
else ifeq ($(RTCONFIG_LANTIQ),y)
	@cp include/tdts/sysdeps/lantiq-tmcfg.h include/tdts/tmcfg.h -f                      # lantiq
	@cp include/udb/sysdeps/lantiq-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # lantiq
	@cp lib/lantiq/* lib/ -f                                                             # lantiq
	@cp bin/lantiq/* bin/ -rf                                                            # lantiq
else ifeq ($(RTCONFIG_BCMARM),y)
	@cp lib/bcm/* lib/ -f                                                                # bcm all
	@cp src/tdts_udb_shell/sysdeps/brcm-kbuild.inc src/tdts_udb_shell/kbuild.inc -f      # bcm all
	@cp src/tdts_udbfw/sysdeps/brcm/* src/tdts_udbfw/ -f                                 # bcm non-up
ifeq ($(RTCONFIG_BCM7),y)
	@cp include/tdts/sysdeps/bcm7-tmcfg.h include/tdts/tmcfg.h -f                        # bcm7
	@cp include/udb/sysdeps/bcm7-tmcfg_udb.h include/udb/tmcfg_udb.h -f                  # bcm7
	@cp bin/bcm7/* bin/ -rf                                                              # bcm7
	@cp modules/sysdeps/bcm7-Module.symvers modules/sysdeps/Module.symvers -rf           # bcm7
else ifeq ($(HND_ROUTER),y)
	@cp include/tdts/sysdeps/hnd-tmcfg.h include/tdts/tmcfg.h -f                         # hnd
	@cp include/udb/sysdeps/hnd-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # hnd
	@cp lib/hnd/* lib/ -f                                                                # hnd
	@cp bin/hnd/* bin/ -rf                                                               # hnd
	@cp modules/sysdeps/hnd-Module.symvers modules/sysdeps/Module.symvers -rf            # hnd
	@cp src/tdts_udbfw/sysdeps/hnd/* src/tdts_udbfw/ -f                                  # hnd
	@cp src/tdts_udb_shell/sysdeps/hnd-kbuild.inc src/tdts_udb_shell/kbuild.inc -f       # hnd
else ifeq ($(RTCONFIG_BCM_7114),y)
	@cp include/tdts/sysdeps/bcm7114-tmcfg.h include/tdts/tmcfg.h -f                     # bcm7114
	@cp include/udb/sysdeps/bcm7114-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm7114
	@cp bin/bcm7114/* bin/ -rf                                                           # bcm7114
	@cp modules/sysdeps/bcm7114-Module.symvers modules/sysdeps/Module.symvers -rf        # bcm7114
else
ifeq ($(RTCONFIG_BCMSMP),y)
	@cp include/tdts/sysdeps/bcm6_smp-tmcfg.h include/tdts/tmcfg.h -f                    # bcm6_smp
	@cp include/udb/sysdeps/bcm6_smp-tmcfg_udb.h include/udb/tmcfg_udb.h -f              # bcm6_smp
	@cp bin/bcm6_smp/* bin/ -rf                                                          # bcm6_smp
	@cp modules/sysdeps/bcm6_smp-Module.symvers modules/sysdeps/Module.symvers -rf       # bcm6_smp
else
	@cp include/tdts/sysdeps/bcm6_up-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6_up
	@cp include/udb/sysdeps/bcm6_up-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6_up
	@cp bin/bcm6_up/* bin/ -rf                                                           # bcm6_up
	@cp modules/sysdeps/bcm6_up-Module.symvers modules/sysdeps/Module.symvers -rf        # bcm6_up
	@cp src/tdts_udbfw/sysdeps/brcm_up/* src/tdts_udbfw/ -f                              # bcm up
endif
endif
endif
else  # ($(wildcard $(TOP)/bwdpi_source/prebuilt/*),)
pre_build:
ifeq ($(RTCONFIG_SOC_IPQ8064),y)
	@cp include/tdts/sysdeps/ipq806x-tmcfg.h include/tdts/tmcfg.h -f                     # ipq806x
	@cp include/udb/sysdeps/ipq806x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq806x
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	@cp include/tdts/sysdeps/dakota-tmcfg.h include/tdts/tmcfg.h -f                      # dakota
	@cp include/udb/sysdeps/dakota-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # dakota
else ifeq ($(RTCONFIG_RALINK),y)
	@cp include/tdts/sysdeps/mtk-tmcfg.h include/tdts/tmcfg.h -f                         # mtk
	@cp include/udb/sysdeps/mtk-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # mtk
else ifeq ($(RTCONFIG_ALPINE),y)
	@cp include/tdts/sysdeps/alpine-tmcfg.h include/tdts/tmcfg.h -f                      # gt-ac9600
	@cp include/udb/sysdeps/alpine-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # gt-ac9600
else ifeq ($(RTCONFIG_LANTIQ),y)
	@cp include/tdts/sysdeps/lantiq-tmcfg.h include/tdts/tmcfg.h -f                      # lantiq
	@cp include/udb/sysdeps/lantiq-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # lantiq
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
	@cp include/tdts/sysdeps/bcm7-tmcfg.h include/tdts/tmcfg.h -f                        # bcm7
	@cp include/udb/sysdeps/bcm7-tmcfg_udb.h include/udb/tmcfg_udb.h -f                  # bcm7
else ifeq ($(HND_ROUTER),y)
	@cp include/tdts/sysdeps/hnd-tmcfg.h include/tdts/tmcfg.h -f                         # hnd
	@cp include/udb/sysdeps/hnd-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # hnd
else ifeq ($(RTCONFIG_BCM_7114),y)
	@cp include/tdts/sysdeps/bcm7114-tmcfg.h include/tdts/tmcfg.h -f                     # bcm7114
	@cp include/udb/sysdeps/bcm7114-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm7114
else
ifeq ($(RTCONFIG_BCMSMP),y)
	@cp include/tdts/sysdeps/bcm6_smp-tmcfg.h include/tdts/tmcfg.h -f                    # bcm6_smp
	@cp include/udb/sysdeps/bcm6_smp-tmcfg_udb.h include/udb/tmcfg_udb.h -f              # bcm6_smp
else
	@cp include/tdts/sysdeps/bcm6_up-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6_up
	@cp include/udb/sysdeps/bcm6_up-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6_up
endif
endif
endif
endif # ($(wildcard $(TOP)/bwdpi_source/prebuilt/*),)


asus_build:
	@cd asus/ && $(MAKE)

asus_sql_build:
	@cd asus_sql/ && $(MAKE)

ifeq ($(wildcard $(TOP)/bwdpi_source/src/tdts_shell/*.c),)
kernel_build:
else
kernel_build:
ifeq ($(REBUILD_TDTS_MODULE),1)
	-@cd src/tdts_shell && $(MAKE)
	-@cd src/tdts_udb_shell && $(MAKE)
	-@cd src/tdts_udbfw && $(MAKE)
endif
endif

all: pre_build asus_build asus_sql_build kernel_build

ifeq ($(wildcard $(TOP)/bwdpi_source/prebuilt/*),)
install: all
	# asus function here
	@cd asus/ && $(MAKE) install
	@cd asus_sql/ && $(MAKE) install
	# [INSTALL] PREPARE bwdpi_source/RC_INDEP/
	@rm -rf RC_INDEP
	@mkdir RC_INDEP
	@tar -xf bin/dcd.tar -C RC_INDEP/.
	@tar -xf bin/shn_ctrl.tar -C RC_INDEP/.
	@tar -xf bin/tdts_wred.tar -C RC_INDEP/.
	@cp -f bin/libshn_*.so RC_INDEP
	@cp -f bin/rule.trf RC_INDEP
	@cp -f bin/tdts_rule_agent RC_INDEP
	@cp -f bin/tcd RC_INDEP
	@rm -f RC_INDEP/*.sh
	@rm -f RC_INDEP/*.conf
	-@cp -f src/tdts*/*.ko RC_INDEP/
ifeq ($(RTCONFIG_SOC_IPQ8064),y)
	-@cp -f modules/ipq806x/* RC_INDEP/
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	-@cp -f modules/dakota/* RC_INDEP/
else ifeq ($(RTCONFIG_RALINK),y)
	-@cp -f modules/mtk/* RC_INDEP/
	-@cp -f bin/partial/rule.trf RC_INDEP/
else ifeq ($(RTCONFIG_ALPINE),y)
	-@cp -f modules/alpine/* RC_INDEP/
else ifeq ($(RTCONFIG_LANTIQ),y)
	-@cp -f modules/lantiq/* RC_INDEP/
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
	-@cp -f modules/bcm7/* RC_INDEP/
else ifeq ($(HND_ROUTER),y)
	-@cp -f modules/hnd/* RC_INDEP/
else ifeq ($(RTCONFIG_BCM_7114),y)
	-@cp -f modules/bcm7114/* RC_INDEP/
else
ifeq ($(RTCONFIG_BCMSMP),y)
	-@cp -f modules/bcm6_smp/* RC_INDEP/
else
	-@cp -f modules/bcm6_up/* RC_INDEP/
	-@cp -f bin/partial/rule.trf RC_INDEP/
endif
endif
endif
ifeq ($(HND_ROUTER),y)
	@echo "start to install bwdpi_source into 94908HND"
	@install -d $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.so $(INSTALLDIR)/usr/lib/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.ko $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.trf $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.cert $(INSTALLDIR)/usr/bwdpi/
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred $(INSTALLDIR)/usr/sbin/wred
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_conf $(INSTALLDIR)/usr/sbin/wred_set_conf
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_wbl $(INSTALLDIR)/usr/sbin/wred_set_wbl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/dcd $(INSTALLDIR)/usr/sbin/dcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tcd $(INSTALLDIR)/usr/sbin/tcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/shn_ctrl $(INSTALLDIR)/usr/sbin/shn_ctrl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tdts_rule_agent $(INSTALLDIR)/usr/sbin/tdts_rule_agent
	@install -D $(TOP)/bwdpi_source/RC_INDEP/sample.bin $(INSTALLDIR)/usr/sbin/sample.bin
endif
else
install:
	# asus function here
	@cd asus/ && $(MAKE) install
	@cd asus_sql/ && $(MAKE) install
	# [INSTALL] prebuilt into RC_INDEP
	-@mkdir RC_INDEP
	-@cp -f prebuilt/* RC_INDEP/
ifeq ($(HND_ROUTER),y)
	@echo "start to install bwdpi_source into 94908HND"
	@install -d $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.so $(INSTALLDIR)/usr/lib/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.ko $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.trf $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.cert $(INSTALLDIR)/usr/bwdpi/
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred $(INSTALLDIR)/usr/sbin/wred
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_conf $(INSTALLDIR)/usr/sbin/wred_set_conf
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_wbl $(INSTALLDIR)/usr/sbin/wred_set_wbl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/dcd $(INSTALLDIR)/usr/sbin/dcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tcd $(INSTALLDIR)/usr/sbin/tcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/shn_ctrl $(INSTALLDIR)/usr/sbin/shn_ctrl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tdts_rule_agent $(INSTALLDIR)/usr/sbin/tdts_rule_agent
	@install -D $(TOP)/bwdpi_source/RC_INDEP/sample.bin $(INSTALLDIR)/usr/sbin/sample.bin
endif
endif

ifeq ($(wildcard $(TOP)/bwdpi_source/prebuilt/*),)
clean:
	# asus function here
	@cd asus && $(MAKE) clean
	@cd asus_sql && $(MAKE) clean
ifeq ($(REBUILD_TDTS_MODULE),1)
	# kernel clean here
	-@cd src/tdts_shell && $(MAKE) clean && rm tdts_udb_core.o -f
	-@cd src/tdts_udb_shell && $(MAKE) clean && rm tdts_udb_core.o -f
	-@cd src/tdts_udbfw && $(MAKE) clean && rm tdts_udb_core.o -f
endif
else
clean:
endif
