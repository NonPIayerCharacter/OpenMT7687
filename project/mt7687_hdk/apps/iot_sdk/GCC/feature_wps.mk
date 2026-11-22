ifeq ($(PROJ_PATH),)
include feature.mk
else
include $(PROJ_PATH)/feature.mk
endif

MTK_WIFI_WPS_ENABLE                 = y
#XIP Overflow when add wpa3 support for wps, because of
#too many features, so we turn off the FOTA here
MTK_FOTA_ENABLE                     = n
MTK_FOTA_CLI_ENABLE                 = n
MTK_WIFI_WPA3_SAE_ENABLE            = y
MTK_WIFI_PMF_ENABLE                 = y
