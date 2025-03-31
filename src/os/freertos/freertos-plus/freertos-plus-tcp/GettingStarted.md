Moving to version 4.0.0 and later from 3.x.x:
--------------------------------

Version 4.0.0 and later adds new files to support IPv6 functionality, breaking each file into logically separated IPv4 and IPv6 files. The folder structure of FreeRTOS-Plus-TCP remains unchanged. Build separation is added to make the library modular, enabling users to compile and create a low-footprint binary with only the required functionalities. For more details on supported build combinations, see [History.txt](https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/History.txt) .

Some of the APIs have changed which is illustrated in the section below. However, there is a backward compatibility mode provided as well.

Backward Compatibility Mode:
---------------------------
   If you are moving your code from version 3.x.x to version 4.0.0 and later, then set the "ipconfigIPv4_BACKWARD_COMPATIBLE" macro to 1 in “FreeRTOSIPConfigDefaults.h” to run the code in backward compatible mode.
   The "Existing API's" defined in all the API changes below work only when the backward compatibility mode is enabled.

API changes in version 4.0.0 and later:
----------------------

The changes mentioned below uses ipconfigIPv4_BACKWARD_COMPATIBLE flag to differentiate between old API and new API.

Change 1:

   - Existing API: FreeRTOS_IPInit
      - Backward compatibility with the IPv4 FreeRTOS+TCP V3.x.x which only supports single network interface. This can be used for single  interface IPv4 systems.
   - New API: FreeRTOS_IPInit_Multi
      - Supports multiple interfaces. Before calling this function, at least 1 interface and 1 end-point must have been set-up. FreeRTOS_IPInit_Multi() replaces the earlier FreeRTOS_IPInit().

Change 2:

   - Existing API: FreeRTOS_GetAddressConfiguration/FreeRTOS_SetAddressConfiguration
      - Get/Set the address configuration from the global variables initialised during FreeRTOS_IPInit
   - New API: FreeRTOS_GetEndPointConfiguration/FreeRTOS_SetEndPointConfiguration
      - Get/Set the same address configuration from/to the end point

Change 3:

   - Existing API:  FreeRTOS_GetUDPPayloadBuffer
      - Backward compatibility with the IPv4 FreeRTOS+TCP V3.x.x. This can still be used for IPv4 use cases.
   - New API:FreeRTOS_GetUDPPayloadBuffer_Multi
      - A new argument (uint8_t ucIPType) to specify IP type to support both IPv4 and IPv6

Change 4:

   - Existing API: pxFillInterfaceDescriptor
      - It is there for backward compatibility. The function FreeRTOS_IPInit() will call it to initialise the interface and end-point objects
   - New API: prefix_pxFillInterfaceDescriptor
      - where prefix = Network Interface Name
      - E.g pxWinPcap_FillInterfaceDescriptor
      - New function with the same functionality

Change 5:

   - Existing API: vApplicationIPNetworkEventHook
   - New API: vApplicationIPNetworkEventHook_Multi
      - New argument “struct xNetworkInterface * pxNetworkInterface” added.

Change 6:

   - Existing API: xApplicationDHCPHook
   - New API: xApplicationDHCPHook_Multi
      - New argument "struct xNetworkEndPoint * pxEndPoint" added.

Change 7:

   - Existing API: xApplicationDNSQueryHook
   - New API: xApplicationDNSQueryHook_Multi
      - New argument "struct xNetworkEndPoint * pxEndPoint" added.

**NOTE** : We are NOT considering the APIs changes in FreeRTOS_IP_Private.h for backward compatibility as those are not part of published interface.

Running Demos:
-------------
The demos can be found on [this page](https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS-Plus/Demo).

In all the demos, there is a backward compatibility mode which can be enabled by setting the flag “ipconfigIPv4_BACKWARD_COMPATIBLE” to 1 in the header file “FreeRTOSIPConfigDefaults.h”.
This flag is by default set to zero.

New IPv6 WinSim Demo can be found on [this page](https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS-Plus/Demo/FreeRTOS_Plus_TCP_IPv6_Demo/IPv6_Multi_WinSim_demo).
