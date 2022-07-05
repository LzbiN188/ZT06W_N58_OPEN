#ifndef __NWY_TEST_CLI_ADPT_H__
#define __NWY_TEST_CLI_ADPT_H__

#include "nwy_osi_api.h"
#include "nwy_usb_serial.h"

void nwy_test_cli_dbg(const char *func, int line, char* fmt, ... );
#define NWY_CLI_LOG(...) nwy_test_cli_dbg(__func__, __LINE__, __VA_ARGS__)

int nwy_test_cli_wait_select();
void nwy_test_cli_select_enter();

#endif