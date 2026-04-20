/*
 * FreeRTOS+TCP V4.3.3
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file IPTraceMacroDefaults.h
 * @brief This file provides default (empty) implementations for any IP trace
 *        macros that are not defined by the user.  See
 *        https://freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/08-Trace-macros
 */

#ifndef IP_TRACE_MACRO_DEFAULTS_H
#define IP_TRACE_MACRO_DEFAULTS_H

#ifndef FREERTOS_IP_CONFIG_DEFAULTS_H
    #error FreeRTOSIPConfigDefaults.h has not been included yet
#endif

/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                           NETWORK TRACE MACROS                            */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER
 *
 * Called when a task attempts to obtain a network buffer, but a buffer was
 * not available even after any defined block period.
 */
#ifndef iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER
    #define iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER_FROM_ISR
 *
 * Called when an interrupt service routine attempts to obtain a network
 * buffer, but a buffer was not available.
 */
#ifndef iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER_FROM_ISR
    #define iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER_FROM_ISR()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_BUFFER_OBTAINED
 *
 * Called when the network buffer at address pxBufferAddress is obtained from
 * the TCP/IP stack by an RTOS task.
 */
#ifndef iptraceNETWORK_BUFFER_OBTAINED
    #define iptraceNETWORK_BUFFER_OBTAINED( pxBufferAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_BUFFER_OBTAINED_FROM_ISR
 *
 * Called when the network buffer at address pxBufferAddress is obtained from
 * the TCP/IP stack by an interrupt service routine.
 */
#ifndef iptraceNETWORK_BUFFER_OBTAINED_FROM_ISR
    #define iptraceNETWORK_BUFFER_OBTAINED_FROM_ISR( pxBufferAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_BUFFER_RELEASED
 *
 * Called when the network buffer at address pxBufferAddress is released back
 * to the TCP/IP stack.
 */
#ifndef iptraceNETWORK_BUFFER_RELEASED
    #define iptraceNETWORK_BUFFER_RELEASED( pxBufferAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_DOWN
 *
 * Called when the network driver indicates that the network connection has
 * been lost (not implemented by all network drivers).
 */
#ifndef iptraceNETWORK_DOWN
    #define iptraceNETWORK_DOWN()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_EVENT_RECEIVED
 *
 * Called when the TCP/IP stack processes an event previously posted to the
 * network event queue. eEvent will be one of the following values:
 *
 * eNetworkDownEvent - The network interface has been lost and/or needs
 *                     [re]connecting.
 * eNetworkRxEvent - The network interface has queued a received Ethernet
 *                   frame.
 * eARPTimerEvent - The Resolution timer expired.
 * eStackTxEvent - The software stack has queued a packet to transmit.
 * eDHCPEvent - Process the DHCP state machine.
 *
 * Note the events are defined by the private eIPEvent_t type which is not
 * generally accessible.
 */
#ifndef iptraceNETWORK_EVENT_RECEIVED
    #define iptraceNETWORK_EVENT_RECEIVED( eEvent )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_INTERFACE_INPUT
 *
 * Called when a packet of length uxDataLength and with the contents at
 * address pucEthernetBuffer has been received.
 */
#ifndef iptraceNETWORK_INTERFACE_INPUT
    #define iptraceNETWORK_INTERFACE_INPUT( uxDataLength, pucEthernetBuffer )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_INTERFACE_OUTPUT
 *
 * Called when a packet of length uxDataLength and with the contents at
 * address pucEthernetBuffer has been sent.
 */
#ifndef iptraceNETWORK_INTERFACE_OUTPUT
    #define iptraceNETWORK_INTERFACE_OUTPUT( uxDataLength, pucEthernetBuffer )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_INTERFACE_RECEIVE
 *
 * Called when a packet is received from the network by the network driver.
 * Note this macro is called by the network driver rather than the TCP/IP stack
 * and may not be called at all by drivers provided by third parties.
 */
#ifndef iptraceNETWORK_INTERFACE_RECEIVE
    #define iptraceNETWORK_INTERFACE_RECEIVE()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_INTERFACE_TRANSMIT
 *
 * Called when a packet is sent to the network by the network driver. Note this
 * macro is called by the network driver rather than the TCP/IP stack and may
 * not be called at all by drivers provided by third parties.
 */
#ifndef iptraceNETWORK_INTERFACE_TRANSMIT
    #define iptraceNETWORK_INTERFACE_TRANSMIT()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceSTACK_TX_EVENT_LOST
 *
 * Called when a packet generated by the TCP/IP stack is dropped because there
 * is insufficient space in the network event queue (see the
 * ipconfigEVENT_QUEUE_LENGTH setting in FreeRTOSIPConfig.h).
 */
#ifndef iptraceSTACK_TX_EVENT_LOST
    #define iptraceSTACK_TX_EVENT_LOST( xEvent )
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                           NETWORK TRACE MACROS                            */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                            DRIVER TRACE MACROS                            */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceETHERNET_RX_EVENT_LOST
 *
 * Called when a packet received by the network driver is dropped for one of
 * the following reasons: There is insufficient space in the network event
 * queue (see the ipconfigEVENT_QUEUE_LENGTH setting in FreeRTOSIPConfig.h),
 * the received packet has an invalid data length, or there are no network
 * buffers available (see the ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS setting
 * in FreeRTOSIPConfig.h). Note this macro is called by the network driver
 * rather than the TCP/IP stack and may not be called at all by drivers
 * provided by third parties.
 */
#ifndef iptraceETHERNET_RX_EVENT_LOST
    #define iptraceETHERNET_RX_EVENT_LOST()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceWAITING_FOR_TX_DMA_DESCRIPTOR
 *
 * Called when a transmission at the network driver level cannot complete
 * immediately because the driver is having to wait for a DMA descriptor to
 * become free. Try increasing the configNUM_TX_ETHERNET_DMA_DESCRIPTORS
 * setting in FreeRTOSConfig.h (if it exists for the network driver being
 * used).
 */
#ifndef iptraceWAITING_FOR_TX_DMA_DESCRIPTOR
    #define iptraceWAITING_FOR_TX_DMA_DESCRIPTOR()
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            DRIVER TRACE MACROS                            */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             UDP TRACE MACROS                              */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDING_UDP_PACKET
 *
 * Called when a UDP packet is sent to the IP address ulIPAddress. ulIPAddress
 * is expressed as a 32-bit number in network byte order.
 */
#ifndef iptraceSENDING_UDP_PACKET
    #define iptraceSENDING_UDP_PACKET( ulIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                             UDP TRACE MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                           SOCKET TRACE MACROS                             */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceBIND_FAILED
 *
 * A call to FreeRTOS_bind() failed. usPort is the port number the socket
 * xSocket was to be bound to.
 */
#ifndef iptraceBIND_FAILED
    #define iptraceBIND_FAILED( xSocket, usPort )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_CREATE_EVENT_GROUP
 *
 * Called when an event group could not be created, possibly due to
 * insufficient heap space, during new socket creation.
 */
#ifndef iptraceFAILED_TO_CREATE_EVENT_GROUP
    #define iptraceFAILED_TO_CREATE_EVENT_GROUP()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_CREATE_SOCKET
 *
 * A call to FreeRTOS_socket() failed because there was insufficient FreeRTOS
 * heap memory available for the socket structure to be created.
 */
#ifndef iptraceFAILED_TO_CREATE_SOCKET
    #define iptraceFAILED_TO_CREATE_SOCKET()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_NOTIFY_SELECT_GROUP
 */
#ifndef iptraceFAILED_TO_NOTIFY_SELECT_GROUP
    #define iptraceFAILED_TO_NOTIFY_SELECT_GROUP( xSocket )
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceNO_BUFFER_FOR_SENDTO
 *
 * Called when a call to FreeRTOS_sendto() tries to allocate a buffer, but a
 * buffer was not available even after any defined block period.
 */
#ifndef iptraceNO_BUFFER_FOR_SENDTO
    #define iptraceNO_BUFFER_FOR_SENDTO()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceRECVFROM_DISCARDING_BYTES
 *
 * FreeRTOS_recvfrom() is discarding xNumberOfBytesDiscarded bytes because the
 * number of bytes received is greater than the number of bytes that will fit
 * in the user supplied buffer (the buffer passed in as a FreeRTOS_recvfrom()
 * function parameter).
 */
#ifndef iptraceRECVFROM_DISCARDING_BYTES
    #define iptraceRECVFROM_DISCARDING_BYTES( xNumberOfBytesDiscarded )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceRECVFROM_INTERRUPTED
 *
 * Called when a blocking call to FreeRTOS_recvfrom() is interrupted through a
 * call to FreeRTOS_SignalSocket().
 */
#ifndef iptraceRECVFROM_INTERRUPTED
    #define iptraceRECVFROM_INTERRUPTED()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceRECVFROM_TIMEOUT
 *
 * Called when FreeRTOS_recvfrom() gets no data on the given socket even after
 * any defined block period.
 */
#ifndef iptraceRECVFROM_TIMEOUT
    #define iptraceRECVFROM_TIMEOUT()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDTO_DATA_TOO_LONG
 *
 * Called when the data requested to be sent using a call to FreeRTOS_sendto()
 * is too long and could not be sent.
 */
#ifndef iptraceSENDTO_DATA_TOO_LONG
    #define iptraceSENDTO_DATA_TOO_LONG()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDTO_SOCKET_NOT_BOUND
 *
 * Called when the socket used in the call to FreeRTOS_sendto() is not already
 * bound to a port.
 */
#ifndef iptraceSENDTO_SOCKET_NOT_BOUND
    #define iptraceSENDTO_SOCKET_NOT_BOUND()
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                           SOCKET TRACE MACROS                             */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             ARP TRACE MACROS                              */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceARP_PACKET_RECEIVED
 *
 * Called when an ARP packet is received, even if the local network node is not
 * involved in the ARP transaction.
 */
#ifndef iptraceARP_PACKET_RECEIVED
    #define iptraceARP_PACKET_RECEIVED()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceARP_TABLE_ENTRY_CREATED
 *
 * Called when a new entry in the ARP table is created to map the IP address
 * ulIPAddress to the MAC address ucMACAddress. ulIPAddress is expressed as a
 * 32-bit number in network byte order. ucMACAddress is a pointer to an
 * MACAddress_t structure.
 */
#ifndef iptraceARP_TABLE_ENTRY_CREATED
    #define iptraceARP_TABLE_ENTRY_CREATED( ulIPAddress, ucMACAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceARP_TABLE_ENTRY_EXPIRED
 *
 * Called when the entry for the IP address ulIPAddress in the ARP cache is
 * removed. ulIPAddress is expressed as a 32-bit number in network byte order.
 */
#ifndef iptraceARP_TABLE_ENTRY_EXPIRED
    #define iptraceARP_TABLE_ENTRY_EXPIRED( ulIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceARP_TABLE_ENTRY_WILL_EXPIRE
 *
 * Called when an ARP request is about to be sent because the entry for the IP
 * address ulIPAddress in the ARP cache has become stale. ulIPAddress is
 * expressed as a 32-bit number in network byte order.
 */
#ifndef iptraceARP_TABLE_ENTRY_WILL_EXPIRE
    #define iptraceARP_TABLE_ENTRY_WILL_EXPIRE( ulIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceCREATING_ARP_REQUEST
 *
 * Called when the IP generates an ARP request packet.
 */
#ifndef iptraceCREATING_ARP_REQUEST
    #define iptraceCREATING_ARP_REQUEST( ulIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ARP_BUFFER_FULL
 *
 * A packet has come in from an unknown IPv4 address. An ARP request has been
 * sent, but the queue is still filled with a different packet.
 */
#ifndef iptraceDELAYED_ARP_BUFFER_FULL
    #define iptraceDELAYED_ARP_BUFFER_FULL()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptrace_DELAYED_ARP_REQUEST_REPLIED
 *
 * An ARP request has been sent, and a matching reply is received. Now the
 * original packet will be processed by the IP-task.
 */
#ifndef iptrace_DELAYED_ARP_REQUEST_REPLIED
    #define iptrace_DELAYED_ARP_REQUEST_REPLIED()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ARP_REQUEST_STARTED
 *
 * A packet came in from an unknown IPv4 address. An ARP request has been sent
 * and the network buffer is stored for processing later.
 */
#ifndef iptraceDELAYED_ARP_REQUEST_STARTED
    #define iptraceDELAYED_ARP_REQUEST_STARTED()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ARP_TIMER_EXPIRED
 *
 * A packet was stored for delayed processing, but there is no ARP reply. The
 * network buffer will be released without being processed.
 */
#ifndef iptraceDELAYED_ARP_TIMER_EXPIRED
    #define iptraceDELAYED_ARP_TIMER_EXPIRED()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceDROPPED_INVALID_ARP_PACKET
 *
 * Called when an ARP packet is dropped due to invalid protocol and hardware
 * fields in the header at address pxARPHeader.
 */
#ifndef iptraceDROPPED_INVALID_ARP_PACKET
    #define iptraceDROPPED_INVALID_ARP_PACKET( pxARPHeader )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptracePACKET_DROPPED_TO_GENERATE_ARP
 *
 * Called when a packet destined for the IP address ulIPAddress is dropped
 * because the ARP cache does not contain an entry for the IP address. The
 * packet is automatically replaced by an ARP packet. ulIPAddress is expressed
 * as a 32-bit number in network byte order.
 */
#ifndef iptracePACKET_DROPPED_TO_GENERATE_ARP
    #define iptracePACKET_DROPPED_TO_GENERATE_ARP( ulIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptracePROCESSING_RECEIVED_ARP_REPLY
 *
 * Called when the ARP cache is about to be updated in response to the
 * reception of an ARP reply. ulIPAddress holds the ARP message's target IP
 * address (as a 32-bit number in network byte order), which may not be the
 * local network node (depending on the FreeRTOSIPConfig.h settings).
 */
#ifndef iptracePROCESSING_RECEIVED_ARP_REPLY
    #define iptracePROCESSING_RECEIVED_ARP_REPLY( ulIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDING_ARP_REPLY
 *
 * An ARP reply is being sent in response to an ARP request from the IP address
 * ulIPAddress. ulIPAddress is expressed as a 32-bit number in network byte
 * order.
 */
#ifndef iptraceSENDING_ARP_REPLY
    #define iptraceSENDING_ARP_REPLY( ulIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                             ARP TRACE MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             ND TRACE MACROS                               */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceND_TABLE_ENTRY_EXPIRED
 */
#ifndef iptraceND_TABLE_ENTRY_EXPIRED
    #define iptraceND_TABLE_ENTRY_EXPIRED( pxIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceND_TABLE_ENTRY_WILL_EXPIRE
 */
#ifndef iptraceND_TABLE_ENTRY_WILL_EXPIRE
    #define iptraceND_TABLE_ENTRY_WILL_EXPIRE( pxIPAddress )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ND_BUFFER_FULL
 *
 * A packet has come in from an unknown IPv6 address. An ND request has been
 * sent, but the queue is still filled with a different packet.
 */
#ifndef iptraceDELAYED_ND_BUFFER_FULL
    #define iptraceDELAYED_ND_BUFFER_FULL()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptrace_DELAYED_ND_REQUEST_REPLIED
 */
#ifndef iptrace_DELAYED_ND_REQUEST_REPLIED
    #define iptrace_DELAYED_ND_REQUEST_REPLIED()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ND_REQUEST_STARTED
 *
 * A packet came in from an unknown IPv6 address. An ND request has been sent
 * and the network buffer is stored for processing later.
 */
#ifndef iptraceDELAYED_ND_REQUEST_STARTED
    #define iptraceDELAYED_ND_REQUEST_STARTED()
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ND_TIMER_EXPIRED
 *
 * A packet was stored for delayed processing, but there is no ND reply. The
 * network buffer will be released without being processed.
 */
#ifndef iptraceDELAYED_ND_TIMER_EXPIRED
    #define iptraceDELAYED_ND_TIMER_EXPIRED()
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                              ND TRACE MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             DHCP TRACE MACROS                             */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS
 *
 * Called when the default IP address is used because an IP address could not
 * be obtained from a DHCP. ulIPAddress is expressed as a 32-bit number in
 * network byte order.
 */
#ifndef iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS
    #define iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS( ulIPAddress )
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IPv6_ADDRESS
 */
#ifndef iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IPv6_ADDRESS
    #define iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IPv6_ADDRESS( xIPAddress )
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceDHCP_SUCCEEDED
 *
 * Called when DHCP negotiation is complete and the IP address in
 * ulOfferedIPAddress is offered to the device.
 */
#ifndef iptraceDHCP_SUCCEEDED
    #define iptraceDHCP_SUCCEEDED( ulOfferedIPAddress )
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceSENDING_DHCP_DISCOVER
 *
 * Called when a DHCP discover packet is sent.
 */
#ifndef iptraceSENDING_DHCP_DISCOVER
    #define iptraceSENDING_DHCP_DISCOVER()
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceSENDING_DHCP_REQUEST
 *
 * Called when a DHCP request packet is sent.
 */
#ifndef iptraceSENDING_DHCP_REQUEST
    #define iptraceSENDING_DHCP_REQUEST()
#endif

/*-----------------------------------------------------------------------*/

/*===========================================================================*/
/*                             DHCP TRACE MACROS                             */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             DNS TRACE MACROS                              */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDING_DNS_REQUEST
 *
 * Called when a DNS request is sent.
 */
#ifndef iptraceSENDING_DNS_REQUEST
    #define iptraceSENDING_DNS_REQUEST()
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                             DNS TRACE MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             ICMP TRACE MACROS                             */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceICMP_PACKET_RECEIVED
 *
 * Called when an ICMP packet is received.
 */
#ifndef iptraceICMP_PACKET_RECEIVED
    #define iptraceICMP_PACKET_RECEIVED()
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceSENDING_PING_REPLY
 *
 * Called when an ICMP echo reply (ping reply) is sent to the IP address
 * ulIPAddress in response to an ICMP echo request (ping request) originating
 * from the same address. ulIPAddress is expressed as a 32-bit number in
 * network byte order.
 */
#ifndef iptraceSENDING_PING_REPLY
    #define iptraceSENDING_PING_REPLY( ulIPAddress )
#endif

/*-----------------------------------------------------------------------*/

/*===========================================================================*/
/*                             ICMP TRACE MACROS                             */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                      ROUTER ADVERTISEMENT TRACE MACROS                    */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceRA_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS
 */
#ifndef iptraceRA_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS
    #define iptraceRA_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS( ipv6_address )
#endif

/*---------------------------------------------------------------------------*/

/*
 * iptraceRA_SUCCEEDED
 */
#ifndef iptraceRA_SUCCEEDED
    #define iptraceRA_SUCCEEDED( ipv6_address )
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                      ROUTER ADVERTISEMENT TRACE MACROS                    */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             MEM STATS MACROS                              */
/*===========================================================================*/

/*-----------------------------------------------------------------------*/

/*
 * iptraceMEM_STATS_CLOSE
 *
 * Should be called by the application when the collection of memory
 * statistics should be stopped.
 */
#ifndef iptraceMEM_STATS_CLOSE
    #define iptraceMEM_STATS_CLOSE()
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceMEM_STATS_CREATE
 *
 * Called when an object at address pxObject of type xMemType and size
 * uxSize has been allocated from the heap.
 */
#ifndef iptraceMEM_STATS_CREATE
    #define iptraceMEM_STATS_CREATE( xMemType, pxObject, uxSize )
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceMEM_STATS_DELETE
 *
 * Called when an object at address pxObject has been deallocated and the
 * memory has been returned to the heap.
 */
#ifndef iptraceMEM_STATS_DELETE
    #define iptraceMEM_STATS_DELETE( pxObject )
#endif

/*-----------------------------------------------------------------------*/

/*===========================================================================*/
/*                             MEM STATS MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                           TCP DUMP TRACE MACROS                           */
/*===========================================================================*/

/*-----------------------------------------------------------------------*/

/*
 * iptraceDUMP_INIT
 */
#ifndef iptraceDUMP_INIT
    #define iptraceDUMP_INIT( pcFileName, pxEntries )
#endif

/*-----------------------------------------------------------------------*/

/*
 * iptraceDUMP_PACKET
 */
#ifndef iptraceDUMP_PACKET
    #define iptraceDUMP_PACKET( pucBuffer, uxLength, xIncoming )
#endif

/*-----------------------------------------------------------------------*/

/*===========================================================================*/
/*                           TCP DUMP TRACE MACROS                           */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/

#endif /* IP_TRACE_MACRO_DEFAULTS_H */
