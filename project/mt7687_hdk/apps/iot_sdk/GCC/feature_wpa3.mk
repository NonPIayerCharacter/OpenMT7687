ifeq ($(PROJ_PATH),)
include feature.mk
else
include $(PROJ_PATH)/feature.mk
endif

MTK_WIFI_WPA3_SAE_ENABLE            = y
MTK_WIFI_PMF_ENABLE                 = y
