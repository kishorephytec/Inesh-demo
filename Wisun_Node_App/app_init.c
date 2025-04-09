/***************************************************************************//**
 * @file
 * @brief Application init
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <assert.h>
#include "app_init.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_wisun_event_mgr.h"
#include "sl_wisun_app_core_util.h"
#include "sl_wisun_app_core_config.h"
#include "app.h"
#include "app_custom_callback.h"
#include "uart_udp_com.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void app_init(void)
{
  /* Init project info */
  sl_wisun_app_core_util_project_info_init("Wi-SUN UDP Client Application");

  /* Creating App main thread */
  const osThreadAttr_t app_task_attr = {
    .name        = "AppMain",
    .attr_bits   = osThreadDetached,
    .cb_mem      = NULL,
    .cb_size     = 0,
    .stack_mem   = NULL,
    .stack_size  = app_stack_size_word_to_byte(SL_WISUN_APP_CORE_MAIN_STACK_SIZE_WORD),
    .priority    = osPriorityNormal,
    .tz_module   = 0
  };
  osThreadId_t app_thr_id = osThreadNew(app_task,
                                        NULL,
                                        &app_task_attr);
  assert(app_thr_id != NULL);
}

void udp_init(void){
    //Thread for UDP response  
  const osThreadAttr_t udp_task_attr = {
    .name        = "UDPResponse",
    .attr_bits   = osThreadDetached,
    .cb_mem      = NULL,
    .cb_size     = 0,
    .stack_mem   = NULL,
    .stack_size  = app_stack_size_word_to_byte(1024UL), // Adjust stack size as needed
    .priority    = osPriorityNormal1,
    .tz_module   = 0
};
  
  // Create the thread
  osThreadId_t udp_thr_id = osThreadNew(udp_response_task, NULL, &udp_task_attr);
  

  assert(udp_thr_id != NULL);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
