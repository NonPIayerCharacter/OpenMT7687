include feature.mk

MTK_WIFI_REPEATER_ENABLE            = y
MTK_WIFI_WPA3_SAE_ENABLE            = y
MTK_WIFI_PMF_ENABLE                 = y

# this macro will  take effect only after the MTK_WIFI_REPEATER_ENABLE to be set to y
MTK_WIFI_DHCPD_DHCP_COEXIST_ENABLE  = y
