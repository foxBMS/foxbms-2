/*
 * FreeRTOS+TCP V2.3.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

#ifndef FREERTOS_ND_H
#define FREERTOS_ND_H

#include "FreeRTOS.h"

/* Application level configuration options. */
#include "FreeRTOSIPConfig.h"
#include "FreeRTOSIPConfigDefaults.h"

#include "FreeRTOS_ARP.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

#if ( ipconfigUSE_IPv6 != 0 )
/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

/**
 * @brief 'NDCacheRow_t' defines one row in the ND address cache.
 * @note About A value that is periodically decremented but can
 *       also be refreshed by active communication.  The ND cache entry
 *       is removed if the value reaches zero.
 */
    typedef struct xND_CACHE_TABLE_ROW
    {
        IPv6_Address_t xIPAddress;            /**< The IP address of an ND cache entry. */
        MACAddress_t xMACAddress;             /**< The MAC address of an ND cache entry. */
        struct xNetworkEndPoint * pxEndPoint; /**< The end-point on which the
                                               * remote device had responded. */
        uint8_t ucAge;                        /**< See here above. */
        uint8_t ucValid;                      /**< pdTRUE: xMACAddress is valid, pdFALSE: waiting for ND reply */
    } NDCacheRow_t;

/*
 * If ulIPAddress is already in the ND cache table then reset the age of the
 * entry back to its maximum value.  If ulIPAddress is not already in the ND
 * cache table then add it - replacing the oldest current entry if there is not
 * a free space available.
 */
    void vNDRefreshCacheEntry( const MACAddress_t * pxMACAddress,
                               const IPv6_Address_t * pxIPAddress,
                               NetworkEndPoint_t * pxEndPoint );

/** @brief Options that can be sent in a ROuter Advertisement packet. */
    #define ndICMP_SOURCE_LINK_LAYER_ADDRESS    1
    #define ndICMP_TARGET_LINK_LAYER_ADDRESS    2
    #define ndICMP_PREFIX_INFORMATION           3
    #define ndICMP_REDIRECTED_HEADER            4
    #define ndICMP_MTU_OPTION                   5

/*
 * @brief Send a neighbour solicitation.
 * @param[in] pxIPAddress: A network buffer big enough to hold the ICMP packet.
 * @param[in,out] pxMACAddress: When found, the array of 6 bytes will be filled
 *                with the MAC-address.
 * @param[in,out] ppxEndPoint: The pointer to a pointer will point to an
 *                             end-point to which the device has responded.
 *
 * @note Look for ulIPAddress in the ND cache.  If the IP address exists, copy the
 * associated MAC address into pxMACAddress, refresh the ND cache entry's
 * age, and return eARPCacheHit.  If the IP address does not exist in the ND
 * cache return eARPCacheMiss.  If the packet cannot be sent for any reason
 * (maybe DHCP is still in process, or the addressing needs a gateway but there
 * isn't a gateway defined) then return eCantSendPacket.
 */
    eARPLookupResult_t eNDGetCacheEntry( IPv6_Address_t * pxIPAddress,
                                         MACAddress_t * const pxMACAddress,
                                         struct xNetworkEndPoint ** ppxEndPoint );

/**
 * @brief Reduce the age counter in each entry within the ND cache.  An entry is no
 * longer considered valid and is deleted if its age reaches zero.
 * Just before getting to zero, 3 times a neighbour solicitation will be sent.
 */
    void vNDAgeCache( void );

/**
 * @brief Send a neighbour solicitation.
 * @param[in] pxNetworkBuffer: A network buffer big enough to hold the ICMP packet.
 * @param[in] pxIPAddress: The IPv6 address of the target device.
 *
 * @note Send out an ND request for the IPv6 address contained in pxNetworkBuffer, and
 * add an entry into the ND table that indicates that an ND reply is
 * outstanding so re-transmissions can be generated.
 */
    void vNDSendNeighbourSolicitation( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                       const IPv6_Address_t * pxIPAddress );

    #if ( ipconfigUSE_RA != 0 )

/**
 * @brief Send a router solicitation.
 * @param[in] pxNetworkBuffer: A network buffer big enough to hold the ICMP packet.
 * @param[in] pxIPAddress: The multi-cast address of the routers ( normally ff02::2 ).
 */
        void vNDSendRouterSolicitation( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                        IPv6_Address_t * pxIPAddress );
    #endif /* ( ipconfigUSE_RA != 0 ) */

    #if ( ipconfigUSE_RA != 0 )

/**
 * @brief Work on the RA/SLAAC processing.
 * @param[in] xDoReset: WHen true, the state-machine will be reset and initialised.
 * @param[in] pxEndPoint: The end-point for which the RA/SLAAC process should be done..
 */
        void vRAProcess( BaseType_t xDoReset,
                         NetworkEndPoint_t * pxEndPoint );
    #endif /* ( ipconfigUSE_RA != 0 ) */

/**
 * @brief Send an ND advertisement.
 * @param[in] pxEndPoint: The end-point for which an ND advertisement should be sent.
 */
    void FreeRTOS_OutputAdvertiseIPv6( NetworkEndPoint_t * pxEndPoint );
    #if ( ipconfigSUPPORT_OUTGOING_PINGS == 1 )

/**
 * @brief Send an IPv6 ping message to a remote device.
 * @param[in] pxIPAddress: The IPv6 address of the other device.
 * @param[in] uxNumberOfBytesToSend: The number of bytes to be echoed.
 * @param[in] uxBlockTimeTicks: The number of clock-tick to wait
 *            for space in the IP-task queue.
 * @return pdTRUE when a packets was successfully created
 *         and passed to the IP-task.
 */
        BaseType_t FreeRTOS_SendPingRequestIPv6( const IPv6_Address_t * pxIPAddress,
                                                 size_t uxNumberOfBytesToSend,
                                                 TickType_t uxBlockTimeTicks );
    #endif

/**
 * @brief Create an IPv16 address, based on a prefix.
 *
 * @param[out] pxIPAddress: The location where the new IPv6 address
 *                          will be stored.
 * @param[in] pxPrefix: The prefix to be used.
 * @param[in] uxPrefixLength: The length of the prefix.
 * @param[in] xDoRandom: A non-zero value if the bits after the
 *                       prefix should have a random value.
 *
 * @return pdPASS if the operation was successful. Or pdFAIL in
 *         case xApplicationGetRandomNumber()
 *         returned an error.
 */
    BaseType_t FreeRTOS_CreateIPv6Address( IPv6_Address_t * pxIPAddress,
                                           const IPv6_Address_t * pxPrefix,
                                           size_t uxPrefixLength,
                                           BaseType_t xDoRandom );

/* Receive a Neighbour Advertisement. */

    #if ( ipconfigUSE_RA != 0 )

/** @brief A neighbour advertisement has been received. Store its
 *         address in the ND address cache.
 *  @param[in] pxNetworkBuffer The buffer containing the packet.
 */
        void vReceiveNA( const NetworkBufferDescriptor_t * pxNetworkBuffer );
    #endif

/* Receive a Router Advertisement. */
    #if ( ipconfigUSE_RA != 0 )

/** @brief A router advertisement has been received.  See if it is
 *         applicable for this device.
 *  @param[in] pxNetworkBuffer The buffer containing the packet.
 */
        void vReceiveRA( const NetworkBufferDescriptor_t * pxNetworkBuffer );
    #endif

    #if ( ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 ) )
/** @brief Print the contents of the ND cache, for debugging only. */
        void FreeRTOS_PrintNDCache( void );
    #endif

    extern const uint8_t pcLOCAL_ALL_NODES_MULTICAST_IP[ ipSIZE_OF_IPv6_ADDRESS ];
    extern const uint8_t pcLOCAL_ALL_NODES_MULTICAST_MAC[ ipMAC_ADDRESS_LENGTH_BYTES ];
#endif /* ipconfigUSE_IPv6 != 0 */


/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_ND_H */
