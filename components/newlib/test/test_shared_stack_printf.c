#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"
#include "test_utils.h"
#include "esp_expression_with_stack.h"

#define SHARED_STACK_SIZE 8192

static StackType_t *shared_stack_sp = NULL; 

void external_stack_function(void)
{
    printf("Executing this printf from external stack! sp=%p\n", get_sp());
    shared_stack_sp = (StackType_t *)get_sp();
}

void another_external_stack_function(void) 
{
    //We can even use Freertos resources inside of this context.
    vTaskDelay(100);
    printf("Done!, sp=%p\n", get_sp());
    TEST_ASSERT_NOT_NULL(printf_lock);

    esp_execute_shared_stack_function(printf_lock, 
                                    shared_stack,
                                    SHARED_STACK_SIZE,
                                    external_stack_function);
    
    TEST_ASSERT(((shared_stack_sp >= shared_stack_sp) && 
                (shared_stack_sp < (shared_stack + SHARED_STACK_SIZE))));
    
    esp_execute_shared_stack_function(printf_lock, 
                                    shared_stack,
                                    SHARED_STACK_SIZE,
                                    another_external_stack_function); 
    
    TEST_ASSERT(((shared_stack_sp >= shared_stack_sp) && 
                (shared_stack_sp < (shared_stack + SHARED_STACK_SIZE))));

    vSemaphoreDelete(printf_lock);   
    free(shared_stack);
}

