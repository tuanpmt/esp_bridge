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
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	system_init_done_cb(bridge_init);
}
