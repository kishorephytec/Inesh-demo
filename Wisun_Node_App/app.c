/***************************************************************************//**
 * @file
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "app.h"
#include "app_init.h"
#include "uart_hal.h"
#include "uart_udp_com.h"
#include "sl_wisun_trace_util.h"
#include "rail.h"
#include "rail_types.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// Define the RAIL handle
static RAIL_Handle_t railHandle;
//#define RAIL_DECLARE_STATE_BUFFER(name) RAIL_StateBuffer_t name

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
extern int udp_socket;
extern sl_wisun_app_core_current_addr_t ip;
volatile bool uart_data_ready = false;
sl_wisun_mac_address_t mac_address; 
      uint16_t data_length = 0;
      uint8_t data[RING_BUFFER_SIZE];
      int initialisation_done=0;
   RAIL_StateBuffer_t stateBuffer;
      
//extern uint8_t uart_debug_buff[2000];
//extern uint16_t uart_debug_buff_Cnt;
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
char remote_ip[35];
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*App task function*/
void uart_timeout_handler(void)
{
//    uart_data_ready = true;
//            if (uart_data_ready) {
//      // Reset the flag
//      uart_data_ready = false;

      // Example: Process the ring buffer

      while (r_buff.tail != r_buff.head) {
          data[data_length++] = r_buff.buffer[r_buff.tail];
          r_buff.tail = (r_buff.tail + 1) % RING_BUFFER_SIZE;
      }
 
      // Send the data over UDP
      //udp_client_write(udp_socket,remote_ip,UDP_PORT,(char *)data,data_length);
 
      // Reset the ring buffer
//      r_buff.head = 0;
//      r_buff.tail = 0;
      uart_data_ready=true;
//  }

}
void app_task(void *args)
{
  (void) args;

  // connect to the wisun network
  sl_wisun_app_core_util_connect_and_wait();
  
  UART_init();
  sl_wisun_app_core_get_current_addresses(&ip);
    if (inet_ntop(AF_INET6, &ip.border_router, remote_ip, sizeof(remote_ip)) == NULL) {
    perror("inet_ntop");
    return;
  }
  
  if (sl_wisun_get_mac_address(&mac_address) != SL_STATUS_OK) {
      printf("Error: Failed to get MAC address\n");
  }
  
  udp_socket = udp_client_create();
  
  static sockaddr_in6_t server_addr;
  // setting the server address
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = UDP_PORT;
  if (inet_pton(AF_INET6, remote_ip,
                &server_addr.sin6_addr) == SOCKET_RETVAL_ERROR) {
    printf("[Invalid IP address: %s]\n", remote_ip);
    return;
  }
  udp_init(); 
 
RAIL_Config_t railConfig = {
    .eventsCallback = NULL,
};
  railHandle = RAIL_Init(&railConfig, NULL);
  if (railHandle == NULL) {
    printf("Failed to initialize RAIL\n");
  }

// Initialize the RAIL timer for UART//
  UART_init_rail_timer(railHandle);

// Register the UART timeout callback
  register_uart_timeout_cb(uart_timeout_handler);

  char *buff;
  char buf[39]={0x00, 0x01, 0x00, 0x10, 0x00, 0x01, 0x00, 0x1F, 0x60, 0x1D, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0xBE, 0x10, 0x04, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x06, 0x5F, 0x1F, 0x04, 0x00, 0x62, 0x1E, 0x5D, 0xFF, 0xFF};
        uart_hal_write((uint8_t *)buf,39);

  while (1) {
    // User code here
    sl_wisun_app_core_util_dispatch_thread();
    
    if(uart_data_ready){
      if (!initialisation_done) {
        // Buffer to compare
        char buf[51] = {0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x00, 0x2B, 0x61, 0x29, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0xA2, 0x03, 0x02, 0x01, 0x00, 0xA3, 0x05, 0xA1, 0x03, 0x02, 0x01, 0x00, 0xBE, 0x10, 0x04, 0x0E, 0x08, 0x00, 0x06, 0x5F, 0x1F, 0x04, 0x00, 0x00, 0x00, 0x10, 0x03, 0x40, 0x00, 0x07};

        // Compare the `data` buffer with the predefined buffer
        if (memcmp(data, buf, 43) == 0) {
            // Data to write if the comparison matches
            char data_read[21] = {0x00, 0x01, 0x00, 0x10, 0x00, 0x01, 0x00, 0x0D, 0xC0, 0x01, 0xC1, 0x00, 0x01, 0x00, 0x00, 0x60, 0x01, 0x00, 0xFF, 0x02, 0x00};
            uart_hal_write((uint8_t *)data_read, 21);
        }

        // Check specific conditions in the `data` buffer
        if ((data[10] == 0xC1) || (data[11] == 0xC1)) {
            // Shift data forward by one byte
            for (int i = data_length - 1; i >= 0; i--) {
                data[i + 1] = data[i];
            }

            // Add a new byte at the start of the buffer
            data[0] = 0x4D;
            data_length++;
            initialisation_done = 1;
        }
    }
      uint8_t *packet = (uint8_t *)malloc(data_length + 12); // 12 bytes for header
          if (packet == NULL) {
              printf("[Error: Memory allocation failed for packet]\n");
              return;
          }
            create_buffer_packet(packet, data, data_length);
      udp_client_write(udp_socket,remote_ip,UDP_PORT,(char *)packet,data_length+12);
      r_buff.head = 0;
      r_buff.tail = 0;
      
      memset(data,0,RING_BUFFER_SIZE);
      data_length=0;
      
      uart_data_ready=false;
      app_wisun_free(packet);
    }
    



  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
