#include "w5500evb.h"

#define START_TASK_PRIO		1
#define START_STK_SIZE		128

TaskHandle_t Start_Task_Handler;

void Start_Task(void* pvParameters)
{
	taskENTER_CRITICAL();

	W5500EVB_Status_Task_Creat();

	W5500EVB_Network_Task_Creat();

	vTaskDelete(NULL);

	taskEXIT_CRITICAL();
}

void Start_Task_Creat(void)
{
	xTaskCreate(
		(TaskFunction_t)Start_Task,
		(const char*)"Start",
		(uint16_t)START_STK_SIZE,
		(void*)NULL,
		(UBaseType_t)START_TASK_PRIO,
		(TaskHandle_t*)&Start_Task_Handler
	);
}
