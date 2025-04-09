/** \file uart_udp_com.c
 *******************************************************************************
 ** \brief  Provides APIs for UART driver
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2023-24 Procubed Innovations Pvt Ltd.
 ** All rights reserved.
 **
 ** THIS SOFTWARE IS PROVIDED BY "AS IS" AND ALL WARRANTIES OF ANY KIND,
 ** INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR USE,
 ** ARE EXPRESSLY DISCLAIMED.  THE DEVELOPER SHALL NOT BE LIABLE FOR ANY
 ** DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. THIS SOFTWARE
 ** MAY NOT BE USED IN PRODUCTS INTENDED FOR USE IN IMPLANTATION OR OTHER
 ** DIRECT LIFE SUPPORT APPLICATIONS WHERE MALFUNCTION MAY RESULT IN THE DIRECT
 ** PHYSICAL HARM OR INJURY TO PERSONS. ALL SUCH IS USE IS EXPRESSLY PROHIBITED.
 **
 *******************************************************************************
 **  \endcond
 */


/*******************************************************************************
* File inclusion
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "socket/socket.h"
#include "sl_wisun_trace_util.h"
#include "sl_wisun_app_core.h"
#include "sl_wisun_app_core_util.h"
#include "uart_hal.h"
#include "uart_udp_com.h"

/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/



/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/


/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/





/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/



/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

int udp_socket;
sl_wisun_app_core_current_addr_t ip;
extern char remote_ip[35];

extern sl_wisun_mac_address_t mac_address; 

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/


/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
void udp_client_read(const int32_t sockid, const uint16_t size);
  /* None */

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/
//Create a sockid
int udp_client_create(void)
{
  int32_t sockid = SOCKET_INVALID_ID; // client socket id

  // create client socket
  sockid = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_IP);

  if (sockid == SOCKET_INVALID_ID) {
    printf("[Failed to create socket: %ld]\n", sockid);
    return -1;
  } else {
    printf("[Socket created: %ld]\n", sockid);
  }
  
   struct sockaddr_in6 local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin6_family = AF_INET6;
    local_addr.sin6_port = htons(UDP_PORT);  // Bind to specific port
  
 local_addr.sin6_addr = in6addr_any;
    // Bind the socket to the address and port
    if (bind(sockid, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        printf("[Failed to bind socket to port: 1234]\n");
        return -1;
    } else {
        printf("[Socket bound to port: 1234]\n");
    }
  
  return sockid;
}

#define TIMEOUT_FLAG (1UL << 0)
extern char remote_ip[35];
//UDP response
void udp_response_task(void *argument) {
osEventFlagsId_t timeout_event;
    uint32_t timeout = 5000U; // Timeout duration in milliseconds
        
    // Create an event flag to manage the timeout
    timeout_event = osEventFlagsNew(NULL);
    if (timeout_event == NULL) {
        printf("Failed to create event flags.\n");
        return;
    }

     SL_WISUN_THREAD_LOOP {

        udp_client_read(udp_socket,256);

  
  // Wait for the timeout duration
        osEventFlagsWait(timeout_event, TIMEOUT_FLAG, osFlagsWaitAny, timeout);
    }
    }



/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
void udp_client_read(const int32_t sockid, const uint16_t size)
{
  //char *c = (char *) app_wisun_malloc((size + 1) * sizeof(char));
  
  static uint8_t c[256] = { 0U };
  int32_t res;
  static sockaddr_in6_t server_addr;
  socklen_t len = sizeof(server_addr);
  // memset(c,0,size);

  res = recvfrom(sockid, c, size, 0,
                 (struct sockaddr *)&server_addr, &len);
  if (res == SOCKET_RETVAL_ERROR || !res) {
   // memset(c,0,size);
    return;
  }
  
  
  if(res>0){
   c[res] = '\0'; 
  
  //printf("%s %d\n", c,res);
  
  uart_hal_write((uint8_t*)c,res);
  }
  
 // app_wisun_free(c);
  
 
}

/* write to tcp client socket */
void udp_client_write(const int32_t sockid, const char *remote_ip_address,
                               const uint16_t remote_port, const char *str, int len)
{
  int32_t res;
  static sockaddr_in6_t server_addr;
  if (remote_ip_address == NULL) {
    return;
  }
 
  if (str == NULL) {
    return;
  }
 
  // setting the server address
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = remote_port;
  if (inet_pton(AF_INET6, remote_ip_address,
                &server_addr.sin6_addr) == SOCKET_RETVAL_ERROR) {
    return;
  }

bool sent = false;

for (int retry = 0; retry < 3; retry++) {
    if (sendto(sockid, str, len, 0,(const struct sockaddr *) &server_addr, sizeof(server_addr)) >=0) {
        sent = true;
        break;
    } else {
        osDelay(10);
    }
}
//  res = sendto(sockid, str, len, 0,
//               (const struct sockaddr *) &server_addr, sizeof(server_addr));
//  if (res == SOCKET_RETVAL_ERROR) {
//    printf("[Failed to send on socket: %ld]\n", sockid);
//  }
  
}

// Function to create a buffer packet
void create_buffer_packet(uint8_t *packet, const uint8_t *data, uint16_t data_length) {
    if (packet == NULL || data == NULL) {
        printf("[Error: Null pointer passed to create_buffer_packet]\n");
        return;
    }

    uint16_t index = 0;

    // Add identifier (1 byte)
    packet[index++] = 0xC1;

    // Add MAC address (8 bytes)
    memcpy(&packet[index], mac_address.address, 8); // Use the global mac_address variable
    index += 8;

    // Add data length (2 bytes, big-endian format)
    packet[index++] = (data_length >> 8) & 0xFF; // High byte
    packet[index++] = data_length & 0xFF;        // Low byte

    // Add data (x bytes)
    memcpy(&packet[index], data, data_length);
    index += data_length;

    // Add end frame (1 byte)
    packet[index++] = 0xFF;
}


/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/


/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
