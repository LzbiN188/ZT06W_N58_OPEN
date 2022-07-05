#ifndef __NWY_TEST_CLI_FUNC_DEF_H__
#define __NWY_TEST_CLI_FUNC_DEF_H__

/**************************SIM*********************************/
void nwy_test_cli_get_sim_status();
void nwy_test_cli_verify_pin();
void nwy_test_cli_get_pin_mode();
void nwy_test_cli_set_pin_mode();
void nwy_test_cli_change_pin();
void nwy_test_cli_verify_puk();
void nwy_test_cli_get_imsi();
void nwy_test_cli_get_iccid();
void nwy_test_cli_get_msisdn();
void nwy_test_cli_set_msisdn();
void nwy_test_cli_get_sim_slot();
void nwy_test_cli_set_sim_slot();

/**************************DATA*********************************/
void nwy_test_cli_data_create();
void nwy_test_cli_get_profile();
void nwy_test_cli_set_profile();
void nwy_test_cli_data_start();
void nwy_test_cli_data_info();
void nwy_test_cli_data_stop();
void nwy_test_cli_data_release();

/**************************NW*********************************/
void nwy_test_cli_nw_get_mode();
void nwy_test_cli_nw_set_mode();
void nwy_test_cli_nw_get_fplmn();
void nwy_test_cli_nw_manual_scan();
void nwy_test_cli_nw_band_lock();
void nwy_test_cli_nw_freq_lock();
void nwy_test_cli_nw_get_ims_st();
void nwy_test_cli_nw_set_ims_st();
void nwy_test_cli_nw_get_def_pdn();
void nwy_test_cli_nw_set_def_pdn();
void nwy_test_cli_nw_get_radio_st();
void nwy_test_cli_nw_set_radio_st();
void nwy_test_cli_nw_get_radio_sign();
void nwy_test_cli_nw_cs_st();
void nwy_test_cli_nw_ps_st();
void nwy_test_cli_nw_lte_st();
void nwy_test_cli_nw_operator_info();
void nwy_test_cli_nw_get_ehplmn();
void nwy_test_cli_nw_get_signal_rssi();
void nwy_test_cli_nw_get_netmsg();

/**************************VOICE*********************************/
void nwy_test_cli_voice_call_start();
void nwy_test_cli_voice_call_end();
void nwy_test_cli_voice_auto_answ();
void nwy_test_cli_voice_volte_set();
void nwy_test_cli_voice_caller_id();
void nwy_test_cli_voice_call_hold();
void nwy_test_cli_voice_call_unhold();

/**************************SMS*********************************/
void nwy_test_cli_sms_send();
void nwy_test_cli_sms_del();
void nwy_test_cli_sms_get_sca();
void nwy_test_cli_sms_set_sca();
void nwy_test_cli_sms_set_storage();
void nwy_test_cli_sms_get_storage();
void nwy_test_cli_sms_set_report_md();
void nwy_test_cli_sms_read();
void nwy_test_cli_sms_pdu_send();
void nwy_test_cli_ussd_send();

/**************************UART*********************************/
void nwy_test_cli_uart_init();
void nwy_test_cli_uart_set_baud();
void nwy_test_cli_uart_get_baud();
void nwy_test_cli_uart_set_para();
void nwy_test_cli_uart_get_para();
void nwy_test_cli_uart_set_tout();
void nwy_test_cli_uart_send();
void nwy_test_cli_uart_reg_rx_cb();
void nwy_test_cli_uart_reg_tx_cb();
void nwy_test_cli_uart_deinit();

/**************************I2C*********************************/
void nwy_test_cli_i2c_init();
void nwy_test_cli_i2c_read();
void nwy_test_cli_i2c_write();
void nwy_test_cli_i2c_put_raw();
void nwy_test_cli_i2c_get_raw();
void nwy_test_cli_i2c_deinit();

/**************************SPI*********************************/
void nwy_test_cli_spi_init();
void nwy_test_cli_spi_trans();
void nwy_test_cli_spi_deinit();

/**************************GPIO*********************************/
void nwy_test_cli_gpio_set_val();
void nwy_test_cli_gpio_get_val();
void nwy_test_cli_gpio_set_dirt();
void nwy_test_cli_gpio_get_dirt();
void nwy_test_cli_gpio_config_irq();
void nwy_test_cli_gpio_enable_irq();
void nwy_test_cli_gpio_disable_irq();
void nwy_test_cli_gpio_close();

/**************************ADC*********************************/
void nwy_test_cli_adc_read();

/**************************PM*********************************/
void nwy_test_cli_pm_save_md();
void nwy_test_cli_pm_get_pwr_st();
void nwy_test_cli_pm_pwr_off();
void nwy_test_cli_pm_set_dtr();
void nwy_test_cli_pm_pwr_key();
void nwy_test_cli_pm_switch_sub_pwr();
void nwy_test_cli_pm_set_sub_pwr();
void nwy_test_cli_pm_set_auto_off();
void nwy_test_cli_pm_reg_charger_cb();

/**************************KEYPAD*********************************/
void nwy_test_cli_keypad_reg_cb();
void nwy_test_cli_keypad_set_debouce();

/**************************PWM*********************************/
void nwy_test_cli_pwm_init();
void nwy_test_cli_pwm_start();
void nwy_test_cli_pwm_stop();
void nwy_test_cli_pwm_deinit();

/**************************LCD*********************************/
void nwy_test_cli_lcd_open();
void nwy_test_cli_lcd_draw_line();
void nwy_test_cli_lcd_draw_chinese();
void nwy_test_cli_lcd_close();
void nwy_test_cli_lcd_open_bl();
void nwy_test_cli_lcd_close_bl();
void nwy_test_cli_lcd_set_bl_level();

/**************************SD*********************************/
void nwy_test_cli_sd_get_st();
void nwy_test_cli_sd_mnt();
void nwy_test_cli_sd_unmnt();

/**************************FLASH*********************************/
void nwy_test_cli_flash_open();
void nwy_test_cli_flash_erase();
void nwy_test_cli_flash_write();
void nwy_test_cli_flash_read();

/**************************TTS*********************************/
void nwy_test_cli_tts_input();
void nwy_test_cli_tts_play_start();
void nwy_test_cli_tts_play_stop();

/**************************FOTA*********************************/
void nwy_test_cli_fota_base_ver();
void nwy_test_cli_fota_app_ver();

/**************************AUDIO*********************************/
void nwy_test_cli_audio_play();
void nwy_test_cli_audio_rec();
void nwy_test_cli_audio_dtmf();

/**************************FS*********************************/
void nwy_test_cli_fs_open();
void nwy_test_cli_fs_write();
void nwy_test_cli_fs_read();
void nwy_test_cli_fs_fsize();
void nwy_test_cli_fs_seek();
void nwy_test_cli_fs_sync();
void nwy_test_cli_fs_fstate();
void nwy_test_cli_fs_trunc();
void nwy_test_cli_fs_close();
void nwy_test_cli_fs_remove();
void nwy_test_cli_fs_rename();
void nwy_test_cli_dir_open();
void nwy_test_cli_dir_read();
void nwy_test_cli_dir_tell();
void nwy_test_cli_dir_seek();
void nwy_test_cli_dir_rewind();
void nwy_test_cli_dir_close();
void nwy_test_cli_dir_mk();
void nwy_test_cli_dir_remove();
void nwy_test_cli_fs_free_size();
void nwy_test_cli_safe_fs_init();
void nwy_test_cli_safe_fs_read();
void nwy_test_cli_safe_fs_write();
void nwy_test_cli_safe_fs_fszie();

/**************************BLE*********************************/
void nwy_test_cli_ble_open();
void nwy_test_cli_ble_set_adv();
void nwy_test_cli_ble_send();
void nwy_test_cli_ble_recv();
void nwy_test_cli_ble_updata_connt();
void nwy_test_cli_ble_get_status();
void nwy_test_cli_ble_get_ver();
void nwy_test_cli_ble_set_dev_name();
void nwy_test_cli_ble_close();
void nwy_test_cli_ble_set_beacon();
void nwy_test_cli_ble_set_manufacture();
void nwy_test_cli_ble_set_adv_data();
void nwy_test_cli_ble_set_scanrsp_data();
void nwy_test_cli_ble_set_srv();
void nwy_test_cli_ble_set_char();
void nwy_test_cli_ble_conn_status_report();
void nwy_test_cli_ble_conn_status();
void nwy_test_cli_ble_mac_addr();
void nwy_test_cli_ble_add_server();
void nwy_test_cli_ble_add_char();
void nwy_test_cli_ble_add_send_data();
void nwy_test_cli_ble_add_recv_data();
void nwy_test_cli_ble_disconnect();
void nwy_test_cli_ble_read_req();
void nwy_test_cli_ble_set_adv_server_uuid();
void nwy_test_cli_ble_read_rsp();
void nwy_test_cli_ble_add_white_list();
void nwy_test_cli_ble_remove_white_list();
void nwy_test_cli_ble_clean_white_list();
void nwy_test_cli_ble_set_adv_param();

/**************************BLE Client*********************************/
void nwy_test_cli_ble_client_set_enable();
void nwy_test_cli_ble_client_scan();
void nwy_test_cli_ble_client_connect();
void nwy_test_cli_ble_client_disconnect();
void nwy_test_cli_ble_client_discover_srv();
void nwy_test_cli_ble_client_discover_char();
void nwy_test_cli_ble_client_send_data();
void nwy_test_cli_ble_client_recv_data();

/**************************WIFI*********************************/
void nwy_cli_test_wifi_get_st();
void nwy_cli_test_wifi_enable();
void nwy_cli_test_wifi_set_work_md();
void nwy_cli_test_wifi_set_ap_para();
void nwy_cli_test_wifi_set_ap_para_adv();
void nwy_cli_test_wifi_get_clit_info();
void nwy_cli_test_wifi_sta_scan();
void nwy_cli_test_wifi_sta_scan_ret();
void nwy_cli_test_wifi_sta_connt();
void nwy_cli_test_wifi_sta_disconnt();
void nwy_cli_test_wifi_sta_get_hostpot_info();
void nwy_cli_test_wifi_disable();

/**************************GNSS*********************************/
void nwy_test_cli_gnss_open();	
void nwy_test_cli_gnss_set_position_md();
void nwy_test_cli_gnss_set_updata_rate();
void nwy_test_cli_gnss_set_output_fmt();
void nwy_test_cli_gnss_set_startup_fmt();
void nwy_test_cli_gnss_get_nmea();
void nwy_test_cli_gnss_dialing();
void nwy_test_cli_gnss_set_server();
void nwy_test_cli_gnss_open_base();
void nwy_test_cli_wifi_open_base();
void nwy_test_cli_gnss_open_assisted();
void nwy_test_cli_gnss_close();

/**************************TCP*********************************/
void nwy_test_cli_tcp_setup();
void nwy_test_cli_tcp_send();
void nwy_test_cli_tcp_close();

/**************************UDP*********************************/
void nwy_test_cli_udp_setup();
void nwy_test_cli_udp_send();
void nwy_test_cli_udp_close();

/**************************FTP*********************************/
void nwy_test_cli_ftp_login();
void nwy_test_cli_ftp_get();
void nwy_test_cli_ftp_put();
void nwy_test_cli_ftp_fsize();
void nwy_test_cli_ftp_list();
void nwy_test_cli_ftp_delet();
void nwy_test_cli_ftp_logout();

/**************************HTTP*********************************/
void nwy_test_cli_http_setup();
void nwy_test_cli_http_get();
void nwy_test_cli_http_head();
void nwy_test_cli_http_post();
void nwy_test_cli_http_close();
void nwy_test_cli_https_add_cert();
void nwy_test_cli_https_check_cert();
void nwy_test_cli_https_delet_cert();
void nwy_test_cli_http_compute_md5();

/**************************ALI MQTT*********************************/
void nwy_test_cli_alimqtt_connect();
void nwy_test_cli_alimqtt_pub();
void nwy_test_cli_alimqtt_sub();
void nwy_test_cli_alimqtt_unsub();
void nwy_test_cli_alimqtt_state();
void nwy_test_cli_alimqtt_disconnect();

/**************************MQTT*********************************/
void nwy_test_cli_mqtt_connect();
void nwy_test_cli_mqtt_pub();
void nwy_test_cli_mqtt_sub();
void nwy_test_cli_mqtt_unsub();
void nwy_test_cli_mqtt_state();
void nwy_test_cli_mqtt_disconnect();
#ifdef FEATURE_NWY_N58_OPEN_NIPPON
void nwy_test_cli_mqtt_pub_test();
#endif
/**************************OS & Dev API************************/
void nwy_test_cli_get_model();
void nwy_test_cli_get_imei();
void nwy_test_cli_get_chipid();
void nwy_test_cli_get_boot_cause();
void nwy_test_cli_get_sw_ver();
void nwy_test_cli_get_hw_ver();
void nwy_test_cli_get_heap_info();
void nwy_test_cli_get_cpu_temp();

void nwy_test_cli_start_timer();
void nwy_test_cli_stop_timer();
void nwy_test_cli_get_time();
void nwy_test_cli_set_time();
void nwy_test_cli_set_semp();

void nwy_test_cli_send_virt_at();
void nwy_test_cli_reg_at_fwd();
#endif
