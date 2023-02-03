# Checking Operating System Specific Implementations

Including operating system headers should be avoided.
Instead the alternatives defined in the ``task``-wrapper layer
(``src/app/task``) should be used.

## FreeRTOS

Only the ``FreeRTOS`` specific implementations

- ``src/app/task/os/freertos/os_freertos.c``
- ``src/app/task/ftask/freertos/ftask_freertos.c``

might use ``FreeRTOS`` specific includes, types etc.

### Functions

The following regular expression can be used to check for ``FreeRTOS`` specific
- macros:
  ```
  (taskENTER_CRITICAL|taskEXIT_CRITICAL)
  ```
- functions:
  ```
  (uxQueueMessagesWaiting|vTaskDelayUntil|xQueueReceive|xQueueSendToBack|xQueueSendToBackFromISR|xTaskGetTickCount)
  ```
- types:
  ```
  (QueueHandle_t|TaskHandle_t)
  ```

The corresponding OS agnostic macros, functions and types are listed below:

| FreeRTOS                      | foxBMS                            |
|-------------------------------|-----------------------------------|
| ``taskENTER_CRITICAL``        | ``OS_EnterTaskCritical``          |
| ``taskEXIT_CRITICAL``         | ``OS_ExitTaskCritical``           |
| ``uxQueueMessagesWaiting``    | ``OS_GetTickCount``               |
| ``vTaskDelayUntil``           | ``OS_DelayTaskUntil``             |
| ``xQueueReceive``             | ``OS_ReceiveFromQueue``           |
| ``xQueueSendToBack``          | ``OS_SendToBackOfQueue``          |
| ``xQueueSendToBackFromISR``   | ``OS_SendToBackOfQueueFromIsr``   |
| ``xTaskGetTickCount``         | ``OS_GetTickCount``               |
| ``QueueHandle_t``             | ``OS_QUEUE``                      |
| ``TaskHandle_t``              | ``OS_TASK_HANDLE``                |

### Includes

The following regular expression can be used to check for ``FreeRTOS`` specific
includes:

```
#include.*"(atomic.h|croutine.h|deprecated_definitions.h|event_groups.h|FreeRTOS.h|FreeRTOSConfig.h|list.h|message_buffer.h|mpu_prototypes.h|mpu_wrappers.h|portable.h|projdefs.h|queue.h|semphr.h|StackMacros.h|stack_macros.h|stream_buffer.h|task.h|timers.h)
```

### Types and Defines

The following regular expression can be used to check for ``FreeRTOS`` specific
types and defines:

```
(pdFALSE|pdTRUE|pdPASS|pdFAIL)
```
