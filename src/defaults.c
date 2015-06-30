#include <invram.h>

struct nvram_tuple defaults_config[] = 
{
	{ "netbiosname", "WNR612v2", 0 },
	
	/* Miscellaneous parameters */
	{ "endis_ntp", "1", 0 },
	{ "timer_interval", "3600", 0 },			/* Timer interval in seconds */
	{ "ntpserver1", "time-f.netgear.com", 0 }, 	/* NTP server */
	{ "ntpserver2", "time-g.netgear.com", 0 },
	{ "ntpadjust", "0", 0 },
	{ "dstflag", "0", 0 },
	{ "ntp_server", "GMT-0", 0 },
#ifdef REGION_NA
	{ "ntpserver_select", "GMT+8", 0 },
	{ "ntp_hidden_select", "4", 0 },
	{ "time_zone", "GMT+8", 0 },          /* Time zone (GNU TZ format) */
#elif defined(REGION_PR)
	{ "ntpserver_select", "GMT-8", 0 },
	{ "ntp_hidden_select", "31", 0 },
	{ "time_zone", "GMT-8", 0 }, 
#elif defined(REGION_RU)
	{ "ntpserver_select", "GMT-3", 0 },
	{ "ntp_hidden_select", "25", 0 },
	{ "time_zone", "GMT-3", 0 }, 
#elif defined(REGION_KO)
	{ "ntpserver_select", "GMT-9", 0 },
	{ "ntp_hidden_select", "33", 0 },
	{ "time_zone", "GMT-9", 0 }, 
#elif defined(REGION_PT)
	{ "ntpserver_select", "GMT+3", 0 },
	{ "ntp_hidden_select", "12", 0 },
	{ "time_zone", "GMT+3", 0 }, 
#else
	{ "ntpserver_select", "GMT-0", 0 },
	{ "ntp_hidden_select", "16", 0 },
	{ "time_zone", "GMT-0", 0 }, 
#endif
	{ "log_level", "0", 0 },                	/* Bitmask 0:off 1:denied 2:accepted */
	{ "is_default", "1", 0 },               	/* is it default setting: 1:yes 0:no*/
	{ "os_server", "", 0 },                 	/* URL for getting upgrades */
	{ "stats_server", "", 0 }, 			/* URL for posting stats */
	{ "console_loglevel", "1", 0 },		/* Kernel panics only */
	{ "raw_iface", "eth0", 0 },		/* Physical ethernet Device */ 
	{ "run_refresh", "no", 0},		/* for top ato refresh*/

	/* LAN Parameters */
	{ "true_lanif", "eth1", 0 },
	{ "lan_ifname", "br0", 0 },		/* LAN interface name */
	{ "lan_ifnames", "eth1 ath0", 0 },	/* Enslaved LAN interfaces */
	{ "lan_exifnames", "eth2", 0 },	/* Enslaved LAN interfaces */
	{ "lan_hwifname", "eth1", 0 },		//dvd.chen
	{ "lan_hwaddr", "", 0 },			/* LAN interface MAC address */
	{ "lan_dhcp", "1", 0 },
	{ "lan_proto", "dhcp", 0 },		/* [static|dhcp] */
	{ "lan_ipaddr", "192.168.0.1", 0 },	/* LAN IP address */
	{ "wan_dhcp_ipaddr", "0.0.0.0", 0 },	/* in DHCP mode, WAN IP address */
	{ "wan_dhcp_netmask", "0.0.0.0", 0 },
	{ "wan_dhcp_gateway", "0.0.0.0", 0 },
	{ "lan_netmask", "255.255.255.0", 0 },  /* LAN netmask */
	{ "lan_gateway", "0.0.0.0", 0 },		/* LAN gateway */
	{ "lan_route", "", 0 },	/* LAN default gateway */
	//{ "lan_stp", "1", 0 },		/* LAN spanning tree protocol */
	{ "lan_wins", "", 0 },		/* x.x.x.x x.x.x.x ... */
	{ "lan_domain", "", 0 },		/* LAN domain name */
	//{ "lan_lease", "86400", 0 },	/* LAN lease time in seconds */
	{ "lan_lease", "48", 0 },	//dvd.chen, unit:1800 sec 
	{ "lan_stp", "1", 0 },		/* LAN spanning tree protocol */
	{ "lan_route", "", 0 },		/* Static routes (ipaddr:netmask:gateway:metric:ifname ...) */
	{ "dhcp_start", "192.168.0.2" , 0 },
	{ "dhcp_end", "192.168.0.254" , 0 },
	{ "dhcp_route", "" , 0 },	//dvd.chen, for dhcpd.conf
	{ "dhcp_dns", "" , 0 },		//dvd.chen, for dhcpd.conf

	/* Wireless parameters */
	{ "wl_rxbuf", "128", 0 },			/* rxbuf used in driver */
	{ "wl_txbuf", "512", 0 },			/* txbuf used in driver */
	{ "wl_auto_antenna", "1", 0 },	/* 1 if enable smart antenna */
	{ "wl_fix_antenna", "1", 0 },		/* if wl_auto_antenna set to 1, this parameter will be used */
	{ "wl_ifname", "ath0", 0 },		/* Interface name */
	{ "wl_mode", "6", 0 },
	{ "wl_simple_mode", "1", 0 },		/* wlan mode, default value is 145Mbps  */
	{ "wl_usermode", "ap", 0 }, 
	{ "wl_sectype_ath0", "3", 0 },			/* 1:off 2:wep 3:psk 4:psks 5:psk/psk2 */
	{ "wl_sectype_ath1", "1", 0 },
	{ "wl_sectype_ath2", "1", 0 },
	{ "wl_sectype_ath3", "1", 0 },
	{ "wl_sectype_ath4", "1", 0 },
	{ "wl_hwaddr", "", 0 },			/* MAC address */
	{ "is_channel_auto","1", 0},
	{ "wl_ssid_ath0", "NETGEAR", 0 },		/* Service set ID (network name) */
	{ "wl_ssid_ath1", "NETGEAR_guest1", 0 },	
	{ "wl_ssid_ath2", "NETGEAR_guest2", 0 },	
	{ "wl_ssid_ath3", "NETGEAR_guest3", 0 },	
	{ "wl_ssid_ath4", "NETGEAR_guest4", 0 },	
	{ "wl_country_code","12",0 },		/* Country (default country DNI assigned )*/
#if defined(REGION_NA)
	{ "wl_country", "10", 0 },              /* Country (default obtained from driver) */
	{ "region_flag", "DISABLED", 0 },
#elif defined(REGION_PR)
	{ "wl_country", "12", 0 },              /* Country (default obtained from driver) */
	{ "region_flag", "", 0 },
#elif defined(REGION_RU)
	{ "wl_country", "13", 0 },              /* Country (default obtained from driver) */
	{ "region_flag", "", 0 },
#elif defined(REGION_KO)
	{ "wl_country", "7", 0 },              /* Country (default obtained from driver) */
	{ "region_flag", "", 0 },
#elif defined(REGION_PT)
	{ "wl_country", "9", 0 },              /* Country (default obtained from driver) */
	{ "region_flag", "", 0 },
#else
	{ "wl_country", "4", 0 },              /* Country (default obtained from driver) */
	{ "region_flag", "", 0 },
#endif
	{ "wl_endis_country_ie", "1", 0 },      /* Enable or Disable the country ie in Beacon packet */
	{ "wl_radio", "1", 0 },                 /* Enable (1) or disable (0) radio */
	{ "wl_closed", "0", 0 },                /* Closed (hidden) network */
	{ "wl_cwmmode", "0", 0 },               /* Channel Width Management 0-- 20MHz */
	{ "wl_wep", "disabled", 0 },            /* WEP data encryption (enabled|disabled) */
	{ "wl_auth_ath0", "2", 0 },                  /* Shared key authentication (0) or required (1) */
	{ "wl_auth_ath1", "2", 0 },   
	{ "wl_auth_ath2", "2", 0 },   
	{ "wl_auth_ath3", "2", 0 },   
	{ "wl_auth_ath4", "2", 0 },   
	{ "wl_key_ath0", "1", 0 },                   /* Current WEP key */
	{ "wl_key_ath1", "1", 0 }, 
	{ "wl_key_ath2", "1", 0 }, 
	{ "wl_key_ath3", "1", 0 }, 
	{ "wl_key_ath4", "1", 0 }, 
	{ "wl_key1_ath0", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key2_ath0", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key3_ath0", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key4_ath0", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key1_ath1", "", 0 },                   
	{ "wl_key2_ath1", "", 0 },                   
	{ "wl_key3_ath1", "", 0 },                   
	{ "wl_key4_ath1", "", 0 },
	{ "wl_key1_ath2", "", 0 },                   
	{ "wl_key2_ath2", "", 0 },                   
	{ "wl_key3_ath2", "", 0 },                   
	{ "wl_key4_ath2", "", 0 },
	{ "wl_key1_ath3", "", 0 },                   
	{ "wl_key2_ath3", "", 0 },                   
	{ "wl_key3_ath3", "", 0 },                   
	{ "wl_key4_ath3", "", 0 },
	{ "wl_key1_ath4", "", 0 },                   
	{ "wl_key2_ath4", "", 0 },                   
	{ "wl_key3_ath4", "", 0 },                   
	{ "wl_key4_ath4", "", 0 },
	{ "wl_disa_coex","0", 0 },
	
	{ "wl_maclist", "", 0 },                /* xx:xx:xx:xx:xx:xx ... */
	{ "wl_macmode", "disabled", 0 },      /* "allow" only, "deny" only, or "disabled" */
	{ "wl_allowlist","", 0 },               /* allowlist cleaned */
	{ "wl_denylist","", 0 },                /* denylist cleaned  */
	{ "wl_channel", "0", 0 },              /* Channel number */
	{ "wl_hidden_channel","0",0 },         /*for webpage */ 
	{ "wl_rate", "auto", 0 },                       /* Rate (bps, 0 for auto) */
	{ "wl_frag", "2346", 0 },               /* Fragmentation threshold */
	{ "wla_frag", "2346", 0 },
	{ "wl_rts", "2347", 0 },                /* RTS threshold */
	{ "wla_rts", "2347", 0 },
	{ "wl_dtim", "1", 0 },                  /* DTIM period */
	{ "wl_bcn", "100", 0 },                 /* Beacon interval */
	{ "wl_plcphdr", "auto", 0 },            /* 802.11b PLCP preamble type */
	{ "wla_plcphdr", "auto", 0 },
	{ "wl_txctrl", "0", 0 },
	{ "wla_txctrl", "100", 0 },
	{ "wl_txctrl_web", "0", 0 },            /* Transmit Power Control for webpage */
	{ "wla_txctrl_web", "0", 0 },
	{ "wl_afterburner", "off", 0 },         /* AfterBurner */
	{ "wl_frameburst", "off", 0 },          /* BRCM Frambursting mode (off|on) */
	{ "wl_wme", "1", 0 },                   /* WME mode (Enable|Disable) */

	/* WPA parameters */
	{ "wl_auth_mode", "none", 0 },          /* Network authentication mode (radius|none) */
	{ "wl_wpa_psk_ath0", "", 0 },                /* WPA pre-shared key */
	{ "wl_wpa_psk_ath1", "", 0 },
	{ "wl_wpa_psk_ath2", "", 0 },
	{ "wl_wpa_psk_ath3", "", 0 },
	{ "wl_wpa_psk_ath4", "", 0 },
	{ "wl_wpa1_psk_ath0", "", 0 },
	{ "wl_wpa1_psk_ath1", "", 0 },
	{ "wl_wpa1_psk_ath2", "", 0 },
	{ "wl_wpa1_psk_ath3", "", 0 },
	{ "wl_wpa1_psk_ath4", "", 0 },
	{ "wl_wpa2_psk_ath0", "", 0 },                /* WPA pre-shared key */
	{ "wl_wpa2_psk_ath1", "", 0 }, 
	{ "wl_wpa2_psk_ath2", "", 0 }, 
	{ "wl_wpa2_psk_ath3", "", 0 }, 
	{ "wl_wpa2_psk_ath4", "", 0 }, 
	{ "wl_wpas_psk_ath0", "", 0 },                /* WPA pre-shared key */
	{ "wl_wpas_psk_ath1", "", 0 },
	{ "wl_wpas_psk_ath2", "", 0 },
	{ "wl_wpas_psk_ath3", "", 0 },
	{ "wl_wpas_psk_ath4", "", 0 },
	{ "wl_wpa_gtk_rekey", "0", 0 },          /* WPA GTK rekey */
	{ "wl_radius_ipaddr", "", 0 },           /* RADIUS server IP address */
	{ "wl_radius_key", "", 0 },              /* RADIUS shared secret */
	{ "wl_radius_port", "1812", 0 },         /* RADIUS server UDP port */
	{ "wl_crypto", "tkip", 0 },              /* WPA data encryption */
	{ "wl_net_reauth", "36000", 0 },         /* Network Re-auth/PMK caching duration */
	{ "wl_akm", "", 0 },                     /* WPA akm list */
	{ "endis_xr", "1", 0 },			/*Enable eXtended Range(XR) Feature*/	
	{ "endis_108", "0", 0 },		/*Disable Advanced 108Mbps Features*/
	{ "endis_wl_wmm", "1", 0},      /*Enable WMM*/
	{ "endis_wla_wmm", "0", 0},
	{ "endis_pin", "0", 0},
	{ "wla_endis_pin", "0", 0},
	{ "endis_wsc_config", "0", 0},

	/* WPS */ 
	{ "wps_status", "1",0 },
	{ "wla_wps_status", "1",0 },
	{ "wps_client", "", 0 },        /* wps_client status */
	{ "WPS_type", "0", 0 },         /* wps_type, 0 for push button, 1 for pin num */
	{ "endis_wl_wps", "1", 0 },
	{ "endis_wla_wps", "1", 0 },


	/* WME parameters */
	/* EDCA parameters for STA */
	{ "wl_wme_sta_bk", "15 1023 7 0 0 off", 0 },    /* WME STA AC_BK paramters */
	{ "wl_wme_sta_be", "15 1023 3 0 0 off", 0 },    /* WME STA AC_BE paramters */
	{ "wl_wme_sta_vi", "7 15 2 6016 3008 off", 0 }, /* WME STA AC_VI paramters */
	{ "wl_wme_sta_vo", "3 7 2 3264 1504 off", 0 },  /* WME STA AC_VO paramters */

	/* EDCA parameters for AP */
	{ "wl_wme_ap_bk", "15 1023 7 0 0 off", 0 },     /* WME AP AC_BK paramters */
	{ "wl_wme_ap_be", "15 63 3 0 0 off", 0 },       /* WME AP AC_BE paramters */
	{ "wl_wme_ap_vi", "7 15 1 6016 3008 off", 0 },  /* WME AP AC_VI paramters */
	{ "wl_wme_ap_vo", "3 7 1 3264 1504 off", 0 },   /* WME AP AC_VO paramters */

	{ "wl_wme_no_ack", "off", 0},           /* WME No-Acknowledgmen mode */
	/*wpas*/
	{ "wl_sec_wpaphrase_len", "0", 0 },
	/*wep*/
	{ "key_length", "0", 0 },

	/* WEP parameters */            /* clear all configurations by user */
	{ "wl_key_length_ath0", "5", 0 },
  { "wl_key_length_ath1", "5", 0 },
  { "wl_key_length_ath2", "5", 0 },
  { "wl_key_length_ath3", "5", 0 },
  { "wl_key_length_ath4", "5", 0 },
  
	{ "wl_wep_64_key1_ath0", "", 0 },
	{ "wl_wep_64_key2_ath0", "", 0 },
	{ "wl_wep_64_key3_ath0", "", 0 },
	{ "wl_wep_64_key4_ath0", "", 0 },
	{ "wl_wep_128_key1_ath0", "", 0 },
	{ "wl_wep_128_key2_ath0", "", 0 },
	{ "wl_wep_128_key3_ath0", "", 0 },
	{ "wl_wep_128_key4_ath0", "", 0 },

  { "wl_wep_64_key1_ath1", "", 0 },
	{ "wl_wep_64_key2_ath1", "", 0 },
	{ "wl_wep_64_key3_ath1", "", 0 },
	{ "wl_wep_64_key4_ath1", "", 0 },
	{ "wl_wep_128_key1_ath1", "", 0 },
	{ "wl_wep_128_key2_ath1", "", 0 },
	{ "wl_wep_128_key3_ath1", "", 0 },
	{ "wl_wep_128_key4_ath1", "", 0 },
	
	{ "wl_wep_64_key1_ath2", "", 0 },
	{ "wl_wep_64_key2_ath2", "", 0 },
	{ "wl_wep_64_key3_ath2", "", 0 },
	{ "wl_wep_64_key4_ath2", "", 0 },
	{ "wl_wep_128_key1_ath2", "", 0 },
	{ "wl_wep_128_key2_ath2", "", 0 },
	{ "wl_wep_128_key3_ath2", "", 0 },
	{ "wl_wep_128_key4_ath2", "", 0 },
	
	{ "wl_wep_64_key1_ath3", "", 0 },
	{ "wl_wep_64_key2_ath3", "", 0 },
	{ "wl_wep_64_key3_ath3", "", 0 },
	{ "wl_wep_64_key4_ath3", "", 0 },
	{ "wl_wep_128_key1_ath3", "", 0 },
	{ "wl_wep_128_key2_ath3", "", 0 },
	{ "wl_wep_128_key3_ath3", "", 0 },
	{ "wl_wep_128_key4_ath3", "", 0 },
	
	{ "wl_wep_64_key1_ath4", "", 0 },
	{ "wl_wep_64_key2_ath4", "", 0 },
	{ "wl_wep_64_key3_ath4", "", 0 },
	{ "wl_wep_64_key4_ath4", "", 0 },
	{ "wl_wep_128_key1_ath4", "", 0 },
	{ "wl_wep_128_key2_ath4", "", 0 },
	{ "wl_wep_128_key3_ath4", "", 0 },
	{ "wl_wep_128_key4_ath4", "", 0 },
  
	/* WPA/WPA2 Enterprise */
        { "wl_wpae_mode", "WPAE-TKIPAES", 0 },          /* WPA Mode for WPA/WPA2-Enterprise */
	{ "wl_radiusSerIp", "", 0 },           /* RADIUS server IP address */
	{ "wl_radiusSecret", "", 0 },              /* RADIUS shared secret */
	{ "wl_radiusPort", "1812", 0 },         /* RADIUS server UDP port */

#if 0   /*  support wireless a/n, prefix="wla_" , a/n & b/g/n use the same country code */

	{ "wla_ssid", "NETGEAR", 0 },		/* Service set ID (network name) */
	{ "wla_radio", "0", 0 },                 /* Enable (1) or disable (0) radio */
	{ "wla_closed", "0", 0 },                /* Closed (hidden) network */
	{ "wla_cwmmode", "0", 0 },               /* Channel Width Management 0-- 20MHz */
	{ "wla_wep", "disabled", 0 },            /* WEP data encryption (enabled|disabled) */
	{ "wla_auth", "2", 0 },                  /* Shared key authentication (0) or required (1) */
	{ "wla_key", "1", 0 },                   /* Current WEP key */
	{ "wla_key1", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wla_key2", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wla_key3", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wla_key4", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wla_maclist", "", 0 },                /* xx:xx:xx:xx:xx:xx ... */
	{ "wla_macmode", "disabled", 0 },      /* "allow" only, "deny" only, or "disabled" */
	{ "wla_allowlist","", 0 },               /* allowlist cleaned */
	{ "wla_denylist","", 0 },                /* denylist cleaned  */
	{ "wla_channel", "0", 0 },              /* Channel number */
	{ "wla_hidden_channel","0",0 },         /*for webpage */ 
    	{ "wla_mode", "3", 0 },
	{ "wla_simple_mode", "3", 0 },           /* wlan mode, default value is 300Mbps  */
	{ "wla_usermode", "ap", 0 },
	{ "wla_sectype", "1", 0 },                       /* 0:off 1:wep 2:psk 3:psks 4:psk/psk2 */
	{ "wla_enable_video_value", "0", 0},
	{ "wla1_enable_video_value", "0", 0},

	/* WPA parameters */
	{ "wla_auth_mode", "none", 0 },          /* Network authentication mode (radius|none) */
	{ "wla_wpa_psk", "", 0 },                /* WPA pre-shared key */
	{ "wla_wpa1_psk", "", 0 },
	{ "wla_wpa2_psk", "", 0 },                /* WPA pre-shared key */
	{ "wla_wpas_psk", "", 0 },                /* WPA pre-shared key */
	{ "wla_wpa_gtk_rekey", "0", 0 },          /* WPA GTK rekey */

	/* WEP parameters */            /* clear all configurations by user */
	{ "wla_key_length", "64", 0 },
	{ "wla_wep_64_key1", "", 0 },
	{ "wla_wep_64_key2", "", 0 },
	{ "wla_wep_64_key3", "", 0 },
	{ "wla_wep_64_key4", "", 0 },
	{ "wla_wep_128_key1", "", 0 },
	{ "wla_wep_128_key2", "", 0 },
	{ "wla_wep_128_key3", "", 0 },
	{ "wla_wep_128_key4", "", 0 },

	/* WPA/WPA2 Enterprise */
        { "wla_wpae_mode", "WPAE-TKIPAES", 0 },          /* WPA Mode for WPA/WPA2-Enterprise */
	{ "wla_radiusSerIp", "", 0 },           /* RADIUS server IP address */
	{ "wla_radiusSecret", "", 0 },              /* RADIUS shared secret */
	{ "wla_radiusPort", "1812", 0 },         /* RADIUS server UDP port */
	
#endif	

#if 0  /*  support wireless Guest Network prefix="wlg1_" */

	{ "wlg1_ssid", "NETGEAR_Guest1", 0 },		/* Service set ID (network name) */
	{ "wlg1_wep", "disabled", 0 },            /* WEP data encryption (enabled|disabled) */
	{ "wlg1_auth", "2", 0 },                  /* Shared key authentication (0) or required (1) */
	{ "wlg1_key", "1", 0 },                   /* Current WEP key */
	{ "wlg1_key1", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wlg1_key2", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wlg1_key3", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wlg1_key4", "", 0 },                   /* 5/13 char ASCII or 10/26 char hex */
	{ "wlg1_sectype", "1", 0 },                       /* 0:off 1:wep 2:psk 3:psks 4:psk/psk2 */


	/* WPA parameters */
	{ "wlg1_auth_mode", "none", 0 },          /* Network authentication mode (radius|none) */
	{ "wlg1_wpa_psk", "", 0 },                /* WPA pre-shared key */
	{ "wlg1_wpa1_psk", "", 0 },
	{ "wlg1_wpa2_psk", "", 0 },                /* WPA pre-shared key */
	{ "wlg1_wpas_psk", "", 0 },                /* WPA pre-shared key */
	{ "wlg1_wpa_gtk_rekey", "0", 0 },          /* WPA GTK rekey */

	/* WEP parameters */            /* clear all configurations by user */
	{ "wlg1_key_length", "64", 0 },
	{ "wlg1_wep_64_key1", "", 0 },
	{ "wlg1_wep_64_key2", "", 0 },
	{ "wlg1_wep_64_key3", "", 0 },
	{ "wlg1_wep_64_key4", "", 0 },
	{ "wlg1_wep_128_key1", "", 0 },
	{ "wlg1_wep_128_key2", "", 0 },
	{ "wlg1_wep_128_key3", "", 0 },
	{ "wlg1_wep_128_key4", "", 0 },	
	{ "wlg1_endis_guestNet", "0", 0 },        	/* Enable Guest Network */
	{ "wlg1_endis_guestSSIDbro", "1", 0 },   		/* Enable SSID Broadcast */
	{ "wlg1_endis_allow_guest", "0", 0 },       	/* Allow Guest to access MY Local Network */
	{ "wlg1_wpae_mode", "WPAE-TKIPAES", 0 },          /* WPA Mode for WPA/WPA2-Enterprise */
	{ "wlg1_radiusSerIp", "", 0 },           /* RADIUS server IP address */
	{ "wlg1_radiusSecret", "", 0 },              /* RADIUS shared secret */
	{ "wlg1_radiusPort", "1812", 0 },         /* RADIUS server UDP port */

#endif


	
	/* WAN Settings */
	/* WAN H/W parameters */
	{ "wan_ifname", "eth0", 0 },                /* WAN interface name */
	{ "wan_ifnames", "eth0", 0 },               /* WAN interface names */
	{ "wan_hwname", "", 0 },                /* WAN driver name (e.g. et1) */
	{ "wan_hwaddr", "", 0 },                /* WAN interface MAC address */
	{ "wan_hwifname", "eth0", 0 },			//dvd.chen

	/* WAN TCP/IP parameters */
	{ "wan_status", "0", 0 },				/* wan enable or disable mark */
	{ "wan_proto", "pppoe", 0 },             /* [static|dhcp|pppoe|disabled] */
	{ "wan_ipaddr", "0.0.0.0", 0 },         /* WAN IP address */
	{ "wan_netmask", "0.0.0.0", 0 },        /* WAN netmask */
	{ "wan_gateway", "0.0.0.0", 0 },        /* WAN gateway */
	{ "wan_dns", "", 0 },                   /* x.x.x.x x.x.x.x ... */
	{ "wan_wins", "", 0 },                  /* x.x.x.x x.x.x.x ... */
	{ "wan_hostname", "WNR612v2", 0 },              /* WAN hostname */
	{ "wan_domain", "", 0 },                /* WAN domain name */
	{ "wan_lease", "86400", 0 },            /* WAN lease time in seconds */

	/*wan setup*/
	{"wan_endis_spi","1",0},
	{"wan_endis_sipalg","0",0},
	{"wan_endis_dmz","0",0},
	{"wan_endis_rspToPing","1",0},
	{"wan_nat_fitering","0",0},
	{"wan_pptp_mtu","1436",0},
	{"wan_dhcp_mtu","1500",0},
	{"wan_endis_dod","1",0}, /*Connect Automatically, as Required*/
	{"ipv6_pass","0",0},
	/* PPPoE parameters */
	{ "wan_pppoe_ifname", "", 0 },          /* PPPoE enslaved interface */
	{ "wan_pppoe_username", "", 0 },        /* PPP username */
	{ "wan_pppoe_passwd", "", 0 },          /* PPP password */
	{ "wan_pppoe_idletime", "0", 0 },      /* Dial on demand max idle time (seconds) */
	{ "wan_pppoe_keepalive", "0", 0 },      /* Restore link automatically */
	{ "wan_pppoe_demand", "0", 0 },         /* Dial on demand */
	{ "wan_pppoe_mru", "1492", 0 },         /* Negotiate MRU to this value */
	{ "wan_pppoe_mtu", "1492", 0 },         /* Negotiate MTU to the smaller of this value or the
						   peer MRU */
	{ "wan_pppoe_service", "DOM.RU", 0 },         /* PPPoE service name */
	{ "wan_pppoe_ac", "DOM.RU", 0 },              /* PPPoE access concentrator name */
	{ "wan_ether_dns1", "", 0 },
	{ "wan_ether_dns2", "", 0 },
	{ "wan_pppoe_mac_assign", "0", 0},
	{ "wan_pppoe_this_mac", "", 0 },
	{ "wan_autodns", "1", 0 },
	
	/*wan bpa*/
	{ "internet_type", "1", 0 },
	{ "internet_ppp_type", "0", 0 },
	{ "wan_bpa_username", "", },
	{ "wan_bpa_password", "", 0 },
	{ "wan_bpa_demand", "1", 0 },         /* Dial on demand */
	{ "wan_bpa_idle_time", "300", 0 },
	{ "wan_bpa_servicename", "login-server", 0 },
	{ "wan_bpa_dns_assign", "0", 0 },
	{ "wan_bpa_mac_assign", "0", 0 },
	{ "wan_bpa_this_mac", "", 0 },

	/*wan_pppoe*/
	{ "wan_pppoe_wan_assign", "0", 0 },
	{ "wan_pppoe_ip", "", 0 },
	{ "wan_pppoe_dns_assign", "0", 0 },
	{ "wan_pppoe_netmask", "", 0 },
	{ "enable_dual_access", "0", 0 },
	{ "wan_pppoe_dual_assign", "0", 0 },
	{ "wan_pppoe_dual_ip", "", 0 },
	{ "wan_pppoe_dual_netmask", "", 0 },
	{ "wan_pppoe_dual_gateway", "", 0 },
	{ "pppoe_dynamic_dual_mode", "0", 0 },

	/*wan pptp*/
	{ "wan_pptp_username", "", 0 },
	{ "wan_pptp_password", "", 0 },
	{ "wan_pptp_demand", "1", 0 },         /* Dial on demand */
	{ "wan_pptp_idle_time", "300", 0 },
	{ "wan_pptp_local_ip", "", 0 },
	{ "wan_pptp_netmask", "255.255.255.0", 0 },
	{ "wan_pptp_server_ip", "10.0.0.138", 0 },
	{ "pptp_gw_static_route", "", 0 },
	{ "wan_pptp_connection_id", "", 0 },
	{ "wan_pptp_dns_assign", "0", 0 },
	{ "wan_pptp_mac_assign", "0", 0},
	{ "wan_pptp_this_mac", "", 0 },
	{ "wan_pptp_wan_assign", "0", 0 }, 
	{ "wan_pptp_static_dns", "0", 0 },
	
	/*wan l2tp*/
	{ "wan_l2tp_mtu","1430",0},
	{ "wan_l2tp_username", "", 0 },
	{ "wan_l2tp_password", "", 0 },
	{ "wan_l2tp_demand", "1", 0 },         /* Dial on demand */
	{ "wan_l2tp_idle_time", "300", 0 },
	{ "wan_l2tp_local_ip", "", 0 },
	{ "wan_l2tp_netmask", "255.255.255.0", 0 },
	{ "wan_l2tp_server_ip", "", 0 },
	{ "wan_l2tp_gateway_ip", "", 0 },
	{ "wan_l2tp_connection_id", "", 0 },
	{ "wan_l2tp_dns_assign", "0", 0 },
	{ "wan_l2tp_mac_assign", "0", 0},
	{ "wan_l2tp_this_mac", "", 0 },
	{ "wan_l2tp_wan_assign", "0", 0 }, 
	

	/*dhcp/static*/
	{ "wan_ether_wan_assign", "0", 0 },
	{ "wan_ether_dns_assign", "0", 0 },
	{ "wan_ether_mac_assign", "0", 0 },
	{ "wan_ether_this_mac", "", 0 },

	/*wan mac */
	{ "wan_factory_mac", "", 0 },
	{ "wan_remote_mac", "", 0 },

	/*remote PC's IP */
	{ "remote_ip", "", 0 },

	/* Big switches */
	{ "router_disable", "0", 0 },           /* lan_proto=static lan_stp=0 wan_proto=disabled */
	{ "fw_disable", "0", 0 },               /* Disable firewall(allow new connections from the WAN) */
	{ "nat_disable", "0", 0 },			//dvd.chen added for wnr612v2

	/* Filters */
	{ "filter_maclist", "", 0 },            /* xx:xx:xx:xx:xx:xx ... */
	{ "filter_macmode", "deny", 0 },        /* "allow" only, "deny" only, or "disabled" (allow all) */
	{ "filter_client0", "", 0 },            /* [lan_ipaddr0-lan_ipaddr1|*]:lan_port0-lan_port1,proto,enable,day_start-day_end,sec_start-sec_end,desc */

	/* Port forwards */
	{ "dmz_ipaddr", "", 0 },                /* x.x.x.x (equivalent to 0-60999>dmz_ipaddr:0-60999) */
	//{ "forward_port0", "", 0 },             /* wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1[:,]proto[:,]enable[:,]desc */
	//{ "autofw_port0", "", 0 },              /* out_proto:out_port,in_proto:in_port0-in_port1>to_ port0-to_port1,enable,desc */
	{ "port_forwarding_num", "0", 0 },	//dvd.chen added
	{ "port_trigger_num", "0", 0 },		//dvd.chen added

	/*email*/
	{ "email_notify", "0", 0 },
	{ "email_addr", "", 0 },
	{ "email_smtp", "", 0 },
	{ "email_username", "", 0 },
	{ "email_password", "", 0 },
	{ "email_from_assign", "0", 0 },
	{ "email_this_addr", "", 0 },
	{ "email_send_alert", "1", 0 },
	{ "email_cfAlert_Select", "0", 0 },
	{ "email_cfAlert_Day", "0", 0 },
	{ "email_schedule_hour", "0", 0 },
	{ "email_ntpadjust", "0", 0 },
	{ "email_ntpserver", "GMT-0", 0 },
	{ "email_endis_auth", "0", 0 },
	{ "email_support_ssl", "0", 0},
	{ "product_id", "WNR612v2", 0 },
	/*port_forwarding*/
	{ "port_forward_trigger", "0", 0},
	{ "disable_port_trigger", "0", 0},
	{ "porttrigger_timeout","20",0}, /*Port Triggering Timeout (in minutes)*/
	/*remote*/
	{ "remote_endis","1", 0 },
	{ "remote_access","2", 0 },
	{ "remote_iplist","", 0 },
	{ "remote_port","8080", 0 },

	/*upnp*/
	{ "upnp_enable", "1", 0 },              /* Start UPnP */
	{ "upnp_AdverTime", "1800", 0 },
	{ "upnp_TimeToLive", "4", 0 },

	/*ddns*/
	{ "endis_ddns","0",0 },
	{ "sysDNSHost","",0 },
	{ "sysDNSUser","",0 },
	{ "sysDNSPassword","",0 },
	{ "endis_wildcards","0",0 },
	{ "oray_ddns_server","phlinux3.oray.net",0 },

	/* block sites*/
	{ "block_skeyword","0",0 },
	{ "block_endis_Trusted_IP","0",0 },
	{ "block_KeyWord_DomainList", "", 0 },
	{ "block_trustedip", "", 0 }, 
	/* block service*/
	{ "blockserv_ctrl", "0", 0 },
	/* block schedule*/
	{ "schedule_days_flag", "0", 0 },
	{ "schedule_days_to_block", "everyday", 0 },
	{ "schedule_all_day", "1", 0 },
	{ "schedule_end_block_time", "23:59", 0 },
	{ "hidden_schedule_end_block_time","24:00",0},
	{ "schedule_start_block_time", "00:00", 0 },
	/* for firewall*/
	{ "forfirewall", "remote_management", 0 },
	/* ripd */
	{ "ripd_enable", "0", 0 },
	{ "rip_direction", "0", 0 },	/* set 0 show "Both"*/
	{ "rip_version","0",0 },

	/* Web server parameters */
	{ "http_username", "admin", 0 },                /* Username */
	{ "http_passwd", "password", 0 },               /* Password */
	{ "http_wanport", "", 0 },              /* WAN port to listen on */
	{ "http_lanport", "80", 0 },            /* LAN port to listen on */

	{ "lltd_enable", "0", 0 },

	/* Restore defaults */
	{ "restore_defaults", "0", 0 },         /* Set to 0 to not restore defaults on boot */
	/*auto upgrade from web */
	{ "auto_check_for_upgrade","1",0}, /*set 1 to auto upgrade from web */

	/* show welcome page */
	{  "thank_login", "1" , 0 } ,  /*set 0 to show welcome page*/         

	/*mulpppoe*/
	{ "wan_mulppp_mtu", "1454", 0}, /* NG-SPEC: the MTU of session 2 is set to 1454. Default Session 1 1454.*/
	{ "count_mulpppoe","0",0}, /*rule number for mulpppoe*/
	{ "wan_mulpppoe1_dns_assign", "0",0},
	{ "wan_mulpppoe1_idletime","300",0 },
	{ "wan_mulpppoe1_ip","",0 },
	{ "wan_mulpppoe1_passwd","",0 },
	{ "wan_mulpppoe1_service","",0 },
	{ "wan_mulpppoe1_username" , "" , 0 },
	{ "wan_mulpppoe1_wan_assign" , "0", 0 },
	{ "wan_mulpppoe2_east_password", "guest",0 },
	{ "wan_mulpppoe2_east_username", "guest@flets", 0 },
	{ "wan_mulpppoe2_other_password", "" , 0 },
	{ "wan_mulpppoe2_other_username", "guest" , 0 },
	{ "wan_mulpppoe2_password", "" , 0 },
	{ "wan_mulpppoe2_policy", "0", 0 },
	{ "wan_mulpppoe2_servicename", "" , 0 },
	{ "wan_mulpppoe2_session", "0",0 },
	{ "wan_mulpppoe2_username", "",0 },
	{ "wan_mulpppoe2_west_password", "flets",0},
	{ "wan_mulpppoe2_west_username", "flets@flets", 0},
	/*dns_hijack*/
	{ "dns_hijack", "0", 0},
	{ "blank_status", "", 0},
	/*wlacl*/
	{ "wl_access_ctrl_on", "0" , 0},
	{ "wla_access_ctrl_on", "0" , 0},
	/*ddns*/
	{ "sysDNSProviderlist", "0", 0},	
	{ "change_wan_type", "1", 0 },
	/*wds*/
	{ "wds_endis_fun", "0", 0},
	{ "wla_wds_endis_fun", "0", 0},

	{ "wds_repeater_basic", "0", 0 },
	{ "repeater_ip" , "0.0.0.0",0},
	{ "old_lan_ipaddr", "192.168.0.1",0},
	{ "wds_endis_ip_client","0",0},
	{ "basic_station_mac", "", 0},
	{ "wds_endis_mac_client" , "0" , 0},
	{ "repeater_mac1" , "", 0},
	{ "repeater_mac2" , "", 0},
	{ "repeater_mac3" , "", 0},
	{ "repeater_mac4" , "", 0},
	{ "repeater_mac1_a" , "", 0},
        { "repeater_mac2_a" , "", 0},
        { "repeater_mac3_a" , "", 0},
        { "repeater_mac4_a" , "", 0},

	{ "wds_repeater_basic_a", "0", 0 },
	{ "endis_ssid_broadcast_ath0", "1" , 0},
	{ "endis_ssid_broadcast_ath1", "1" , 0},
	{ "endis_ssid_broadcast_ath2", "1" , 0},
	{ "endis_ssid_broadcast_ath3", "1" , 0},
	{ "endis_ssid_broadcast_ath4", "1" , 0},
	{ "endis_wlan_iso_ath0", "0" , 0},
	{ "endis_wlan_iso_ath1", "0" , 0},
	{ "endis_wlan_iso_ath2", "0" , 0},
	{ "endis_wlan_iso_ath3", "0" , 0},
	{ "endis_wlan_iso_ath4", "0" , 0},
	{ "wla_endis_ssid_broadcast", "1" , 0},
	{ "endis_wl_radio_ath0", "1" , 0}, 
	{ "endis_wl_radio_ath1", "0" , 0}, 
	{ "endis_wl_radio_ath2", "0" , 0}, 
	{ "endis_wl_radio_ath3", "0" , 0}, 
	{ "endis_wl_radio_ath4", "0" , 0}, 
	{ "endis_wla_radio", "0" , 0},
	{ "endis_allow_guest_ath1", "0", 0 },       	/* Allow Guest to access MY Local Network */
	{ "endis_allow_guest_ath2", "0", 0 },
	{ "endis_allow_guest_ath3", "0", 0 },
	{ "endis_allow_guest_ath4", "0", 0 },
	

	/*traffic meter*/
	{ "endis_traffic" , "0", 0},
	{ "ctrl_volumn_time" , "0",0},
	{ "restart_counter_time", "0:00",0},
	{ "traffic_restart_day","1",0},
	{ "left_time_volumn","0",0},
	{ "traffic_led","0",0},
	{ "traffic_block_all","0",0},
	{ "limit","0",0},
	{ "mon_volumn_limit","0",0},
	{ "round_up","0",0},
	{ "mon_time_limit","0",0},
	{ "show_traffic_timereset","10",0},
	{ "warning_once","0",0},	

	{ "timereset","5",0},

	/* QoS */
	{ "qos_endis_wmm", "1", 0 },
	{ "qos_endis_on", "0" , 0 },
	{ "qos_uprate", "256" , 0 },
	{ "qos_uprate_det", "256" , 0 },
	{ "qos_threshold", "0", 0 },
	{ "qos_auto_bandwidth", "0", 0 },
	{ "qos_bw_mode", "0", 0 },
	{ "qos_list_default", "1", 0 },
	{ "qos_list1", "MSN_messenger 0 MSN_messenger 1 TCP 443 443 ---- ----", 0 },
	//{ "qos_list2", "Skype 0 Skype 0 TCP/UDP 80,443 80,443 ---- ----", 0 },
	{ "qos_list2", "Yahoo_Messenger 0 Yahoo_messenger 1 TCP 5050,5100,11999 5050,5100,11999 ---- ----", 0 },
	{ "qos_list3", "IP_Phone 0 IP_Phone 0 TCP 6670 6670 ---- ----", 0 },
	{ "qos_list4", "Vonage_IP_Phone 0 Vonage_IP_Phone 0 UDP 53,69,5060 53,69,5061 ---- ----", 0 },
	{ "qos_list5", "NetMeeting 0 Netmeeting 1 TCP 1720 1720 ---- ----", 0 },
	{ "qos_list6", "AIM 0 AIM 1 TCP 5190 5190 ---- ----", 0 },
	{ "qos_list7", "Google_Talk 0 Google_Talk 0 TCP 443 443 ---- ----", 0 },
	{ "qos_list8", "Netgear_EVA 0 Netgear_EVA 0 UDP 49152 49155 ---- ----", 0 },//later add
	{ "qos_list9", "Counter-Strike 1 Counter-Strike 1 UDP 27015 27019 ---- ----", 0 },
	{ "qos_list10", "Age-of-Empires 1 Age-of-Empires 1 TCP/UDP 47624 47624 ---- ----", 0 },
	//{ "qos_list11", "Diablo-II 1 Diablo-II 1 TCP 6122 6122 ---- ----", 0 },
	{ "qos_list11", "Everquest 1 Everquest 1 TCP 7000 7000 ---- ----", 0 },
	//{ "qos_list13", "Half-Life 1 Half-Life 1 TCP/UDP 27015 27015 ---- ----", 0 },
	{ "qos_list12", "Quake-2 1 Quake-2 1 TCP/UDP 27960 27960 ---- ----", 0 },
	{ "qos_list13", "Quake-3 1 Quake-3 1 TCP/UDP 27960 27960 ---- ----", 0 },
	{ "qos_list14", "Unreal-Tourment 1 Unreal-Tourment 1 TCP/UDP 7777,27960 7783,27960 ---- ----", 0 },
	{ "qos_list15", "Warcraft 1 Warcraft 1 TCP 6112 6112 ---- ----", 0 },
	//{ "qos_list18", "Return-to-Castle-Wolfenstein 1 Return-to-Castle-Wolfenstein 1 TCP 27950,27960,27965,27952 27950,27960,27965,27952 ---- ----", 0 },
	{ "qos_rule_count", "18", 0 }, 
	{ "qos_su_ip", "0.0.0.0", 0 },
	{ "qos_enable_su", "0", 0 },
	{ "endis_telnet", "0", 0 },
	{ "scienario", "0", 0 },
	{ "scienario2", "0", 0 },

	{ "enet_rxbuf", "252", 0 },
	{ "enet_txbuf", "128", 0 },

	/* USB */
	/* Storage Administration */
	{ "admin_userAdmin", "admin admin admin admin admin 1" },
	{ "admin_userGuest", "guest guest guest guest guest 0" },

	{ "Enable_GUIStringTable","1",0},
#if defined(REGION_NA)
	{ "GUI_Region","English",0},
#elif defined(REGION_PR)
	{ "GUI_Region","Chinese",0},
#elif defined(REGION_RU)
	{ "GUI_Region","Russian",0},
#elif defined(REGION_KO)
	{ "GUI_Region","Korean",0},
#elif defined(REGION_PT)
	{ "GUI_Region","Portuguese",0},
#else
	{ "GUI_Region","English",0},
#endif
	{ "lang_available","1 2 3",0},	/*Support English,German,Chinese */
        { "GUI_Region2","English",0},/* old GUI_Region */
        { "StringTable_default_Ver","V1.0.0.1",0},
        { "StringTable_download_Ver","V1.0.0.1",0},

	/* WatchDog */
	{ "endis_watchdog", "1", 0 },

	/*FTP server */
	{ "ftp_enabled", "0", 0},
	{ "ftp_enable_internet", "0", 0},
	{ "ftp_port", "21", 0},

	/* UPnP Media server */
	{ "upnp_enable_upnp","0", 0 },
	{ "upnp_enable_autoScan","0", 0 },
	{ "upnp_scanTime", "12", 0 },
	{ "upnp_lastScanTime","", 0 }, 
	{ "upnp_scan_shareName","***",0 },

	{ "usb_deviceName","WNR612v2",0},
	{ "usb_workGroup","Workgroup",0},
	{ "usb_enableNet","0",0},
	{ "usb_enableHTTP","0",0},
	{ "usb_enableHvia","1",0},	
	{ "usb_enableUSB","0",0},
	{ "usb_HTTP_via_port","80",0},
	{ "usb_enableFTP","1",0},
	{ "usb_enableFvia","1",0},
	{ "usb_FTP_via_port","21",0},
	{ "usbDeviceName","/mnt/sda1",0},

	{ "ant_g_mode","1",0},
	{ "ant_a_mode","1",0},
	{ "ant_g_select","1",0},
	{ "ant_a_select","2",0},

	/* Parental Control */
	{ "ParentalControl", "0", 0},
	{ "ParentalControl_table", "0,", 0},
	/* Igmpproxy Control */
	{ "igmp_enable", "0", 0},
	
	/* TR069 */
	{ "agent_enable", "0", 0},
	{ "TR069_PeriodicInformEnable", "true", 0},
	{ "TR069_PeriodicInformInterval", "300", 0},	//default is 300
	//{ "TR069_ManagementServerURL", "https://acs.ertelecom.ru/tr069", 0},
	{ "TR069_ManagementServerURL", "http://acs.avsystem.com:10301/acs/", 0}, //no SSL
	//{ "TR069_ManagementServerURL", "https://acs.avsystem.com:10302/acs/", 0}, //with SSL
	//{ "TR069_ManagementServerUname", "ertclient69", 0},
	//{ "TR069_ManagementServerPWD", "0CJz2uCvbI", 0},
	{ "TR069_ManagementServerUname", "root", 0},
	{ "TR069_ManagementServerPWD", "secret", 0},
	{ "TR069_ParameterKey", "None", 0},
	{ "TR069_conn_req_url", "http://127.0.0.1:9171/test", 0},
	{ "TR069_conn_req_port", "9171", 0},
	{ "TR069_conn_req_uname", "root", 0},
	{ "TR069_conn_req_pwd", "secret", 0},
	{ "TR069_stun_endis", "true", 0},
	//{ "TR069_stun_addr", "acs.ertelecom.ru", 0},
	{ "TR069_stun_addr", "acs.voiceip.pl", 0},
	//{ "TR069_stun_port", "6464", 0},
	{ "TR069_stun_port", "3478", 0},
	{ "TR069_stun_uname", "", 0},
	{ "TR069_stun_pwd", "", 0},
	
	{ "LAN_qid", "1 2", 0},
	{ "wl_wep_qid", "1 2 3 4", 0},
	{ "wl_wpa_qid", "1", 0},
	{ "ppp_description", "", 0},
	{ "portmapping_qid", "", 0},
	{ "portmapping_index", "1", 0},
	{ "static_router_qid", "", 0},
	{ "static_router_index", "1", 0},
	
	{ "TR069_ping_state", "None", 0},
	{ "TR069_ping_host", "168.95.1.1", 0},
	{ "TR069_ping_count", "5", 0},
	{ "TR069_ping_datalen", "56", 0},
	{ "TR069_ping_result", "", 0},
	
	{ 0, 0, 0 }	 /* The End One */
};

