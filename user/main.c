#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "cmd.h"
#include "driver/uart.h"

void ICACHE_FLASH_ATTR
bridge_init(void)
{
	CMD_Init();
}

void ICACHE_FLASH_ATTR
user_init(void)
{
	uart_init(BIT_RATE_19200, BIT_RATE_19200);
	wifi_station_set_auto_connect(FALSE);
	system_init_done_cb(bridge_init);
}
