/** \file uart_udp_com.h
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


#ifndef _UART_UDP_COM_
#define _UART_UDP_COM_

#ifdef __cplusplus
extern "C" {
#endif
  
/**
 *****************************************************************************
 * @ingroup sysdoc
 *
 * @{
 *****************************************************************************/

/**
 *****************************************************************************
 * 
 * @}     
 *****************************************************************************/

/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/

/**
 ** \defgroup uart_udp_com UART UDP commuinication
 */

/*@{*/

/**
 ** \defgroup uart_udp_com_def  UART UDP commuinication
 ** \ingroup uart_udp_com
 */

/*@{*/

	/* None */

/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
*/

#define UDP_PORT 1234

/*@}*/

/*
** =============================================================================
** Public Variable Declarations
** =============================================================================
*/


/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

/**
 ** \defgroup uart_udp_com_data_func UART UDP APIs
 ** \ingroup uart_udp_com
 */

/*@{*/

int udp_client_create(void);
void udp_response_task(void *argument);
void udp_client_write(const int32_t sockid, const char *remote_ip_address,const uint16_t remote_port, const char *str,int len);
void uart_timeout_handler(void);
void create_buffer_packet(uint8_t *packet, const uint8_t *data, uint16_t data_length);


/*@}*/

/*@}*/



 /*
 ** ============================================================================
 ** Public Macro definitions
 ** ============================================================================
 */

 
 /*
 ** ============================================================================
 ** Public Structures, Unions & enums Type Definitions
 ** ============================================================================
 */



 /*
 ** ============================================================================
 ** Public Variable Declarations
 ** ============================================================================
 */

 /* None */

 /*
 ** ============================================================================
 ** Private Variable Definitions
 ** ============================================================================
 */

 /* None */

 /*
 ** =============================================================================
 ** Public Variables Definitions
 ** =============================================================================
 **/

 /* None */

 /*
 ** ============================================================================
 ** External Variable Declarations
 ** ============================================================================
 */

 /* None */

 /*
 **=========================================================================
 **  Public Function Prototypes
 **=========================================================================
 */


  
  
  
#ifdef __cplusplus
}
#endif
#endif /* _ADI_UART_UDP_COM_ */
