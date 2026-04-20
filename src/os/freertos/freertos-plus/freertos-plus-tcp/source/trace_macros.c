#include "trace_macros.h"
#include "FreeRTOS_IP.h"
#include <stdio.h>

char buffer[40u] = "";

 char * TRC_Convert32BitToIpFormat(uint32_t ipAddress){
    FreeRTOS_inet_ntop(FREERTOS_AF_INET4,( const void * )&ipAddress, buffer, sizeof(buffer));
     return buffer;
}
