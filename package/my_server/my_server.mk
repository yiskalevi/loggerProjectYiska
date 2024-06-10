################################################################################
#
# udp_server package
#
################################################################################

MY_SERVER_VERSION = 1.0
MY_SERVER_SITE = package/my_server/src
MY_SERVER_SITE_METHOD = local

define MY_SERVER_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define MY_SERVER_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/my_server  $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

