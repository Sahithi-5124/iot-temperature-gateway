################################################################################
# IoT Temperature Gateway
################################################################################

IOT_GATEWAY_VERSION = 1.0
IOT_GATEWAY_SITE = $(BR2_EXTERNAL_IOT_GATEWAY_PATH)/../../../application
IOT_GATEWAY_SITE_METHOD = local
IOT_GATEWAY_LICENSE = MIT
IOT_GATEWAY_DEPENDENCIES = mosquitto

define IOT_GATEWAY_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -std=c11 -Wall -Wextra -Wpedantic" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define IOT_GATEWAY_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/iot_gateway \
		$(TARGET_DIR)/usr/bin/iot_gateway
endef

$(eval $(generic-package))
