################################################################################
#
# roboos-hal
#
################################################################################

ROBOOS_HAL_VERSION = 0.1
ROBOOS_HAL_SITE = $(TOPDIR)/board/roboos/hal
ROBOOS_HAL_SITE_METHOD = local

define ROBOOS_HAL_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) -DHAL_TEST_MAIN \
		-o $(@D)/hal_test $(@D)/roboos_hal.c -lm
endef

define ROBOOS_HAL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hal_test \
		$(TARGET_DIR)/usr/bin/hal_test
endef

$(eval $(generic-package))
