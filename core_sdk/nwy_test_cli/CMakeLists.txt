cmake_minimum_required(VERSION 3.13)

include_directories(comm modules)

add_appimg(${target} ${flash_ldscript}  comm/nwy_test_cli_adpt.c 
                                        comm/nwy_test_cli_menu.c
										comm/nwy_test_cli_utils.c
										modules/nwy_test_cli_tcpip.c
										modules/nwy_test_cli_mobile.c
										modules/nwy_test_cli_iot.c
										modules/nwy_test_cli_sdw.c
										modules/nwy_test_cli_driver.c
										modules/nwy_test_cli_lbs.c
										modules/nwy_test_cli_os.c)