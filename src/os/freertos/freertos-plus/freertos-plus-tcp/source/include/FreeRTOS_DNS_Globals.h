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

#ifndef FREERTOS_DNS_GLOBALS_H
#define FREERTOS_DNS_GLOBALS_H

#include "FreeRTOS.h"

#include "FreeRTOSIPConfig.h"
#include "FreeRTOSIPConfigDefaults.h"

#include "FreeRTOS_Sockets.h"

#define dnsPARSE_ERROR              0UL

#if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )
    #define dnsDNS_PORT             0x3500U     /**< Little endian: Port used for DNS. */
    #define dnsONE_QUESTION         0x0100U     /**< Little endian representation of a DNS question.*/
    #define dnsOUTGOING_FLAGS       0x0001U     /**< Little endian representation of standard query. */
    #define dnsRX_FLAGS_MASK        0x0f80U     /**< Little endian:  The bits of interest in the flags field of incoming DNS messages. */
    #define dnsEXPECTED_RX_FLAGS    0x0080U     /**< Little Endian: Should be a response, without any errors. */
#else
    #define dnsDNS_PORT             0x0035U     /**< Big endian: Port used for DNS. */
    #define dnsONE_QUESTION         0x0001U     /**< Big endian representation of a DNS question.*/
    #define dnsOUTGOING_FLAGS       0x0100U     /**< Big endian representation of standard query. */
    #define dnsRX_FLAGS_MASK        0x800fU     /**< Big endian: The bits of interest in the flags field of incoming DNS messages. */
    #define dnsEXPECTED_RX_FLAGS    0x8000U     /**< Big endian: Should be a response, without any errors. */

#endif /* ipconfigBYTE_ORDER */
#if ( ipconfigUSE_DNS != 0 )

/** @brief If the top two bits in the first character of a name field are set then the
 * name field is an offset to the string, rather than the string itself. */
    #define dnsNAME_IS_OFFSET    ( ( uint8_t ) 0xc0 )

/* NBNS flags. */
    #if ( ipconfigUSE_NBNS == 1 )
        #define dnsNBNS_FLAGS_RESPONSE        0x8000U /**< NBNS response flag. */
        #define dnsNBNS_FLAGS_OPCODE_MASK     0x7800U /**< NBNS opcode bitmask. */
        #define dnsNBNS_FLAGS_OPCODE_QUERY    0x0000U /**< NBNS opcode query. */
    #endif /* ( ipconfigUSE_NBNS == 1 ) */

/* Host types. */
    #define dnsTYPE_A_HOST            0x01U /**< DNS type A host. */
    #define dnsTYPE_AAAA_HOST         0x001CU
    #define dnsTYPE_ANY_HOST          0x00FFU

    #define dnsCLASS_IN               0x01U /**< DNS class IN (Internet). */

/* Maximum hostname length as defined in RFC 1035 section 3.1. */
    #define dnsMAX_HOSTNAME_LENGTH    0xFFU

    #ifndef _lint
        /* LLMNR constants. */
        #define dnsLLMNR_TTL_VALUE            300U    /**< LLMNR time to live value of 5 minutes. */
        #define dnsLLMNR_FLAGS_IS_RESPONSE    0x8000U /**< LLMNR flag value for response. */
    #endif /* _lint */

/* NBNS constants. */
    #if ( ipconfigUSE_NBNS != 0 )
        #define dnsNBNS_TTL_VALUE               3600UL  /**< NBNS TTL: 1 hour valid. */
        #define dnsNBNS_TYPE_NET_BIOS           0x0020U /**< NBNS Type: NetBIOS. */
        #define dnsNBNS_CLASS_IN                0x01U   /**< NBNS Class: IN (Internet). */
        #define dnsNBNS_NAME_FLAGS              0x6000U /**< NBNS name flags. */
        #define dnsNBNS_ENCODED_NAME_LENGTH     32      /**< NBNS encoded name length. */

/** @brief If the queried NBNS name matches with the device's name,
 * the query will be responded to with these flags: */
        #define dnsNBNS_QUERY_RESPONSE_FLAGS    ( 0x8500U )
    #endif /* ( ipconfigUSE_NBNS != 0 ) */

/** @brief Define the ASCII value of '.' (Period/Full-stop). */
    #define ASCII_BASELINE_DOT    46U

/* The Link-local Multicast Name Resolution (LLMNR)
 * is included.
 * Note that a special MAC address is required in addition to the NIC's actual
 * MAC address: 01:00:5E:00:00:FC
 *
 * The target IP address will be 224.0.0.252
 */
    #if ( ipconfigBYTE_ORDER == pdFREERTOS_BIG_ENDIAN )
        #define ipLLMNR_IP_ADDR    0xE00000FCUL
    #else
        #define ipLLMNR_IP_ADDR    0xFC0000E0UL
    #endif /* ipconfigBYTE_ORDER == pdFREERTOS_BIG_ENDIAN */

    #define ipMDNS_TIME_TO_LIVE    255U

    #define ipLLMNR_PORT           5355U  /* Standard LLMNR port. */
    #define ipDNS_PORT             53U    /* Standard DNS port. */
    #define ipNBNS_PORT            137U   /* NetBIOS Name Service. */
    #define ipNBDGM_PORT           138U   /* Datagram Service, not included. */

/** @brief freertos_addrinfo is the equivalent of 'struct addrinfo'. */
    struct freertos_addrinfo
    {
        BaseType_t ai_flags;                /**< The field is included for completeness, but it is not used. */
        BaseType_t ai_family;               /**< The type of IP-address, either FREERTOS_AF_INET4 or FREERTOS_AF_INET6. */
        BaseType_t ai_socktype;             /**< n.a. */
        BaseType_t ai_protocol;             /**< n.a. */
        socklen_t ai_addrlen;               /**< The length of the address, either ipSIZE_OF_IPv4_ADDRESS or ipSIZE_OF_IPv6_ADDRESS. */
        struct freertos_sockaddr * ai_addr; /**< The socket address. */
        char * ai_canonname;                /**< The name of the host. */
        struct freertos_addrinfo * ai_next; /**< A pointer to the next find result, or NULL. */
        struct
        {
            /* In order to avoid allocations, reserve space here for *ai_addr and *ai_canonname. */
            struct freertos_sockaddr sockaddr;

            #if ( ipconfigUSE_DNS_CACHE != 0 )
                char ucName[ ipconfigDNS_CACHE_NAME_LENGTH ];
            #endif
        }
        xPrivateStorage; /**< In order to avoid extra calls to malloc, the necessary space is reserved 'statically'. */
    };
/* DNS answer record header. */
    #include "pack_struct_start.h"
    struct xDNSAnswerRecord
    {
        uint16_t usType;       /**< Type of DNS answer record. */
        uint16_t usClass;      /**< Class of DNS answer record. */
        uint32_t ulTTL;        /**< Number of seconds the result can be cached. */
        uint16_t usDataLength; /**< Length of the data field. */
    }
    #include "pack_struct_end.h"
    typedef struct xDNSAnswerRecord DNSAnswerRecord_t;

/* Below #include just tells the compiler to pack the structure.
 * It is included in to make the code more readable */
    #include "pack_struct_start.h"
    struct xDNSMessage
    {
        uint16_t usIdentifier;    /**< Query identifier. Used to match up replies to outstanding queries. */
        uint16_t usFlags;         /**< Flags. */
        uint16_t usQuestions;     /**< Number of questions asked in this query. */
        uint16_t usAnswers;       /**< Number of answers being provided in this query. */
        uint16_t usAuthorityRRs;  /**< Authoritative name server resource records. */
        uint16_t usAdditionalRRs; /**< Additional resource records.*/
    }
    #include "pack_struct_end.h"
    typedef struct xDNSMessage DNSMessage_t;

/** @brief A struct with a set of variables that are shared among the helper functions
 *         for the function 'DNS_ParseDNSReply()'. For internal use only.
 */
    typedef struct xParseSet
    {
        DNSMessage_t * pxDNSMessageHeader; /**< A pointer to the UDP payload buffer where the DNS message is stored. */
        uint16_t usQuestions;              /**< The number of DNS questions that were asked. */
        uint16_t usAnswers;                /**< The number of DNS answers that were given. */
        uint8_t * pucUDPPayloadBuffer;     /**< A pointer to the original UDP load buffer. */
        uint8_t * pucByte;                 /**< A pointer that is used while parsing. */
        size_t uxBufferLength;             /**< The total number of bytes received in the UDP payload. */
        size_t uxSourceBytesRemaining;     /**< As pucByte is incremented, 'uxSourceBytesRemaining' will be decremented. */
        uint16_t usType;                   /**< The type of address, recognised are dnsTYPE_A_HOST ( Ipv4 ) and
                                            *   dnsTYPE_AAAA_HOST ( IPv6 ). */
        uint32_t ulIPAddress;              /**< The IPv4 address found. In an IPv6 look-up, store a non-zero value when
                                            *   an IPv6 address was found. */
        size_t uxAddressLength;            /**< The size of the address, either ipSIZE_OF_IPv4_ADDRESS or
                                            *   ipSIZE_OF_IPv6_ADDRESS */
        uint16_t usNumARecordsStored;      /**< The number of A-records stored during a look-up. */
        uint16_t usPortNumber;             /**< The port number that belong to the protocol ( DNS, MDNS etc ). */
        #if ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_MDNS == 1 )
            uint16_t usClass;              /**< Only the value 'dnsCLASS_IN' is recognised, which stands for "Internet". */
            char * pcRequestedName;        /**< A pointer to the full name of the host being looked up. */
        #endif

        #if ( ( ipconfigUSE_DNS_CACHE != 0 ) || ( ipconfigDNS_USE_CALLBACKS != 0 ) || ( ipconfigUSE_MDNS != 0 ) || ( ipconfigUSE_LLMNR != 0 ) )
            BaseType_t xDoStore;                          /**< Becomes true when a DNS reply was requested by this device,
                                                           *   i.e. it has a matching request ID. */
            char pcName[ ipconfigDNS_CACHE_NAME_LENGTH ]; /**< A copy of the name that is mentioned in the questions. */
        #endif
        struct freertos_addrinfo * pxLastAddress;         /**< This variable is used while creating a linked-list of IP-addresses. */
        struct freertos_addrinfo ** ppxLastAddress;       /**< This variable is also used while creating a linked-list of IP-addresses. */
    } ParseSet_t;
    #if ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_MDNS == 1 )

        #include "pack_struct_start.h"
        struct xLLMNRAnswer
        {
            uint8_t ucNameCode;    /**< Name type. */
            uint8_t ucNameOffset;  /**< The name is not repeated in the answer, only the offset is given with "0xc0 <offs>" */
            uint16_t usType;       /**< Type of the Resource record. */
            uint16_t usClass;      /**< Class of the Resource record. */
            uint32_t ulTTL;        /**< Seconds till this entry can be cached. */
            uint16_t usDataLength; /**< Length of the address in this record. */
            uint32_t ulIPAddress;  /**< The IP-address. */
        }
        #include "pack_struct_end.h"
        typedef struct xLLMNRAnswer LLMNRAnswer_t;
    #endif /* if ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_MDNS == 1 ) */

    #if ( ipconfigUSE_NBNS == 1 )

        #include "pack_struct_start.h"
        struct xNBNSRequest
        {
            uint16_t usRequestId;                          /**< NBNS request ID. */
            uint16_t usFlags;                              /**< Flags of the DNS message. */
            uint16_t ulRequestCount;                       /**< The number of requests/questions in this query. */
            uint16_t usAnswerRSS;                          /**< The number of answers in this query. */
            uint16_t usAuthRSS;                            /**< Number of authoritative resource records. */
            uint16_t usAdditionalRSS;                      /**< Number of additional resource records. */
            uint8_t ucNameSpace;                           /**< Length of name. */
            uint8_t ucName[ dnsNBNS_ENCODED_NAME_LENGTH ]; /**< The domain name. */
            uint8_t ucNameZero;                            /**< Terminator of the name. */
            uint16_t usType;                               /**< Type of NBNS record. */
            uint16_t usClass;                              /**< Class of NBNS request. */
        }
        #include "pack_struct_end.h"
        typedef struct xNBNSRequest NBNSRequest_t;

        #include "pack_struct_start.h"
        struct xNBNSAnswer
        {
            uint16_t usType;       /**< Type of NBNS answer. */
            uint16_t usClass;      /**< Class of NBNS answer. */
            uint32_t ulTTL;        /**< Time in seconds for which the answer can be cached. */
            uint16_t usDataLength; /**< Data length. */
            uint16_t usNbFlags;    /**< NetBIOS flags 0x6000 : IP-address, big-endian. */
            uint32_t ulIPAddress;  /**< The IPv4 address. */
        }
        #include "pack_struct_end.h"
        typedef struct xNBNSAnswer NBNSAnswer_t;
    #endif /* if ( ipconfigUSE_NBNS == 1 ) */

    #if ( ipconfigDNS_USE_CALLBACKS != 0 )

/*
 * Users may define this type of function as a callback.
 * It will be called when a DNS reply is received or when a timeout has been reached.
 */
        typedef void (* FOnDNSEvent ) ( const char * /* pcName */,
                                        void * /* pvSearchID */,
                                        struct freertos_addrinfo * /* pxAddressInfo */ );
        /** @brief The structure to hold information for a DNS callback. */
        typedef struct xDNS_Callback
        {
            TickType_t uxRemainingTime;    /**< Timeout in ms */
            FOnDNSEvent pCallbackFunction; /**< Function to be called when the address has been found or when a timeout has been reached */
            TimeOut_t uxTimeoutState;      /**< Timeout state. */
            void * pvSearchID;             /**< Search ID of the callback function. */
            struct xLIST_ITEM xListItem;   /**< List struct. */
            BaseType_t xIsIPv6;            /**< Remember whether this was a IPv6 lookup. */
            char pcName[ 1 ];              /**< 1 character name. */
        } DNSCallback_t;
    #endif /* if ( ipconfigDNS_USE_CALLBACKS != 0 ) */

/**
 * @brief structure to hold the buffer, its size and the data length
 */
    typedef struct xDNSBuffer
    {
        uint8_t * pucPayloadBuffer; /**< Buffer pointer */
        size_t uxPayloadLength;     /**< Payload size */
    } DNSBuffer_t;

    #if ( ipconfigUSE_MDNS == 1 ) || ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_NBNS == 1 )

/*
 * The following function should be provided by the user and return true if it
 * matches the domain name.
 */
        #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
            /* Even though the function is defined in main.c, the rule is violated. */
            /* misra_c_2012_rule_8_6_violation */
            extern BaseType_t xApplicationDNSQueryHook( const char * pcName );
        #else
            /* Even though the function is defined in main.c, the rule is violated. */
            /* misra_c_2012_rule_8_6_violation */
            extern BaseType_t xApplicationDNSQueryHook_Multi( struct xNetworkEndPoint * pxEndPoint,
                                                              const char * pcName );
        #endif

    #endif /* ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_NBNS == 1 ) */
#endif /* ipconfigUSE_DNS */

/* Keeping this outside of ipconfigUSE_DNS flag as these are used inside IPv4 UDP code */
#define ipMDNS_PORT              5353U       /* Standard mDNS port. */

#if ( ipconfigBYTE_ORDER == pdFREERTOS_BIG_ENDIAN )
    #define ipMDNS_IP_ADDRESS    0xe00000fbU /* 224.0.0.251 */
#else
    #define ipMDNS_IP_ADDRESS    0xfb0000e0U /* 224.0.0.251 */
#endif

#endif /* FREERTOS_DNS_GLOBALS_H */
