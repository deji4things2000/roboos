################################################################################
#
# rilp
#
################################################################################

RILP_VERSION = 1.0.0
RILP_SITE = $(TOPDIR)/board/roboos/rilp
RILP_SITE_METHOD = local

define RILP_BUILD_CMDS
    $(MAKE) CC=$(TARGET_CC) AR=$(TARGET_AR) -C $(@D) all
endef

define RILP_INSTALL_TARGET_CMDS
    $(MAKE) DESTDIR=$(TARGET_DIR) -C $(@D) install
endef

$(eval $(generic-package))
