################################################################################
#
# rilp-kernel
#
################################################################################

RILP_KERNEL_VERSION = 0.1
RILP_KERNEL_SITE = $(TOPDIR)/board/roboos/rilp-kernel
RILP_KERNEL_SITE_METHOD = local

RILP_KERNEL_MODULE_SUBDIRS = .

$(eval $(kernel-module))
$(eval $(generic-package))
