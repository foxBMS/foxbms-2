# Checking Operating System Specific Implementations

Including operating system headers should be avoided.
Instead the alternatives defined in the ``task``-wrapper layer (``src/app/task``) should be used.

## FreeRTOS

Only the ``FreeRTOS`` specific implementations

- ``src/app/task/os/freertos/os_freertos.c``
- ``src/app/task/ftask/freertos/ftask_freertos.c``

might use ``FreeRTOS`` specific includes, types etc.

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
