WIFI_STACK = middleware/MTK/wifi_stack
WIFI_STACK_ROM = $(WIFI_STACK)/rom/$(MTK_ROM_VER)
PATFORM_CHIP = driver/chip/mt7686

CFLAGS += -I$(SOURCE_DIR)/$(PLATFORM_CHIP)/inc
CFLAGS += -I$(SOURCE_DIR)/$(WIFI_RAM_PATH)/inc

include $(SOURCE_DIR)/prebuilt/$(WIFI_STACK_ROM)/rom_flags.mk

CFLAGS += -I$(SOURCE_DIR)/prebuilt/$(WIFI_STACK)/inc

ifeq ($(MTK_WIFI_STA_CSA_ENABLE),y)
LIBS += $(SOURCE_DIR)/prebuilt/$(WIFI_STACK)/lib/libwifi_$(IC_CONFIG)_ram_repeater_wps.a
else ifeq ($(MTK_WIFI_REPEATER_ENABLE)$(MTK_WIFI_WPS_ENABLE),yy)
LIBS += $(SOURCE_DIR)/prebuilt/$(WIFI_STACK)/lib/libwifi_$(IC_CONFIG)_ram_repeater_wps.a
else ifeq ($(MTK_WIFI_REPEATER_ENABLE),y)
LIBS += $(SOURCE_DIR)/prebuilt/$(WIFI_STACK)/lib/libwifi_$(IC_CONFIG)_ram_repeater.a
else ifeq ($(MTK_WIFI_WPS_ENABLE),y)
LIBS += $(SOURCE_DIR)/prebuilt/$(WIFI_STACK)/lib/libwifi_$(IC_CONFIG)_ram_wps.a
else ifeq ($(MTK_WIFI_WPA3_SAE_ENABLE),y)
LIBS += $(SOURCE_DIR)/prebuilt/$(WIFI_STACK)/lib/libwifi_$(IC_CONFIG)_ram_wpa3.a
else ifeq ($(MTK_WIFI_PMF_ENABLE),y)
LIBS += $(SOURCE_DIR)/prebuilt/$(WIFI_STACK)/lib/libwifi_$(IC_CONFIG)_ram_wpa3.a
else
LIBS += $(SOURCE_DIR)/prebuilt/$(WIFI_STACK)/lib/libwifi_$(IC_CONFIG)_ram.a
endif
IC2_CONFIG = mt5932
#SIM_E2 = _sim
SIM_E2 = 

WIFI_PREBUILT_DIR = $(SOURCE_DIR)/prebuilt/middleware/MTK/wifi_stack
ifeq ($(MTK_WIFI_ROM_ENABLE), y)
WIFI_ROM_SYM = $(WIFI_PREBUILT_DIR)/rom/$(MTK_ROM_VER)/$(IC2_CONFIG)_wifi_rom$(SIM_E2).sym
WIFI_ROM_DATA = $(SOURCE_DIR)/prebuilt/middleware/MTK/wifi_stack/rom/$(MTK_ROM_VER)/$(IC2_CONFIG)_wifi_rom.data
else
WIFI_ROM_SYM =
WIFI_ROM_DATA =
endif
