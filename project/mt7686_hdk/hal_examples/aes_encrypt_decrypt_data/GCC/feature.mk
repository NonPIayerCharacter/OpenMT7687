IC_CONFIG                             = mt7686
BOARD_CONFIG                          = mt7686_hdk

ifneq ($(wildcard $(strip $(SOURCE_DIR))/tools/secure_boot/),)
MTK_SECURE_BOOT_ENABLE      = y
else
MTK_SECURE_BOOT_ENABLE      = n
endif
