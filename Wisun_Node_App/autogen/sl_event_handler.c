#include "sl_event_handler.h"

#include "em_chip.h"
#include "sl_device_init_nvic.h"
#include "sl_device_init_dcdc.h"
#include "sl_device_init_hfxo.h"
#include "sl_device_init_clocks.h"
#include "sl_device_init_emu.h"
#include "pa_conversions_efr32.h"
#include "sl_rail_util_pti.h"
#include "sl_sleeptimer.h"
#include "sl_iostream_stdlib_config.h"
#include "sl_mbedtls.h"
#include "nvm3_default.h"
#include "sl_wisun_app_core.h"
#include "sl_wisun_app_setting.h"
#include "sl_wisun_cli_core.h"
#include "sl_wisun_event_mgr.h"
#include "sl_wisun_stack.h"
#include "sl_cli_instances.h"
#include "psa/crypto.h"
#include "cpu.h"
#include "cmsis_os2.h"
#include "sl_iostream_init_instances.h"
#include "sl_wisun_cli_settings.h"

void sl_platform_init(void)
{
  CHIP_Init();
  sl_device_init_nvic();
  sl_device_init_dcdc();
  sl_device_init_hfxo();
  sl_device_init_clocks();
  sl_device_init_emu();
  nvm3_initDefault();
  CPU_Init();
  osKernelInitialize();
}

void sl_kernel_start(void)
{
  osKernelStart();
}

void sl_driver_init(void)
{
}

void sl_service_init(void)
{
  sl_sleeptimer_init();
 // sl_iostream_stdlib_disable_buffering();
  sl_mbedtls_init();
  psa_crypto_init();
  sl_iostream_init_instances();
  //sl_cli_instances_init();
}

void sl_stack_init(void)
{
  sl_rail_util_pa_init();
  sl_rail_util_pti_init();
  sl_wisun_stack_init();
}

void sl_internal_app_init(void)
{
  sl_wisun_app_core_init();
  app_wisun_setting_init();
  //app_wisun_cli_init();
  app_settings_init();
  app_wisun_event_mgr_init();
  //sl_wisun_ping_init();
}

void sl_iostream_init_instances(void)
{
}

